/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 University of Kansas
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
 * Author: Justin Rohrer <rohrej@ittc.ku.edu>
 *
 * James P.G. Sterbenz <jpgs@ittc.ku.edu>, director
 * ResiliNets Research Group  http://wiki.ittc.ku.edu/resilinets
 * Information and Telecommunication Technology Center (ITTC)
 * and Department of Electrical Engineering and Computer Science
 * The University of Kansas Lawrence, KS USA.
 *
 * Work supported in part by NSF FIND (Future Internet Design) Program
 * under grant CNS-0626918 (Postmodern Internet Architecture),
 * NSF grant CNS-1050226 (Multilayer Network Resilience Analysis and Experimentation on GENI),
 * US Department of Defense (DoD), and ITTC at The University of Kansas.
 */

/*
 * This example program allows one to run ns-3 DSDV, AODV, or OLSR under
 * a typical random waypoint mobility model.
 *
 * By default, the simulation runs for 200 simulated seconds, of which
 * the first 50 are used for start-up time.  The number of nodes is 50.
 * Nodes move according to RandomWaypointMobilityModel with a speed of
 * 20 m/s and no pause time within a 300x1500 m region.  The WiFi is
 * in ad hoc mode with a 2 Mb/s rate (802.11b) and a Friis loss model.
 * The transmit power is set to 7.5 dBm.
 *
 * It is possible to change the mobility and density of the network by
 * directly modifying the speed and the number of nodes.  It is also
 * possible to change the characteristics of the network by changing
 * the transmit power (as power increases, the impact of mobility
 * decreases and the effective density increases).
 *
 * By default, OLSR is used, but specifying a value of 2 for the protocol
 * will cause AODV to be used, and specifying a value of 3 will cause
 * DSDV to be used.
 *
 * By default, there are 10 source/sink data pairs sending UDP data
 * at an application rate of 2.048 Kb/s each.    This is typically done
 * at a rate of 4 64-byte packets per second.  Application data is
 * started at a random time between 50 and 51 seconds and continues
 * to the end of the simulation.
 *
 * The program outputs a few items:
 * - packet receptions are notified to stdout such as:
 *   <timestamp> <node-id> received one packet from <src-address>
 * - each second, the data reception statistics are tabulated and output
 *   to a comma-separated value (csv) file
 * - some tracing and flow monitor configuration that used to work is
 *   left commented inline in the program
 */

#include <fstream>
#include <iostream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/aodv-module.h"
#include "ns3/applications-module.h"
#include "ns3/energy-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("manet-routing-compare");

class RoutingExperiment {
public:
    RoutingExperiment();
    void Run();
    //static void SetMACParam (ns3::NetDeviceContainer & devices,
    //                                 int slotDistance);
    std::string CommandSetup(int argc, char **argv);

private:
    Ptr<Socket> SetupPacketReceive(Ipv4Address addr, Ptr<Node> node);
    //void PrintReceivedPacket(Ptr<Socket> socket, Ptr<Packet> packet, Address senderAddress);
    void ReceivePacket(Ptr<Socket> socket);
    void CheckThroughput();
    void CreateNodes();
    void CreateDevices();
    void InstallInternetStack();
    void InstallApplications();
    void RemainingEnergy(double oldValue, double remainingEnergy);
    void TotalEnergy(double oldValue, double remainingEnergy);

    ///\name parameters
    //\{
    /// Number of nodes
    int nWifis;
    /// Simulation time, seconds
    double totalTime;
    /// Socket port
    uint32_t port;
    /// Packet aux
    uint32_t bytesTotal;
    /// Packet size
    std::string packetSize;
    /// Data rate
    std::string rate;
    /// Packets received count
    uint32_t packetsReceived = 0;
    /// CSV File Name
    std::string m_CSVfileName;
    /// Sinks nodes 
    uint32_t nSinks;
    /// Transmission power
    double m_txp;
    /// True if tracing mobility
    bool m_traceMobility;
    /// Time pausing
    uint32_t nodePause;
    /// Nodes velocity
    int nodeSpeed; //in m/s
    //\}

    ///\name network
    //\{
    NodeContainer adhocNodes;
    NetDeviceContainer adhocDevices;
    Ipv4InterfaceContainer adhocInterfaces;
    //\}
};

RoutingExperiment::RoutingExperiment()
: nWifis(70),
totalTime(120.0),
port(9),
bytesTotal(0),
packetSize("512"),
rate("2048bps"),
m_CSVfileName("manet-routing.output.csv"),
nSinks(20), //cant nodos tx
m_txp(7.5),
m_traceMobility(false),
nodePause(2), //2 seg
nodeSpeed(20) {
}

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
        NS_LOG_UNCOND(PrintReceivedPacket(socket, packet, senderAddress));
    }
}

void
RoutingExperiment::CheckThroughput() {
    double kbs = (bytesTotal * 8.0) / 1000;
    bytesTotal = 0;

    std::ofstream out(m_CSVfileName.c_str(), std::ios::app);

    out << (Simulator::Now()).GetSeconds() << ","
            << kbs << ","
            << packetsReceived << ","
            << nSinks << ","
            << m_txp << ","
            << nodePause << ","
            << std::endl;

    out.close();
    packetsReceived = 0;
    Simulator::Schedule(Seconds(1.0), &RoutingExperiment::CheckThroughput, this);
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

std::string
RoutingExperiment::CommandSetup(int argc, char **argv) {
    CommandLine cmd;
    cmd.AddValue("nWifis", "Total number of nodes", nWifis);
    cmd.AddValue("totalTime", "Simulation time", totalTime);
    cmd.AddValue("port", "Socket port", port);
    cmd.AddValue("bytesTotal", "Initial bytescount", bytesTotal);
    cmd.AddValue("packetSize", "Packet size", packetSize);
    cmd.AddValue("rate", "Data rate", rate);
    cmd.AddValue("CSVfileName", "The name of the CSV output file name", m_CSVfileName);
    cmd.AddValue("nSinks", "Amoung of tx nodes", nSinks);
    cmd.AddValue("m_txp", "Transmission power", m_txp);
    cmd.AddValue("traceMobility", "Enable mobility tracing", m_traceMobility);
    cmd.AddValue("pause", "Pausing time", nodePause);
    cmd.AddValue("nodeSpeed", "Velocity on mobility", nodeSpeed);
    cmd.Parse(argc, argv);

    return m_CSVfileName;
}

int
main(int argc, char *argv[]) {
    RoutingExperiment experiment;
    std::string CSVfileName = experiment.CommandSetup(argc, argv);

    //blank out the last output file and write the column headers
    std::ofstream out(CSVfileName.c_str());
    out << "SimulationSecond," <<
            "ReceiveRate," <<
            "PacketsReceived," <<
            "NumberOfSinks," <<
            "RoutingProtocol," <<
            "TransmissionPower," <<
            "nodePause," <<
            "Total_energy_consumed" <<
            std::endl;
    out.close();

    experiment.Run();
}

void
RoutingExperiment::Run() {
    Packet::EnablePrinting();
    std::string tr_name("manet-routing-compare");

    CreateNodes();
    CreateDevices();

    /* energy source */
    BasicEnergySourceHelper basicSourceHelper;
    // configure energy source
    basicSourceHelper.Set("BasicEnergySourceInitialEnergyJ", DoubleValue(1.0));
    // install source
    EnergySourceContainer sources = basicSourceHelper.Install(adhocNodes);
    /* device energy model */
    WifiRadioEnergyModelHelper radioEnergyHelper;
    // configure radio energy model
    radioEnergyHelper.Set("TxCurrentA", DoubleValue(0.0174));
    radioEnergyHelper.Set("RxCurrentA", DoubleValue(0.0174));
    // install device model
    DeviceEnergyModelContainer deviceModels = radioEnergyHelper.Install(adhocDevices, sources);
    // create energy source

    InstallInternetStack();
    InstallApplications();

    //    /***************************************************************************/
    //    // all sources are connected to node 1
    //    // energy source
    //    Ptr<BasicEnergySource> basicSourcePtr = DynamicCast<BasicEnergySource> (sources.Get(1));
    //    //  basicSourcePtr->TraceConnectWithoutContext ("RemainingEnergy", MakeCallback (&RoutingExperiment::RemainingEnergy, this));
    //    // device energy model
    //    Ptr<DeviceEnergyModel> basicRadioModelPtr =
    //            basicSourcePtr->FindDeviceEnergyModels("ns3::WifiRadioEnergyModel").Get(0);
    //    NS_ASSERT(basicRadioModelPtr != NULL);
    //    //  basicRadioModelPtr->TraceConnectWithoutContext ("TotalEnergyConsumption", MakeCallback (&RoutingExperiment::TotalEnergy, this));
    //
    //    std::cout << "Starting simulation for " << totalTime << " s ...\n";


    //    std::cout << "At " << Simulator::Now().GetSeconds() << " Cell voltage: " << basicSourcePtr->GetSupplyVoltage() << " V Remaining Capacity: " <<
    //            basicSourcePtr->GetRemainingEnergy() << " Ah" << std::endl;

    NS_LOG_INFO("Run Simulation.");

    CheckThroughput();

    Simulator::Stop(Seconds(totalTime));
    Simulator::Run();

    std::string CSVfileName_energy = m_CSVfileName + "_energy";
    std::ofstream out_(CSVfileName_energy.c_str());


//    for (DeviceEnergyModelContainer::Iterator iter = deviceModels.Begin(); iter != deviceModels.End(); iter++) {
//        double energyConsumed = (*iter)->GetTotalEnergyConsumption();
//        NS_LOG_UNCOND("End of simulation (" << Simulator::Now().GetSeconds()
//                << "s) Total energy consumed by radio = " << energyConsumed << "J");
//        NS_ASSERT(energyConsumed <= 0.1);
//        out_ << (Simulator::Now()).GetSeconds() << ","
//                << energyConsumed
//                << std::endl;
//    }
//    out_.close();
    
    Simulator::Destroy();
}

void
RoutingExperiment::CreateNodes() {
    std::cout << "Creating " << (unsigned) nWifis << " nodes .\n";
    std::cout << nSinks << " nodes will move and transmit .\n";

    adhocNodes.Create(nWifis);
    // Name nodes
    for (int i = 0; i < nWifis; ++i) {
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

    // setting up wifi phy and channel using helpers
    WifiHelper wifi;
    wifi.SetStandard(WIFI_PHY_STANDARD_80211b);

    YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");
    wifiPhy.SetChannel(wifiChannel.Create());

    // Add a mac and disable rate control
    WifiMacHelper wifiMac;
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
            "DataMode", StringValue(phyMode),
            "ControlMode", StringValue(phyMode));

    // wifiPhy.Set("TxPowerStart", DoubleValue(txp));
    // wifiPhy.Set("TxPowerEnd", DoubleValue(txp));

    //Set Non-unicastMode rate to unicast mode
    Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue(phyMode));

    wifiMac.SetType("ns3::AdhocWifiMac");
    adhocDevices = wifi.Install(wifiPhy, wifiMac, adhocNodes);
}

void
RoutingExperiment::InstallInternetStack() {
    AodvHelper aodv;
    // you can configure AODV attributes here using aodv.Set(name, value)
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
    int c_nSink = nSinks;

    Config::SetDefault("ns3::OnOffApplication::PacketSize", StringValue(packetSize));
    Config::SetDefault("ns3::OnOffApplication::DataRate", StringValue(rate));

    OnOffHelper onoff1("ns3::UdpSocketFactory", Address());
    onoff1.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1.0]"));
    onoff1.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));

    for (int i = 0; i < c_nSink; i++) {
        Ptr<Socket> sink = SetupPacketReceive(adhocInterfaces.GetAddress(i), adhocNodes.Get(i));

        AddressValue remoteAddress(InetSocketAddress(adhocInterfaces.GetAddress(i), port));
        onoff1.SetAttribute("Remote", remoteAddress);

        Ptr<UniformRandomVariable> var = CreateObject<UniformRandomVariable> ();
        ApplicationContainer temp = onoff1.Install(adhocNodes.Get(i + nWifis - c_nSink));
        temp.Start(Seconds(var->GetValue(0.0, 1.0)));
        temp.Stop(Seconds(totalTime));
    }
}

/// Trace function for remaining energy at node.

void
RoutingExperiment::RemainingEnergy(double oldValue, double remainingEnergy) {
    NS_LOG_UNCOND(Simulator::Now().GetSeconds()
            << "s Current remaining energy = " << remainingEnergy << "J");
}

/// Trace function for total energy consumption at node.

void
RoutingExperiment::TotalEnergy(double oldValue, double totalEnergy) {
    NS_LOG_UNCOND(Simulator::Now().GetSeconds()
            << "s Total energy consumed by radio = " << totalEnergy << "J");
}