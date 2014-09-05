// // DNS.cpp
// // Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// // Licensed under the Apache License Version 2.0.

// #include <Kernel.hpp>
// #include <HardwareAbstraction/Network.hpp>
// #include <HardwareAbstraction/Devices.hpp>
// #include <StandardIO.hpp>
// #include <Hashmap.hpp>
// #include <string.hpp>

// using namespace Library;

// namespace Kernel {
// namespace HardwareAbstraction {
// namespace Network {
// namespace DNS
// {
// 	struct _DNSMappingv4
// 	{
// 		IPv4Address ip;
// 		uint64_t expiry;
// 	};

// 	struct _DNSMappingv6
// 	{
// 		IPv6Address ip;
// 		uint64_t expiry;
// 	};

// 	static HashMap<string, _DNSMappingv4>* dnsmapv4 = 0;
// 	static HashMap<string, _DNSMappingv6>* dnsmapv6 = 0;

// 	static IPv4Address* DNSServer;
// 	static uint64_t socket = 0;
// 	static uint16_t udpport = 0;

// 	IPv4Address* GetDNSServer()
// 	{
// 		return DNSServer;
// 	}

// 	void SetDNSServer(IPv4Address addr)
// 	{
// 		if(DNSServer)
// 			delete DNSServer;

// 		DNSServer = new IPv4Address;
// 		DNSServer->raw = addr.raw;
// 	}

// 	void FailDNS(string name)
// 	{
// 		(void) name;
// 	}


// 	static void SendDNSPacket(Devices::NIC::GenericNIC* interface, string query, uint16_t id)
// 	{
// 		DNSMessageHeader* header = (DNSMessageHeader*) new uint8_t[sizeof(DNSMessageHeader) + query.Length() + 8];
// 		Library::Memory::Set(header, 0, sizeof(DNSMessageHeader));
// 		header->id = SwapEndian16(id);

// 		// QR bit = 0
// 		// OPCODE = 0
// 		// set recursion desired
// 		header->userecursion = 1;
// 		header->questions = SwapEndian16(1);

// 		// rest is zero.
// 		// setup the question.
// 		uint8_t* question = (uint8_t*) ((uint64_t) header + sizeof(DNSMessageHeader));
// 		uint8_t* name = question + 1;
// 		uint8_t lenind = 0;
// 		for(int i = 0; i < query.Length(); )
// 		{
// 			uint8_t curlen = 0;
// 			while(query[i] != '.')
// 			{
// 				name[i] = query[i];
// 				curlen++;
// 				i++;

// 				if(i == query.Length())
// 					break;
// 			}

// 			question[lenind] = curlen;
// 			lenind += curlen + 1;

// 			if(i == query.Length())
// 				break;

// 			i++;
// 		}
// 		question[query.Length() + 1] = 0;

// 		uint16_t* other = (uint16_t*) ((uint64_t) question + query.Length() + 2);
// 		other[0] = SwapEndian16((uint16_t) RecordType::A);
// 		other[1] = SwapEndian16((uint16_t) RecordClass::Internet);

// 		UDP::SendIPv4Packet(interface, (uint8_t*) header, sizeof(DNSMessageHeader) + query.Length() + 6, *DNSServer, udpport, 53);
// 	}

// 	static void HandleDNSPacket(uint8_t* packet, uint64_t bytes)
// 	{
// 		UNUSED(bytes);

// 		DNSMessageHeader* dns = (DNSMessageHeader*) packet;
// 		uint8_t* raw = packet;

// 		if(dns->responsecode != 0)
// 		{
// 			Log(1, "DNS Query error - %x", dns->responsecode);
// 			return;
// 		}

// 		uint64_t offset = sizeof(DNSMessageHeader);
// 		uint16_t questions = SwapEndian16(dns->questions);
// 		for(int i = 0; i < questions; i++)
// 		{
// 			while(raw[offset] != 0)
// 			{
// 				uint8_t len = raw[offset];
// 				offset += len + 1;
// 			}
// 			offset++;
// 			offset += 4;
// 		}

// 		uint16_t* answers = (uint16_t*) (raw + offset);
// 		string name;

// 		// check if it's a pointer.
// 		if(answers[0] & 0xC0)
// 		{
// 			uint16_t stroff = SwapEndian16(*answers);
// 			stroff &= ~0xC000;

// 			uint8_t* chars = raw + stroff;
// 			for(int i = 0; chars[i] != 0; i++)
// 			{
// 				if(chars[i] < 'A' && i > 0)
// 					name.Append('.');

// 				else
// 					name.Append(chars[i]);
// 			}
// 		}
// 		else
// 		{
// 			HALT("error: not supported");
// 		}


// 		// now we have the name, process the actual answer.
// 		RecordType rtype = (RecordType) SwapEndian16(*(answers + 1));
// 		RecordClass rclass = (RecordClass) SwapEndian16(*(answers + 2));
// 		uint32_t ttl = SwapEndian32(*((uint32_t*) (raw + offset + 6)));
// 		// make ttl an 'expiry date'
// 		// this way, when we retrieve the DNS mapping, we can check the expiry date against Time::Now() to see if we need to
// 		// refresh it.
// 		ttl = (uint32_t) (Time::Now() / 1000) + ttl;

// 		assert(rclass == RecordClass::Internet);
// 		if(rtype == RecordType::A)
// 		{
// 			uint32_t rawip = *((uint32_t*) (raw + offset + 12));
// 			_DNSMappingv4 map = { IPv4Address { rawip }, ttl };
// 			dnsmapv4->Put(name, map);
// 		}
// 	}

// 	IPv4Address* QueryDNSv4(string host)
// 	{
// 		_DNSMappingv4* map = dnsmapv4->get(host);

// 		// don't keep it, return and delete.
// 		if(map && map->expiry == 0)
// 		{
// 			IPv4Address* ret = new IPv4Address;
// 			ret->raw = map->ip.raw;
// 			dnsmapv4->Remove(host);

// 			return ret;
// 		}

// 		if(!map || !((Time::Now() / 1000) > map->expiry))
// 		{
// 			SendDNSPacket(Kernel::KernelNIC, host, Kernel::KernelRandom->Generate16());
// 			volatile uint64_t future = Time::Now() + 2000;
// 			while(!(map = dnsmapv4->get(host)))
// 			{
// 				if(Time::Now() > future)
// 				{
// 					Log(1, "DNS failed to resolve");
// 					return nullptr;
// 				}
// 			}

// 			return &map->ip;
// 		}

// 		return &map->ip;
// 	}


// 	void MonitorThread()
// 	{
// 		while(true)
// 		{
// 			if(socket > 0 && Socket::BytesInSocket(socket) > 0)
// 			{
// 				uint64_t bytes = Socket::BytesInSocket(socket);
// 				uint8_t* packetbuffer = new uint8_t[bytes];
// 				Socket::ReadFromSocket(socket, packetbuffer, bytes);

// 				HandleDNSPacket(packetbuffer, bytes);
// 				delete[] packetbuffer;
// 			}
// 			YieldCPU();
// 		}
// 	}





// 	void Initialise()
// 	{
// 		dnsmapv4 = new HashMap<string, _DNSMappingv4>();
// 		dnsmapv6 = new HashMap<string, _DNSMappingv6>();

// 		// wait until we have a legit dns server.
// 		while(!DNSServer)
// 			YieldCPU();

// 		// open a socket on UDP for DNS.
// 		udpport = UDP::AllocateEphemeralPort();
// 		socket = Socket::OpenSocket(*DNSServer, udpport, 53, SocketProtocol::UDP);
// 	}

// }
// }
// }
// }
