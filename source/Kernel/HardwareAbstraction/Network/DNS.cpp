// DNS.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <ctype.h>
#include <HardwareAbstraction/Network.hpp>
#include <HardwareAbstraction/Devices.hpp>

using namespace Library;

namespace Kernel {
namespace HardwareAbstraction {
namespace Network {
namespace DNS
{
	struct DNSMappingv4
	{
		IPv4Address ip;
		uint64_t expiry;
	};

	// struct DNSMappingv6
	// {
	// 	IPv6Address ip;
	// 	uint64_t expiry;
	// };

	struct DNSMessageHeader
	{
		uint16_t id;
		struct
		{
			uint8_t userecursion : 1;
			uint8_t truncated : 1;
			uint8_t authoritative : 1;
			uint8_t opcode : 4;
			uint8_t query : 1;

		} __attribute__ ((packed));

		struct
		{
			uint8_t responsecode : 4;
			uint8_t reserved : 3;
			uint8_t hasrecursion : 1;

		} __attribute__ ((packed));

		uint16_t questions;
		uint16_t answers;
		uint16_t authorities;
		uint16_t extras;

	} __attribute__ ((packed));

	enum class RecordType
	{
		A		= 0x1,
		CNAME		= 0x5,
		AAAA		= 0x1C
	};

	enum class RecordClass
	{
		Internet	= 0x1
	};












	static rde::hash_map<rde::string, DNSMappingv4>* dnsmapv4 = 0;
	// static rde::hash_map<rde::string, DNSMappingv6>* dnsmapv6 = 0;

	static IPv4Address DNSServer;
	static uint64_t socket = 0;
	static uint16_t udpport = 0;

	IPv4Address GetDNSServer()
	{
		return DNSServer;
	}

	void SetDNSServer(IPv4Address addr)
	{
		DNSServer = addr;
	}

	void FailDNS(rde::string name)
	{
		(void) name;
	}


	static void SendDNSPacket(Devices::NIC::GenericNIC* interface, rde::string query, uint16_t id)
	{
		DNSMessageHeader* header = (DNSMessageHeader*) new uint8_t[sizeof(DNSMessageHeader) + query.length() + 8];
		Memory::Set(header, 0, sizeof(DNSMessageHeader));
		header->id = SwapEndian16(id);

		// QR bit = 0
		// OPCODE = 0
		// set recursion desired
		header->userecursion = 1;
		header->questions = SwapEndian16(1);

		// rest is zero.
		// setup the question.
		uint8_t* question = (uint8_t*) ((uint64_t) header + sizeof(DNSMessageHeader));
		uint8_t* name = question + 1;
		uint8_t lenind = 0;
		for(size_t i = 0; i < query.length(); )
		{
			uint8_t curlen = 0;
			while(query[i] != '.')
			{
				name[i] = query[i];
				curlen++;
				i++;

				if(i == query.length())
					break;
			}

			question[lenind] = curlen;
			lenind += curlen + 1;

			if(i == query.length())
				break;

			i++;
		}
		question[query.length() + 1] = 0;

		uint16_t* other = (uint16_t*) ((uint64_t) question + query.length() + 2);
		other[0] = SwapEndian16((uint16_t) RecordType::A);
		other[1] = SwapEndian16((uint16_t) RecordClass::Internet);

		UDP::SendIPv4Packet(interface, (uint8_t*) header, sizeof(DNSMessageHeader) + query.length() + 6, DNSServer, udpport, 53);
	}

	static void HandleDNSPacket(uint8_t* packet, uint64_t bytes)
	{
		UNUSED(bytes);

		DNSMessageHeader* dns = (DNSMessageHeader*) packet;
		uint8_t* raw = packet;

		if(dns->responsecode != 0)
		{
			Log(1, "DNS Query error - %x", dns->responsecode);
			return;
		}

		uint64_t offset = sizeof(DNSMessageHeader);
		uint16_t questions = SwapEndian16(dns->questions);
		for(size_t i = 0; i < questions; i++)
		{
			while(raw[offset] != 0)
			{
				uint8_t len = raw[offset];
				offset += len + 1;
			}
			offset++;
			offset += 4;
		}

		uint16_t* answers = (uint16_t*) (raw + offset);
		rde::string name;

		// check if it's a pointer.
		if(answers[0] & 0xC0)
		{
			uint16_t stroff = SwapEndian16(*answers);
			stroff &= ~0xC000;

			uint8_t* chars = raw + stroff;
			for(int i = 0; chars[i] != 0; i++)
			{
				if(chars[i] < 'A' && i > 0)
					name.append('.');

				else if(isalpha(chars[i]))
					name.append(chars[i]);
			}
		}
		else
		{
			HALT("error: not supported");
		}


		// now we have the name, process the actual answer.
		RecordType rtype = (RecordType) SwapEndian16(*(answers + 1));
		RecordClass rclass = (RecordClass) SwapEndian16(*(answers + 2));
		uint32_t ttl = SwapEndian32(*((uint32_t*) (raw + offset + 6)));
		// make ttl an 'expiry date'
		// this way, when we retrieve the DNS mapping, we can check the expiry date against Time::Now() to see if we need to
		// refresh it.
		ttl = (uint32_t) (Time::Now() / 1000) + ttl;

		assert(rclass == RecordClass::Internet);
		if(rtype == RecordType::A)
		{
			uint32_t rawip = *((uint32_t*) (raw + offset + 12));
			DNSMappingv4 map = { IPv4Address { rawip }, ttl };

			Log("Found IP for '%s': %d.%d.%d.%d", name.c_str(), map.ip.b1, map.ip.b2, map.ip.b3, map.ip.b4);
			(*dnsmapv4)[name] = map;
		}
	}

	IPv4Address QueryDNSv4(rde::string host)
	{
		DNSMappingv4 map = (*dnsmapv4)[host];

		Log("Querying IP address for host '%s'", host.c_str());

		// don't keep it, return and delete.
		if(map.ip.raw != 0 && map.expiry == 0)
		{
			IPv4Address ret;
			ret = map.ip;
			dnsmapv4->erase(host);

			return ret;
		}

		// check if the DNS resolver has expired
		if(map.ip.raw == 0 || !((Time::Now() / 1000) > map.expiry))
		{
			Devices::NIC::GenericNIC* nic = (Devices::NIC::GenericNIC*) Devices::DeviceManager::GetDevice(Devices::DeviceType::EthernetNIC);
			assert(nic);

			SendDNSPacket(nic, host, Kernel::KernelRandom->Generate16());
			volatile uint64_t future = Time::Now() + 2000;
			while((*dnsmapv4)[host].ip.raw == 0)
			{
				if(Time::Now() > future)
				{
					Log(1, "DNS failed to resolve");
					return 0;
				}
			}

			map = (*dnsmapv4)[host];
			return map.ip;
		}

		return map.ip;
	}


	void MonitorThread()
	{
		while(true)
		{
			if(socket > 0 && GetSocketBufferFill(socket) > 0)
			{
				uint64_t bytes = GetSocketBufferFill(socket);
				uint8_t* packetbuffer = new uint8_t[bytes];
				ReadSocket(socket, packetbuffer, bytes);

				HandleDNSPacket(packetbuffer, bytes);
				delete[] packetbuffer;
			}
			else
			{
				YieldCPU();
			}
		}
	}





	void Initialise()
	{
		dnsmapv4 = new rde::hash_map<rde::string, DNSMappingv4>();

		// wait until we have a legit dns server.
		while(DNSServer.raw == 0)
			YieldCPU();

		// open a socket on UDP for DNS.
		// socket = OpenSocket(DNSServer, udpport, 53, SocketProtocol::UDP);

		udpport = UDP::AllocateEphemeralPort();
		socket = OpenSocket(SocketProtocol::UDP, 0);
		BindSocket(socket, 0, udpport);
		ConnectSocket(socket, DNSServer, 53);

		// create the monitor thread
		Multitasking::AddToQueue(Multitasking::CreateKernelThread(MonitorThread, 2));
	}

}
}
}
}
