/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Network Security Lab, University of Washington, Seattle.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Sidharth Nabar <snabar@uw.edu>, He Wu <mdzz@u.washington.edu>
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/internet-module.h"
#include "ns3/aodv-module.h"
#include "ns3/applications-module.h"

#include "ns3/wifi-module.h"

#include "ns3/simple-device-energy-model.h"
#include "ns3/basic-energy-source.h"
#include "ns3/wifi-radio-energy-model.h"
#include "ns3/energy-module.h"
#include "ns3/basic-energy-source-helper.h"
#include "ns3/wifi-radio-energy-model-helper.h"
#include "ns3/energy-source-container.h"
#include "ns3/device-energy-model-container.h"
#include "ns3/yans-wifi-helper.h"

#include "ns3/buildings-module.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/simulator.h"
#include "ns3/double.h"
#include "ns3/config.h"
#include "ns3/string.h"

#include "ns3/flow-monitor-module.h"
#include <cmath>

#include <iostream>
#include <fstream>
#include <vector>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("EnergyExample");

static inline std::string
PrintReceivedPacket(Ptr<Socket> socket, Ptr<Packet> packet, Address senderAddress) {
    std::ostringstream oss;

    oss << Simulator::Now().GetSeconds() << " " << socket->GetNode()->GetId();

    if (InetSocketAddress::IsMatchingType(senderAddress)) {
        InetSocketAddress addr = InetSocketAddress::ConvertFrom(senderAddress);
        oss << " received one packet from " << addr.GetIpv4();
    } else {
        oss << " received one packet!";
    }
    return oss.str();
}

//static void
//-RemainingEnergy(std::string context, double oldValue, double remainingEnergy) {
//
//    if (remainingEnergy == 0) {
//        // NS_LOG_UNCOND("At " << Simulator::Now().GetSeconds() << "s Node " << context << " died.");
//    }
//    // NS_LOG_UNCOND(Simulator::Now().GetSeconds() << "From " << oldValue << " to "
//    //         << "s Current remaining energy = " << remainingEnergy << "J");
//}
//
///// Trace function for total energy consumption at node.
//
//static void
//TotalEnergy(std::string context, double oldValue, double totalEnergy) {
//    //  NS_LOG_UNCOND(Simulator::Now().GetSeconds()
//    //          << "s Total energy consumed by radio = " << totalEnergy << "J");
//}

class RoutingExperiment {
public:
    RoutingExperiment();
    void Run(int argc, char *argv[]);
    //static void SetMACParam (ns3::NetDeviceContainer & devices,
    //                                 int slotDistance);
    std::string CommandSetup(int argc, char **argv);

private:
    Ptr<Socket> SetupPacketReceive(Ipv4Address addr, Ptr<Node> node);
    void ReceivePacket(Ptr<Socket> socket);
    //    void CheckThroughput();
    void CreateNodes();
    void CreateDevices();
    void InstallInternetStack();
    void InstallApplications();
    // void RemainingEnergy(double oldValue, double remainingEnergy);
    // void TotalEnergy(double oldValue, double remainingEnergy);

    ///\name parameters
    //\{
    /// Number of nodes
    uint32_t nWifis;
    /// Simulation time, seconds
    double totalTime;
    /// Socket port
    uint32_t port = 9;
    /// Packet aux
    uint32_t bytesTotal = 0;
    /// Packet size
    std::string packetSize;
    /// Data rate
    std::string rate;
    /// Packets received count
    uint32_t packetsReceived = 0;
    /// CSV File Name
    std::string m_fileName;
    /// Sinks nodes 
    int nSinks = 10;
    /// Transmission power
    double m_txp;
    /// True if tracing mobility
    bool m_traceMobility;
    /// Time pausing
    uint32_t nodePause;
    /// Nodes velocity
    int nodeSpeed; //in m/s
    /// Enable Energy Enhance
    bool energyEnhance;
    //\}

    ///\name network
    //\{
    NodeContainer adhocNodes;
    NetDeviceContainer adhocDevices;
    Ipv4InterfaceContainer adhocInterfaces;
    DeviceEnergyModelContainer deviceModels;
    EnergySourceContainer sources;
    //\}
};

RoutingExperiment::RoutingExperiment()
: nWifis(20),
totalTime(60.0),
packetSize("512"),
rate("2048bps"),
m_fileName("testingAodv.csv"),
m_txp(7.5),
m_traceMobility(false),
nodePause(2.0), //2 seg
nodeSpeed(20),
energyEnhance(false) {
}

/**
 * \param socket Pointer to socket.
 *
 * Packet receiving sink.
 */
void
RoutingExperiment::ReceivePacket(Ptr<Socket> socket) {
    Ptr<Packet> packet;
    Address senderAddress;
    while ((packet = socket->RecvFrom(senderAddress))) {
        bytesTotal += packet->GetSize();
        packetsReceived += 1;
        //     NS_LOG_UNCOND(PrintReceivedPacket(socket, packet, senderAddress));
    }
}

/// Trace function for remaining energy at node.

//void
//RoutingExperiment::CheckThroughput() {
//    double kbs = (bytesTotal * 8.0) / 1024;
//    bytesTotal = 0;
//    //    double energyConsumed = 0;
//    //    double remainingEnergy = 0;
//
//    std::ofstream out(m_CSVfileName.c_str(), std::ios::app);
//
//    //    for (int i = 0; i < nWifis; i++) {
//    //
//    //        Ptr<BasicEnergySource> basicSourcePtr = DynamicCast<BasicEnergySource> (sources.Get(i));
//    //        remainingEnergy += basicSourcePtr->GetRemainingEnergy();
//    //
//    //        Ptr<DeviceEnergyModel> basicRadioModelPtr =
//    //                (basicSourcePtr->FindDeviceEnergyModels("ns3::WifiRadioEnergyModel")).Get(0);
//    //        NS_ASSERT(basicRadioModelPtr != NULL);
//    //        energyConsumed += basicRadioModelPtr->GetTotalEnergyConsumption();
//    //    }
//
//    // NS_LOG_UNCOND("Consumo en segundo " << Simulator::Now().GetSeconds() << ": " << energyConsumed);
//    // NS_LOG_UNCOND("Energia residual  " << Simulator::Now().GetSeconds() << ": " << remainingEnergy);
//
////    out << (Simulator::Now()).GetSeconds() << ","
////            << kbs << ","
////            << packetsReceived << ","
////            << nWifis << ","
////            << nSinks << ","
////            << m_txp << ","
////            << nodePause
////            //            << (energyConsumed - energyConsumed_old_aux) << ","
////            //            << remainingEnergy
////            << std::endl;
////
////    out.close();
//    packetsReceived = 0;
//    // energyConsumed_old_aux = energyConsumed;
//    Simulator::Schedule(Seconds(1.0), &RoutingExperiment::CheckThroughput, this);
//}

Ptr<Socket>
RoutingExperiment::SetupPacketReceive(Ipv4Address addr, Ptr<Node> node) {
    TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
    Ptr<Socket> sink = Socket::CreateSocket(node, tid);
    InetSocketAddress local = InetSocketAddress(addr, port);
    sink->Bind(local);
    sink->SetRecvCallback(MakeCallback(&RoutingExperiment::ReceivePacket, this));

    return sink;
}

int
main(int argc, char *argv[]) {
    /*
    LogComponentEnable ("EnergySource", LOG_LEVEL_DEBUG);
    LogComponentEnable ("BasicEnergySource", LOG_LEVEL_DEBUG);
    LogComponentEnable ("DeviceEnergyModel", LOG_LEVEL_DEBUG);
    LogComponentEnable ("WifiRadioEnergyModel", LOG_LEVEL_DEBUG);
     * 
     */
    // Enable AODV logs by default. Comment this if too noisy
   // LogComponentEnable("AodvRoutingProtocol", LOG_LEVEL_LOGIC);
   // LogComponentEnable("AodvRoutingTable", LOG_LEVEL_LOGIC);

    LogComponentEnable("EnergyExample", LogLevel(LOG_PREFIX_TIME | LOG_PREFIX_NODE | LOG_LEVEL_INFO));



    RoutingExperiment experiment;
    // experiment.CommandSetup(argc, argv);

    //blank out the last output file and write the column headers
    //    std::ofstream out(CSVfileName.c_str());
    //    out << "SimulationSecond," <<
    //            "ReceiveRate," <<
    //            "PacketsReceived," <<
    //            "NumberOfNodes," <<
    //            "NumberOfSinks," <<
    //            //  "RoutingProtocol," <<
    //            "TransmissionPower," <<
    //            "nodePause," <<
    //            //  "Total_energy_consumed," <<
    //            //  "Remaining_energy" <<
    //            std::endl;
    //    out.close();

    experiment.Run(argc, argv);

    return 0;
}

void
RoutingExperiment::Run(int argc, char *argv[]) {
    CommandLine cmd;
    cmd.AddValue("nWifis", "Total number of nodes", nWifis);
    cmd.AddValue("totalTime", "Simulation time", totalTime);
    //  cmd.AddValue("port", "Socket port", port);
    //  cmd.AddValue("bytesTotal", "Initial bytescount", bytesTotal);
    cmd.AddValue("packetSize", "Packet size", packetSize);
    cmd.AddValue("rate", "Data rate", rate);
    cmd.AddValue("fileName", "The name of the CSV output file name", m_fileName);
    // cmd.AddValue("nSinks", "Amoung of tx nodes", nSinks);
    cmd.AddValue("m_txp", "Transmission power", m_txp);
    cmd.AddValue("traceMobility", "Enable mobility tracing", m_traceMobility);
    cmd.AddValue("pause", "Pausing time", nodePause);
    cmd.AddValue("nodeSpeed", "Velocity on mobility", nodeSpeed);
    cmd.AddValue("energyEnhance", "Enable energy enhance", energyEnhance);
    cmd.Parse(argc, argv);

    //  NS_ASSERT(m_CSVfileName != NULL);
    //  NS_LOG_INFO(m_CSVfileName);

    Packet::EnablePrinting();

    CreateNodes();
    CreateDevices();
    InstallInternetStack();
    InstallApplications();

    /** Energy Model **/
    /***************************************************************************/
    /* energy source */
    BasicEnergySourceHelper basicSourceHelper;
    // configure energy source
    basicSourceHelper.Set("BasicEnergySourceInitialEnergyJ", DoubleValue(20));

    // install source
    sources = basicSourceHelper.Install(adhocNodes);

    /* device energy model */
    WifiRadioEnergyModelHelper radioEnergyHelper;
    // configure radio energy model
    radioEnergyHelper.Set("TxCurrentA", DoubleValue(0.0174));
    // install device model
    deviceModels = radioEnergyHelper.Install(adhocDevices, sources);

    NS_LOG_INFO("Run Simulation.");
    //    CheckThroughput();


    ////////////////////////////////////////////////////////

    //iterate on the node container an asign Energy model and Device to each Node
    //    int index = 0;
    //    for (NodeContainer::Iterator j = adhocNodes.Begin(); j != adhocNodes.End(); ++j) {
    //
    //        // adding tracing functions
    //
    //
    //        std::string context = static_cast<std::ostringstream*> (&(std::ostringstream() << index))->str();
    //        //std::cout << "Connecting node " << context << std::endl;
    //        Ptr<BasicEnergySource> basicSourcePtr = DynamicCast<BasicEnergySource> (sources.Get(index));
    //        //nodes.Get(index) -> AggregateObject(basicSourcePtr);
    //        basicSourcePtr->TraceConnect("RemainingEnergy", context, MakeCallback(&RemainingEnergy));
    //
    //        // device energy model
    //        Ptr<DeviceEnergyModel> basicRadioModelPtr =
    //                basicSourcePtr->FindDeviceEnergyModels("ns3::WifiRadioEnergyModel").Get(0);
    //
    //
    //
    //        // device energy model
    //        // Ptr<DeviceEnergyModel> basicRadioModelPtr =
    //        //   basicSourcePtr->FindDeviceEnergyModels ("ns3::WifiRadioEnergyModel").Get (0);
    //        NS_ASSERT(basicSourcePtr != NULL);
    //        basicSourcePtr->TraceConnect("TotalEnergyConsumption", context, MakeCallback(&TotalEnergy));
    //        //nodes.Get (index)->AggregateObject (sources.Get (index));
    //        index++;
    //    }

    ///////////////////////////////////////////////////////////////
    //
    // Calculate Throughput using Flowmonitor
    //
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();


    /** simulation setup **/
    Simulator::Stop(Seconds(totalTime));
    Simulator::Run();

    /***************************************************************************/
    // all sources are connected to node 1
    // energy source
    //    Ptr<BasicEnergySource> basicSourcePtr = DynamicCast<BasicEnergySource> (sources.Get(1));
    //    basicSourcePtr->TraceConnectWithoutContext("RemainingEnergy", MakeCallback(&RoutingExperiment::RemainingEnergy, this));
    //    // device energy modelnet 
    //    Ptr<DeviceEnergyModel> basicRadioModelPtr =
    //            basicSourcePtr->FindDeviceEnergyModels("ns3::WifiRadioEnergyModel").Get(0);
    //    NS_ASSERT(basicRadioModelPtr != NULL);
    //
    //    basicRadioModelPtr->TraceConnectWithoutContext("TotalEnergyConsumption", MakeCallback(&RoutingExperiment::TotalEnergy, this));

    std::cout << "Starting simulation for " << totalTime << " s ...\n";


    //    std::string CSVfileName_energy = m_CSVfileName + "_energy";
    //    std::ofstream out_(CSVfileName_energy.c_str());
    double energy_consumed_total = 0;

    for (DeviceEnergyModelContainer::Iterator iter = deviceModels.Begin(); iter != deviceModels.End(); iter++) {
        double energyConsumed = (*iter)->GetTotalEnergyConsumption();
        //        NS_LOG_UNCOND("End of simulation (" << Simulator::Now().GetSeconds()
        //                << "s) Total energy consumed by radio = " << energyConsumed << "J");
        // NS_ASSERT(energyConsumed <= 0.1);
        energy_consumed_total += energyConsumed;

        //        out_ << (Simulator::Now()).GetSeconds() << ","
        //                << energyConsumed
        //                << std::endl;
    }
    //    out_.close();


    //
    // Calculate Throughput using Flowmonitor
    // Now, do the actual simulation.
    //
    monitor->CheckForLostPackets();

    double RxBytes_monitor = 0;
    uint32_t RxPackets = 0;
    uint32_t TxPackets = 0;
    uint32_t PacketsLost = 0;

    double delay_suma = 0;

    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier());

    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin(); i != stats.end(); ++i) {

        //          Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);

        RxBytes_monitor += i->second.rxBytes;
        RxPackets += i->second.rxPackets;
        TxPackets += i->second.txPackets;
        PacketsLost += i->second.lostPackets;

        delay_suma += (i->second.delaySum).GetSeconds();

        //        double throughput_aux = (i->second.rxBytes * 8.0) / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds()) / 1024;
        //        throughput_monitor += throughput_aux;


        //   std::cout << "Flow " << i->first << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
        //        std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
        //        std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
        //        std::cout << "  Throughput: " << throughput_aux << " Kbps\n";
        //
        //        std::cout << "   Delay: " << i->second.delaySum << " ns\n";
        //   std::cout << "   TxPackets: " << i->second.txPackets << " \n";
        //   std::cout << "   RxPackets: " << i->second.rxPackets << " \n";
        //        std::cout << "   LostPackets: " << i->second.lostPackets << " \n";
    }

    //    std::string fileName_monitor = m_fileName + "_monitor";
    NS_LOG_INFO(m_fileName);
    std::ofstream out_monitor(m_fileName + "_monitor");

    out_monitor << "ThroughputTotal,DelayAve,DataRatioPacket,TotalEnergyConsumed,TotalTxPackets,TotalRxPackets,TotalLost\n" <<
            (RxBytes_monitor * 8.0) / totalTime / 1024 << "," << delay_suma / (double) TxPackets << ","
            << (((TxPackets - PacketsLost) * 100.0) / TxPackets) << "," << energy_consumed_total << ","
            << TxPackets << "," << RxPackets << "," << PacketsLost
            << std::endl;
    out_monitor.close();

    monitor->SerializeToXmlFile("lab-4.flowmon", true, true);

    Simulator::Destroy();
}

void
RoutingExperiment::CreateNodes() {
    std::cout << "Creating " << nWifis << " nodes .\n";
    std::cout << nSinks << " nodes will receive.\n";

    adhocNodes.Create(nWifis);
    // Name nodes
    for (unsigned i = 0; i < nWifis; ++i) {

        std::ostringstream os;
        os << "node-" << i;
        Names::Add(os.str(), adhocNodes.Get(i));
    }

    // mobility configuration
    MobilityHelper mobilityAdhoc;
    int64_t streamIndex = 0; // used to get consistent mobility across scenarios

    ObjectFactory pos;
    pos.SetTypeId("ns3::RandomRectanglePositionAllocator");
    pos.Set("X", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=800.0]"));
    pos.Set("Y", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=800.0]"));

    Ptr<PositionAllocator> taPositionAlloc = pos.Create()->GetObject<PositionAllocator> ();
    streamIndex += taPositionAlloc->AssignStreams(streamIndex);

    std::stringstream ssSpeed;
    ssSpeed << "ns3::UniformRandomVariable[Min=0.0|Max=" << nodeSpeed << "]";
    std::stringstream ssPause;
    ssPause << "ns3::ConstantRandomVariable[Constant=" << (int) nodePause << "]";
    mobilityAdhoc.SetMobilityModel("ns3::RandomWaypointMobilityModel",
            "Speed", StringValue(ssSpeed.str()),
            "Pause", StringValue(ssPause.str()),
            "PositionAllocator", PointerValue(taPositionAlloc));
    mobilityAdhoc.SetPositionAllocator(taPositionAlloc);
    mobilityAdhoc.Install(adhocNodes);
    streamIndex += mobilityAdhoc.AssignStreams(adhocNodes, streamIndex);
    NS_UNUSED(streamIndex); // From this point, streamIndex is unused
}

void
RoutingExperiment::CreateDevices() {
    // Parameters    

    std::string phyMode("DsssRate11Mbps");
    // Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", UintegerValue(1)); // enable rts cts all the time.

    // setting up wifi phy and channel using helpers
    WifiHelper wifi;
    wifi.SetStandard(WIFI_PHY_STANDARD_80211b);

    YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");
    wifiPhy.SetChannel(wifiChannel.Create());

    //    wifiPhy.Set("TxPowerStart", DoubleValue(33));
    //    wifiPhy.Set("TxPowerEnd", DoubleValue(33));
    //    wifiPhy.Set("TxPowerLevels", UintegerValue(1));
    //    wifiPhy.Set("TxGain", DoubleValue(0));
    //    wifiPhy.Set("RxGain", DoubleValue(0));
    //    wifiPhy.Set("EnergyDetectionThreshold", DoubleValue(-61.8));
    //    wifiPhy.Set("CcaMode1Threshold", DoubleValue(-64.8));

    // Add a mac and disable rate control
    WifiMacHelper wifiMac;
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
            "DataMode", StringValue(phyMode),
            "ControlMode", StringValue(phyMode));



    wifiPhy.Set("TxPowerStart", DoubleValue(m_txp));
    wifiPhy.Set("TxPowerEnd", DoubleValue(m_txp));

    //Set Non-unicastMode rate to unicast mode
    Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue(phyMode));

    wifiMac.SetType("ns3::AdhocWifiMac");
    adhocDevices = wifi.Install(wifiPhy, wifiMac, adhocNodes);
}

void
RoutingExperiment::InstallInternetStack() {

    AodvHelper aodv;
    // you can configure AODV attributes here using aodv.Set(name, value)
    aodv.Set("powerSaving", BooleanValue(energyEnhance));
    InternetStackHelper internet;
    internet.SetRoutingHelper(aodv); // has effect on the next Install ()
    internet.Install(adhocNodes);

    //NS_LOG_INFO ("assigning ip address");
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    adhocInterfaces = address.Assign(adhocDevices);

    if (true) {
        Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("aodv.routes", std::ios::out);
        aodv.PrintRoutingTableAllAt(Seconds(8), routingStream);
    }
}

void
RoutingExperiment::InstallApplications() {

    Config::SetDefault("ns3::OnOffApplication::PacketSize", StringValue(packetSize));
    Config::SetDefault("ns3::OnOffApplication::DataRate", StringValue(rate));

    OnOffHelper onoff1("ns3::UdpSocketFactory", Address());
    onoff1.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1.0]"));
    onoff1.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));

    for (int i = 0; i < nSinks; i++) {
        Ptr<Socket> sink = SetupPacketReceive(adhocInterfaces.GetAddress(i), adhocNodes.Get(i));

        AddressValue remoteAddress(InetSocketAddress(adhocInterfaces.GetAddress(i), port));
        onoff1.SetAttribute("Remote", remoteAddress);

        Ptr<UniformRandomVariable> var = CreateObject<UniformRandomVariable> ();
        ApplicationContainer temp = onoff1.Install(adhocNodes.Get(i + nSinks));
        temp.Start(Seconds(var->GetValue(0.0, 0.5)));
        temp.Stop(Seconds(totalTime));
    }
}

/// Trace function for remaining energy at node.
