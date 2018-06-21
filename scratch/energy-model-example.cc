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
totalTime(120.0),
packetSize("512"),
rate("2048bps"),
m_fileName("testingAodv.csv"),
m_txp(7.5),
m_traceMobility(false),
nodePause(2.0), //2 seg
nodeSpeed(30),
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
    basicSourceHelper.Set("BasicEnergySourceInitialEnergyJ", DoubleValue(15));

    // install source
    sources = basicSourceHelper.Install(adhocNodes);

    /* device energy model */
    WifiRadioEnergyModelHelper radioEnergyHelper;
    // configure radio energy model
    radioEnergyHelper.Set("TxCurrentA", DoubleValue(0.0174));
    // install device model
    deviceModels = radioEnergyHelper.Install(adhocDevices, sources);

    NS_LOG_INFO("Run Simulation.");

    // Calculate Throughput using Flowmonitor
    //
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();


    /** simulation setup **/
    Simulator::Stop(Seconds(totalTime));
    Simulator::Run();

    std::cout << "Starting simulation for " << totalTime << " s ...\n";

    double energy_consumed_total = 0;
    double remaining_energy = 0;

    for (DeviceEnergyModelContainer::Iterator iter = deviceModels.Begin(); iter != deviceModels.End(); iter++) {
        double energyConsumed = (*iter)->GetTotalEnergyConsumption();
        energy_consumed_total += energyConsumed;
    }


    for (EnergySourceContainer::Iterator iter = sources.Begin(); iter != sources.End(); iter++) {
        double energyRemain = (*iter)->GetRemainingEnergy();
        remaining_energy += energyRemain;
    }

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

    }

    //    std::string fileName_monitor = m_fileName + "_monitor";
    NS_LOG_INFO(m_fileName);
    std::ofstream out_monitor(m_fileName + "_monitor");

    out_monitor << "ThroughputTotal,DelayAve,DataRatioPacket,TotalEnergyConsumed,TotalTxPackets,TotalRxPackets,TotalLost,RemainingEnergy\n" <<
            (RxBytes_monitor * 8.0) / totalTime / 1024 << "," << delay_suma / (double) TxPackets << ","
            << (((TxPackets - PacketsLost) * 100.0) / TxPackets) << "," << energy_consumed_total << ","
            << TxPackets << "," << RxPackets << "," << PacketsLost<< ","<< remaining_energy
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

        wifiPhy.Set("TxPowerStart", DoubleValue(33));
        wifiPhy.Set("TxPowerEnd", DoubleValue(33));
        wifiPhy.Set("TxPowerLevels", UintegerValue(1));
        wifiPhy.Set("TxGain", DoubleValue(0));
        wifiPhy.Set("RxGain", DoubleValue(0));
        wifiPhy.Set("EnergyDetectionThreshold", DoubleValue(-61.8));
        wifiPhy.Set("CcaMode1Threshold", DoubleValue(-64.8));

    // Add a mac and disable rate control
    WifiMacHelper wifiMac;
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
            "DataMode", StringValue(phyMode),
            "ControlMode", StringValue(phyMode));



//    wifiPhy.Set("TxPowerStart", DoubleValue(m_txp));
    //    wifiPhy.Set("TxPowerEnd", DoubleValue(m_txp));

    //Set Non-unicastMode rate to unicast mode
    Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue(phyMode));

    wifiMac.SetType("ns3::AdhocWifiMac");
    adhocDevices = wifi.Install(wifiPhy, wifiMac, adhocNodes);
}

void
RoutingExperiment::InstallInternetStack() {

    AodvHelper aodv;
    // you can configure AODV attributes here using aodv.Set(name, value)
    aodv.Set("energyEnhance", BooleanValue(energyEnhance));
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
