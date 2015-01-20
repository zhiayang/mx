// IPv4.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <HardwareAbstraction/Network.hpp>
using namespace Library;

namespace Kernel {
namespace HardwareAbstraction {
namespace Network {
namespace IP
{
	#define DefaultMaxHopCount 64

	struct PacketFragment
	{
		uint8_t* fragment;
		uint64_t fragmentlength;
	};


	// map from addr to socket object.
	static rde::hash_map<SocketIPv4Mapping, Socket*>* ipv4socketmap = 0;
	static IPv4Address thisip;
	static IPv4Address subnetmask;
	static IPv4Address gatewayip;

	void Initialise()
	{
		ipv4socketmap = new rde::hash_map<SocketIPv4Mapping, Socket*>();
	}

	static inline uint16_t _add_ones_complement16(uint16_t a, uint16_t b)
	{
		// One's complement arithmatic, overflows increment bottom bit
		return a + b + (b > 0xFFFF - a ? 1 : 0);
	}


	uint16_t CalculateIPChecksum_Partial(uint16_t prev, const void* buf, uint64_t length)
	{
		uint16_t ret = prev;
		const uint16_t* data = (uint16_t*) buf;

		for(uint64_t i = 0; i < length / 2; i ++)
		{
			ret = _add_ones_complement16(ret, SwapEndian16(*data));
			data++;
		}

		if(length % 2 == 1)
			ret = _add_ones_complement16(ret, SwapEndian16(*(const uint8_t*) data));
		return ret;
	}

	uint16_t CalculateIPChecksum_Finalise(uint16_t Value)
	{
		Value = ~Value;	// One's complement it
		return (Value == 0 ? 0xFFFF : Value);
	}

	uint16_t CalculateIPChecksum(const void* buf, uint64_t len)
	{
		return CalculateIPChecksum_Finalise(CalculateIPChecksum_Partial(0, buf, len));
	}

	void MapIPv4Socket(SocketIPv4Mapping addr, Socket* s)
	{
		assert(ipv4socketmap->find(addr) == ipv4socketmap->end());
		(*ipv4socketmap)[addr] = s;
	}

	void UnmapIPv4Socket(SocketIPv4Mapping addr)
	{
		if(ipv4socketmap->find(addr) != ipv4socketmap->end())
			ipv4socketmap->erase(addr);
	}


	IPv4Address GetIPv4Address()
	{
		return thisip;
	}

	void SetIPv4Address(Library::IPv4Address addr)
	{
		thisip = addr;
	}

	IPv4Address GetSubnetMask()
	{
		if(subnetmask.raw == 0)
			return GetIPv4Address();

		else
			return subnetmask;
	}

	void SetSubnetMask(IPv4Address addr)
	{
		subnetmask = addr;
	}

	IPv4Address GetGatewayIP()
	{
		if(gatewayip.raw == 0)
			return GetIPv4Address();

		else
			return gatewayip;
	}

	void SetGatewayIP(IPv4Address addr)
	{
		gatewayip = addr;
	}



	void HandleIPv4Packet(Devices::NIC::GenericNIC* interface, void* packet, uint64_t length)
	{
		UNUSED(interface);

		IPv4Packet* ip = (IPv4Packet*) packet;
		uint64_t raw = (uint64_t) packet;

		// some checks
		if(ip->Version != 4)
		{
			Log(1, "Invalid IPv4 version number in packet");
			return;
		}

		// in bytes, so x 4
		uint64_t headerlength = ip->HeaderLength;
		uint64_t totallength = SwapEndian16(*(uint16_t*)(raw + 2));

		if(headerlength > 5)
		{
			// options present.
			// ignore
			Log("Received IPv4 packet with options, ignoring options...");
		}

		// convert to bytes.
		headerlength *= 4;

		// the packetmap is a hashmap, mapping the identification field to the packet.
		// uint16_t ident = SwapEndian16(ip->Identification);
		PacketFragment* fragment = new PacketFragment;
		fragment->fragment = (uint8_t*) raw;
		fragment->fragmentlength = totallength;

		// check for fragmentation
		if(((ip->FlagsAndFragmentOffset & 0xE0) >> 5) & 0x1)
		{
			Log("Received IPv4 packet with fragmentation, discarding...");
			return;
		}

		// if it's a broadcast, it's to us.
		IPv4Address destip = ip->DestIPAddress;
		if(destip.raw == 0xFFFFFFFF)
			destip.raw = GetIPv4Address().raw;


		bool found = false;
		Socket* skt = (*ipv4socketmap)[SocketIPv4Mapping { ip->SourceIPAddress, destip }];
		if(skt)
			found = true;

		if(!skt && !found)
		{
			skt = (*ipv4socketmap)[SocketIPv4Mapping { IPv4Address(0xFFFFFFFF), destip }];
			if(skt)
				found = true;
		}

		if(found)
		{
			// send into socket buffer.
			skt->recvbuffer.Write((uint8_t*) packet, length);
			return;
		}

		uint8_t* payload = (uint8_t*) (raw + headerlength);
		switch((ProtocolType) ip->Protocol)
		{
			case ProtocolType::ICMP:
				// ICMP::HandlePacket(interface, payload, totallength - headerlength, ip->SourceIPAddress);
				break;

			case ProtocolType::TCP:
				TCP::HandleIPv4Packet(interface, payload, totallength - headerlength, ip->SourceIPAddress, destip);
				break;

			case ProtocolType::UDP:
				UDP::HandleIPv4Packet(interface, payload, totallength - headerlength, ip->SourceIPAddress, destip);
				break;
		}
	}

	void SendIPv4Packet(Devices::NIC::GenericNIC* interface, void* packet, uint16_t length, uint16_t id, Library::IPv4Address dest, ProtocolType prot)
	{
		// first check if the dest is valid
		EUI48Address mac = ARP::SendQuery(interface, dest);

		if(mac.isZero())
		{
			mac = ARP::GatewayMAC;
		}

		// length + IP packet size
		uint8_t* raw = new uint8_t[length + 20];
		Memory::Copy((void*) (raw + 20), packet, length);
		IPv4Packet* ip = (IPv4Packet*) raw;

		// IPv4, 5 * 4 bytes = 20 bytes
		ip->Version = 0x4;
		ip->HeaderLength = 0x5;
		ip->DSCPAndECN = 0;
		ip->TotalLength = SwapEndian16(length + 20);
		ip->Identification = SwapEndian16(id);
		ip->FlagsAndFragmentOffset = 0x40;	// bit 2 set (Don't Fragment), fragment offset = 0
		ip->HopCount = DefaultMaxHopCount;
		ip->Protocol = (uint8_t) prot;
		ip->HeaderChecksum = 0;

		ip->SourceIPAddress = GetIPv4Address();
		ip->DestIPAddress = dest;


		ip->HeaderChecksum = SwapEndian16(CalculateIPChecksum(raw, 20));
		Ethernet::SendPacket(interface, raw, length + 20, Ethernet::EtherType::IPv4, mac);
	}
}
}
}
}
