// // ARP.cpp
// // Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// // Licensed under Creative Commons 3.0 Unported.

// #include <Kernel.hpp>
// #include <HardwareAbstraction/Network.hpp>
// #include <StandardIO.hpp>
// #include <HashMap.hpp>

// using namespace Library::StandardIO;
// using namespace Library;

// namespace Kernel {
// namespace HardwareAbstraction {
// namespace Network {
// namespace ARP
// {
// 	static HashMap<IPv4Address, EUI48Address>* ARPTable = 0;
// 	static bool received = false;
// 	EUI48Address* GatewayMAC;

// 	ARPPacket::ARPPacket()
// 	{
// 		this->HardwareType		= SwapEndian16(0x0001);	// ethernet
// 		this->ProtocolType		= SwapEndian16(0x0800);	// ipv4
// 		this->HardwareAddressLength	= 6;				// 6 byte EUI48 addr
// 		this->ProtocolAddressLength	= 4;				// 4 byte ipv4 addr
// 		this->Operation			= SwapEndian16(0x0001);	// request = 1, reply = 2
// 	}

// 	static bool IsLocal(IPv4Address addr)
// 	{
// 		IPv4Address mask = *IP::GetSubnetMask();
// 		IPv4Address thisip = *IP::GetIPv4Address();

// 		for(int i = 0; i < 4; i++)
// 		{
// 			if((thisip.bytes[i] & mask.bytes[i]) != (addr.bytes[i] & mask.bytes[i]))
// 				return false;
// 		}

// 		return true;
// 	}


// 	void SendPacket(Devices::NIC::GenericNIC* interface, IPv4Address ip)
// 	{
// 		if(!ARPTable)
// 			ARPTable = new HashMap<IPv4Address, EUI48Address>();

// 		ARPPacket* packet = new ARPPacket;
// 		packet->SenderIPv4.raw = 0;
// 		for(int i = 0; i < 6; i++)
// 			packet->SenderMacAddress.mac[i] = interface->GetMAC()[i];

// 		for(int i = 0; i < 6; i++)
// 			packet->TargetMacAddress.mac[i] = 0xFF;

// 		packet->TargetIPv4.raw = ip.raw;
// 		// PrintFormatted("sent arp\n");
// 		Ethernet::SendPacket(interface, packet, sizeof(ARPPacket), Ethernet::EtherType::ARP, packet->TargetMacAddress);
// 	}

// 	void HandlePacket(Devices::NIC::GenericNIC* interface, void* packet, uint64_t length)
// 	{
// 		UNUSED(interface);
// 		UNUSED(length);

// 		if(!ARPTable)
// 			ARPTable = new HashMap<IPv4Address, EUI48Address>(128);

// 		ARPPacket* arp = (ARPPacket*) packet;
// 		ARPTable->Put(arp->SenderIPv4, arp->SenderMacAddress);

// 		// Log("Received ARP reply");
// 		Log("Added translation from IPv4 %d.%d.%d.%d to EUI48 (MAC) %#02x:%#02x:%#02x:%#02x:%#02x:%#02x", arp->SenderIPv4.bytes[0], arp->SenderIPv4.bytes[1], arp->SenderIPv4.bytes[2], arp->SenderIPv4.bytes[3], arp->SenderMacAddress.mac[0], arp->SenderMacAddress.mac[1], arp->SenderMacAddress.mac[2], arp->SenderMacAddress.mac[3], arp->SenderMacAddress.mac[4], arp->SenderMacAddress.mac[5]);

// 		received = true;
// 	}


// 	EUI48Address* SendQuery(Library::IPv4Address addr)
// 	{
// 		// override behaviour: if we're broadcasting IP, then we should broadcast MAC as well.
// 		if(addr.raw == 0xFFFFFFFF)
// 		{
// 			EUI48Address* ret = new EUI48Address;
// 			ret->mac[0] = 0xFF;
// 			ret->mac[1] = 0xFF;
// 			ret->mac[2] = 0xFF;
// 			ret->mac[3] = 0xFF;
// 			ret->mac[4] = 0xFF;
// 			ret->mac[5] = 0xFF;
// 			return ret;
// 		}

// 		if(!ARPTable)
// 			ARPTable = new HashMap<IPv4Address, EUI48Address>();

// 		if(!IsLocal(addr))
// 			return nullptr;

// 		if(!ARPTable->get(addr))
// 		{
// 			// Log("IP Address %d.%d.%d.%d not in cache, sending ARP request...", addr.bytes[0], addr.bytes[1], addr.bytes[2], addr.bytes[3]);
// 			ARP::SendPacket(Kernel::KernelNIC, addr);

// 			// 500 ms
// 			volatile uint64_t timeout = Time::Now() + 500;
// 			while(timeout > Time::Now())
// 			{
// 				// PrintFormatted("%d - %d\n", timeout, Time::Now());
// 				if(received)
// 				{
// 					EUI48Address* ret = ARPTable->get(addr);
// 					// Log("Received ARP reply, IP address %d.%d.%d.%d is at MAC %#02x:%#02x:%#02x:%#02x:%#02x:%#02x", addr.bytes[0], addr.bytes[1], addr.bytes[2], addr.bytes[3], ret->mac[0], ret->mac[1], ret->mac[2], ret->mac[3], ret->mac[4], ret->mac[5]);

// 					received = false;
// 					return ret;
// 				}
// 			}

// 			// Log("ARP reply not received within timeout");
// 			return nullptr;
// 		}
// 		else
// 			return ARPTable->get(addr);
// 	}
// }
// }
// }
// }
