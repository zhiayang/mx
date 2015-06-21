// UDP.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

// implements a set of functions to support processing UDP packets

#include <Kernel.hpp>
#include <HardwareAbstraction/Network.hpp>
using namespace Library;

namespace Kernel {
namespace HardwareAbstraction {
namespace Network {
namespace UDP
{
	struct UDPPacket
	{
		uint16_t sourceport;
		uint16_t destport;
		uint16_t length;
		uint16_t checksum;

	} __attribute__ ((packed));


	static rde::hash_map<SocketFullMappingv4, Socket*>* udpsocketmapv4 = 0;
	static rde::vector<uint16_t>* freeports = 0;

	uint16_t AllocateEphemeralPort()
	{
		assert(freeports);
		uint16_t ret = freeports->back();
		freeports->pop_back();

		Log("allocated udp port %d, %b", ret, freeports->contains(ret));

		return ret;
	}

	void ReleaseEphemeralPort(uint16_t port)
	{
		if(!freeports->contains(port))
			freeports->push_back(port);
	}

	bool IsDuplicatePort(uint16_t port)
	{
		return !(freeports->contains(port)) && port >= EPHEMERAL_PORT_RANGE;
	}

	void MapSocket(SocketFullMappingv4 addr, Socket* s)
	{
		if(udpsocketmapv4->find(addr) != udpsocketmapv4->end())
		{
			Log(1, "Socket (%d.%d.%d.%d:%d -> %d.%d.%d.%d:%d) already mapped, overriding",
				addr.source.ip.b1, addr.source.ip.b2, addr.source.ip.b3, addr.source.ip.b4, addr.source.port,
				addr.dest.ip.b1, addr.dest.ip.b2, addr.dest.ip.b3, addr.dest.ip.b4, addr.dest.port);
		}

		(*udpsocketmapv4)[addr] = s;
	}

	void UnmapSocket(SocketFullMappingv4 addr)
	{
		udpsocketmapv4->erase(addr);
	}

	void Initialise()
	{
		udpsocketmapv4 = new rde::hash_map<SocketFullMappingv4, Socket*>();
		freeports = new rde::vector<uint16_t>();
		for(uint16_t i = 49152; i < UINT16_MAX; i++)
			freeports->push_back(i);
	}



	void SendIPv4Packet(Devices::NIC::GenericNIC* interface, uint8_t* packet, uint64_t length, Library::IPv4Address dest, uint16_t sourceport, uint16_t destport)
	{
		uint8_t* newbuf = new uint8_t[sizeof(UDPPacket) + length];
		Memory::Copy(newbuf + sizeof(UDPPacket), packet, length);

		UDPPacket* udp = (UDPPacket*) newbuf;
		udp->sourceport = SwapEndian16(sourceport);
		udp->destport = SwapEndian16(destport);
		udp->length = SwapEndian16(length + sizeof(UDPPacket));

		udp->checksum = 0;
		IP::SendIPv4Packet(interface, newbuf, sizeof(UDPPacket) + (uint16_t) length, 459, dest, IP::ProtocolType::UDP);
	}


	void HandleIPv4Packet(Devices::NIC::GenericNIC* interface, void* packet, uint64_t length, IPv4Address source, IPv4Address destip)
	{
		UDPPacket* udp = (UDPPacket*) packet;
		uint64_t actuallength = SwapEndian16(udp->length) - sizeof(UDPPacket);

		uint16_t sourceport = SwapEndian16(udp->sourceport);
		uint16_t destport = SwapEndian16(udp->destport);

		// IPv4PortAddress target;
		// target.ip = source;
		// target.port = SwapEndian16(udp->destport);


		Log("Received UDP packet from source IP %d.%d.%d.%d", source.b1, source.b2, source.b3, source.b4);


		IPv4Address matchAny { 0xFFFFFFFF };
		IPv4Address zero { 0 };

		bool triedonce = false;

		// feel dirty
		retry:

		// check for full mapping.
		bool found = false;
		Socket* skt = (*udpsocketmapv4)[SocketFullMappingv4 { IPv4PortAddress { destip, destport }, IPv4PortAddress { source, sourceport } }];
		if(skt)
		{
			found = true;
		}

		// multiphase search: check for sourceip + sourceport + destport
		if(!skt && !found)
		{
			skt = (*udpsocketmapv4)[SocketFullMappingv4 { IPv4PortAddress { source, sourceport }, IPv4PortAddress { matchAny, destport } }];
			if(skt)
			{
				found = true;
			}
		}

		// check for sourceip + destport
		if(!skt && !found)
		{
			skt = (*udpsocketmapv4)[SocketFullMappingv4 { IPv4PortAddress { source, 0 }, IPv4PortAddress { matchAny, destport } }];
			if(skt)
				found = true;
		}

		// check for sourceport + destport
		if(!skt && !found)
		{
			skt = (*udpsocketmapv4)[SocketFullMappingv4 { IPv4PortAddress { zero, sourceport }, IPv4PortAddress { matchAny, destport } }];
			if(skt)
				found = true;
		}

		// finally, only destport.
		if(!skt && !found)
		{
			skt = (*udpsocketmapv4)[SocketFullMappingv4 { IPv4PortAddress { zero, 0 }, IPv4PortAddress { matchAny, destport } }];
			if(skt)
				found = true;
		}

		if(found)
		{
			// send into socket buffer.
			Log("writing received data (%d bytes) (from %d.%d.%d.%d) into socket", actuallength, source.b1, source.b2, source.b3, source.b4);
			skt->recvbuffer.Write((uint8_t*) packet + sizeof(UDPPacket), actuallength);

			return;
		}

		// if we got here, it didn't find.
		if(!triedonce)
		{
			triedonce = true;
			source = IPv4Address { 0 };

			goto retry;
		}

		Log("No open UDP sockets found for target, discarding packet.");

		UNUSED(interface);
		UNUSED(length);
	}


	void HandleIPv6Packet(Devices::NIC::GenericNIC* interface, void* packet, uint64_t length, IPv6Address source, IPv6Address destip)
	{
		UNUSED(interface);
		UNUSED(packet);
		UNUSED(length);
		UNUSED(source);
		UNUSED(destip);

		HALT("UDP over IPv6 not implemented.");
	}

}
}
}
}
