#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"

// Default Network Topology
//
//                           10.1.1.0                            
//                        point-to-point
//                      n0 ------------ n1
//  n8   n7   n6   n5   |               |    n2   n3   n4
//  |    |    |    |    |               |    |    |    |
//  =====================               ================
//     LAN2 10.1.3.0                      LAN1 10.1.2.0

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TaskScript");

int 
main (int argc, char *argv[])
{
  bool verbose = true;
  uint32_t nCsma3 = 3;
  uint32_t nCsma4 = 4;
  CommandLine cmd (__FILE__);
  cmd.AddValue ("nCsma3", "Number of \"extra\" CSMA nodes/devices", nCsma3);
  cmd.AddValue ("nCsma4", "Number of \"extra\" CSMA nodes/devices", nCsma4);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);

  cmd.Parse (argc,argv);

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

  nCsma3 = nCsma3 == 0 ? 1 : nCsma3;
  nCsma4 = nCsma4 == 0 ? 1 : nCsma4;

//------------Creating Point-to-Point part------------------------------------------------------
NodeContainer p2pNodes;    
p2pNodes.Create (2);

PointToPointHelper pointToPoint;
pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));  

NetDeviceContainer p2pDevices;
p2pDevices = pointToPoint.Install (p2pNodes); 


//------------Creating LAN1 Part (CSMA----------------------------------------------------------
NodeContainer csmaNodes1;
csmaNodes1.Add (p2pNodes.Get (1));   
csmaNodes1.Create (nCsma3);

CsmaHelper csma1;
csma1.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
csma1.SetChannelAttribute ("Delay", StringValue ("50ms"));

NetDeviceContainer csmaDevices1;
csmaDevices1 = csma1.Install (csmaNodes1);


//------------------Creating LAN2 Part (CSMA------------------------------------------------------
NodeContainer csmaNodes2;
csmaNodes2.Add (p2pNodes.Get (0));   
csmaNodes2.Create (nCsma4);

CsmaHelper csma2;
csma2.SetChannelAttribute ("DataRate", StringValue ("200Mbps"));
csma2.SetChannelAttribute ("Delay", StringValue ("20ms"));

NetDeviceContainer csmaDevices2;
csmaDevices2 = csma2.Install (csmaNodes2);


//----------------Installing Mobility on the nodes------------------------------------------------------
MobilityHelper mobility;
mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
mobility.Install (p2pNodes);
mobility.Install (csmaNodes1);
mobility.Install (csmaNodes2);


//------------Installing Protocol Stack on the nodes------------------------------------------------------
InternetStackHelper stack;
stack.Install (csmaNodes1);   
stack.Install (csmaNodes2);   


//------------------------Installing IP on the nodes------------------------------------------------------
Ipv4AddressHelper address;
address.SetBase ("10.1.1.0", "255.255.255.0");
Ipv4InterfaceContainer p2pInterfaces;
p2pInterfaces = address.Assign (p2pDevices);

address.SetBase ("10.1.2.0", "255.255.255.0");
Ipv4InterfaceContainer csmaInterfaces1;
csmaInterfaces1 = address.Assign (csmaDevices1);

address.SetBase ("10.1.3.0", "255.255.255.0");
Ipv4InterfaceContainer csmaInterfaces2;
csmaInterfaces2 = address.Assign (csmaDevices2);

//----creating the applications for sending and receieving data packets----------------------------------
UdpEchoServerHelper echoServer (9);
ApplicationContainer serverApps = echoServer.Install (csmaNodes1.Get (nCsma3));

serverApps.Start (Seconds (1.0));
serverApps.Stop (Seconds (10.0));


UdpEchoClientHelper echoClient (csmaInterfaces1.GetAddress (nCsma3), 9);  // UdpEchoClientHelper (server IP, port)
echoClient.SetAttribute ("MaxPackets", UintegerValue (20));  //
echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
echoClient.SetAttribute ("PacketSize", UintegerValue (1024));


ApplicationContainer clientApps = echoClient.Install (csmaNodes2.Get (nCsma4));

clientApps.Start (Seconds (2.0));
clientApps.Stop (Seconds (10.0));

Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

//------------------------------------Animation Part------------------------------------------------------
AnimationInterface anim("task-anim.xml"); 
 
//Point to Point
anim.SetConstantPosition (p2pNodes.Get (0), 40, 15, 0); 
anim.UpdateNodeDescription (p2pNodes.Get (0), "n0"); 
anim.UpdateNodeColor (p2pNodes.Get (0), 153, 204, 0);  //green
anim.SetConstantPosition (p2pNodes.Get (1), 50, 15, 0);
anim.UpdateNodeDescription (p2pNodes.Get (1), "n1"); 
anim.UpdateNodeColor (p2pNodes.Get (1), 153, 204, 0);  //green
  
  
  //CSMA1
anim.SetConstantPosition (csmaNodes1.Get (1), 60, 40, 0); 
anim.UpdateNodeDescription (csmaNodes1.Get (1), "n2"); 
anim.UpdateNodeColor (csmaNodes1.Get (1), 0, 140, 255);  //blue

anim.SetConstantPosition (csmaNodes1.Get (2), 70, 40, 0);    
anim.UpdateNodeDescription (csmaNodes1.Get (2), "n3");   
anim.UpdateNodeColor (csmaNodes1.Get (2), 0, 140, 255);  //blue  

anim.SetConstantPosition (csmaNodes1.Get (3), 80, 40, 0);
anim.UpdateNodeDescription (csmaNodes1.Get (3), "n4"); 
anim.UpdateNodeColor (csmaNodes1.Get (3), 0, 140, 255);  //blue
  
  //CSMA2
anim.SetConstantPosition (csmaNodes2.Get (4), 10, 40, 0); 
anim.UpdateNodeDescription (csmaNodes2.Get (4), "n8"); 
anim.UpdateNodeColor (csmaNodes2.Get (4), 255, 153, 204);  //pink

anim.SetConstantPosition (csmaNodes2.Get (3), 20, 40, 0);    
anim.UpdateNodeDescription (csmaNodes2.Get (3), "n7");   
anim.UpdateNodeColor (csmaNodes2.Get (3), 255, 153, 204);  //pink  

anim.SetConstantPosition (csmaNodes2.Get (2), 30, 40, 0);    
anim.UpdateNodeDescription (csmaNodes2.Get (2), "n6");   
anim.UpdateNodeColor (csmaNodes2.Get (2), 255, 153, 204);  //pink  

anim.SetConstantPosition (csmaNodes2.Get (1), 40, 40, 0);
anim.UpdateNodeDescription (csmaNodes2.Get (1), "n5"); 
anim.UpdateNodeColor (csmaNodes2.Get (1), 255, 153, 204);  //pink


Simulator::Run ();
Simulator::Destroy ();
return 0;
}
