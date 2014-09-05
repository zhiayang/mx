// // UDP.cpp
// // Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// // Licensed under the Apache License Version 2.0.

// // implements a set of functions to support processing UDP packets

// #include <Kernel.hpp>
// #include <HardwareAbstraction/Network.hpp>
// using namespace Library;

// namespace Kernel {
// namespace HardwareAbstraction {
// namespace Network {
// namespace UDP
// {
// 	HashMap<SocketFullMappingv4, Socket::SocketObj>* udpsocketmapv4 = 0;
// 	HashMap<SocketFullMappingv6, Socket::SocketObj>* udpsocketmapv6 = 0;

// 	static LinkedObjList<uint16_t>* usedports = 0;
// 	static uint16_t nextfree = 49152;

// 	uint16_t AllocateEphemeralPort()
// 	{
// 		if(!usedports)
// 			usedports = new LinkedObjList<uint16_t>();

// 		usedports->push_back(nextfree);
// 		return nextfree++;
// 	}

// 	void ReleaseEphemeralPort(uint16_t port)
// 	{
// 		for(uint64_t i = 0, s = usedports->size(); i < s; i++)
// 		{
// 			if(usedports->get(i) == port)
// 				usedports->RemoveAt(i);
// 		}
// 	}



// 	void SendIPv4Packet(Devices::NIC::GenericNIC* interface, uint8_t* packet, uint64_t length, Library::IPv4Address dest, uint16_t sourceport, uint16_t destport)
// 	{
// 		uint8_t* newbuf = new uint8_t[sizeof(UDPPacket) + length];
// 		Memory::Copy(newbuf + sizeof(UDPPacket), packet, length);

// 		UDPPacket* udp = (UDPPacket*) newbuf;
// 		udp->sourceport = SwapEndian16(sourceport);
// 		udp->destport = SwapEndian16(destport);
// 		udp->length = SwapEndian16(length + sizeof(UDPPacket));

// 		udp->checksum = 0;
// 		IP::SendIPv4Packet(interface, newbuf, sizeof(UDPPacket) + (uint16_t) length, 459, dest, IP::ProtocolType::UDP);
// 	}


// 	void HandleIPv4Packet(Devices::NIC::GenericNIC* interface, void* packet, uint64_t length, IPv4Address source, IPv4Address destip)
// 	{
// 		UDPPacket* udp = (UDPPacket*) packet;
// 		uint64_t actuallength = SwapEndian16(udp->length) - sizeof(UDPPacket);

// 		uint16_t sourceport = SwapEndian16(udp->sourceport);
// 		uint16_t destport = SwapEndian16(udp->destport);

// 		// IPv4PortAddress target;
// 		// target.ip = source;
// 		// target.port = SwapEndian16(udp->destport);

// 		bool triedonce = false;

// 		// feel dirty
// 		retry:

// 		// check for full mapping.
// 		bool found = false;
// 		Socket::SocketObj* skt = udpsocketmapv4->get(SocketFullMappingv4 { IPv4PortAddress { destip, destport }, IPv4PortAddress { source, sourceport } });
// 		if(skt)
// 			found = true;

// 		// multiphase search: check for sourceip + sourceport + destport
// 		if(!skt && !found)
// 		{
// 			skt = udpsocketmapv4->get(SocketFullMappingv4 { IPv4PortAddress { source, sourceport }, IPv4PortAddress { IPv4Address { 0xFFFFFFFF }, destport } });
// 			if(skt)
// 				found = true;
// 		}

// 		// check for sourceip + destport
// 		if(!skt && !found)
// 		{
// 			skt = udpsocketmapv4->get(SocketFullMappingv4 { IPv4PortAddress { source, 0 }, IPv4PortAddress { IPv4Address { 0xFFFFFFFF}, destport } });
// 			if(skt)
// 				found = true;
// 		}

// 		// check for sourceport + destport
// 		if(!skt && !found)
// 		{
// 			skt = udpsocketmapv4->get(SocketFullMappingv4 { IPv4PortAddress { IPv4Address { 0 }, sourceport }, IPv4PortAddress { IPv4Address { 0xFFFFFFFF }, destport } });
// 			if(skt)
// 				found = true;
// 		}

// 		// finally, only destport.
// 		if(!skt && !found)
// 		{
// 			skt = udpsocketmapv4->get(SocketFullMappingv4 { IPv4PortAddress { IPv4Address { 0 }, 0 }, IPv4PortAddress { IPv4Address { 0xFFFFFFFF }, destport } });
// 			if(skt)
// 				found = true;
// 		}

// 		if(found)
// 		{
// 			// send into socket buffer.
// 			skt->recvbuffer->Write((uint8_t*) packet + sizeof(UDPPacket), actuallength);
// 			skt->packetsizes->Write(actuallength);
// 			skt->packetcount++;
// 			return;
// 		}

// 		// if we got here, it didn't find.
// 		if(!triedonce)
// 		{
// 			triedonce = true;
// 			source = IPv4Address { 0 };
// 			goto retry;
// 		}

// 		Log("No sockets found for target, discarding packet.");

// 		UNUSED(interface);
// 		UNUSED(length);
// 	}


// 	void HandleIPv6Packet(Devices::NIC::GenericNIC* interface, void* packet, uint64_t length, IPv6Address source, IPv6Address destip)
// 	{
// 		UNUSED(interface);
// 		UNUSED(packet);
// 		UNUSED(length);
// 		UNUSED(source);
// 		UNUSED(destip);

// 		HALT("UDP over IPv6 not implemented.");
// 	}

// }
// }
// }
// }
