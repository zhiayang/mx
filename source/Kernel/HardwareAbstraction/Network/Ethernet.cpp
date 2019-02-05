// Ethernet.cpp
// Copyright (c) 2014 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <HardwareAbstraction/Network.hpp>
#include <StandardIO.hpp>


namespace Kernel {
namespace HardwareAbstraction {
namespace Network {
namespace Ethernet
{

	struct EthernetFrameHeader
	{
		EUI48Address destmac;
		EUI48Address sourcemac;
		uint16_t ethertype;

	} __attribute__((packed));


	static uint32_t CalculateCRC(uint8_t* message, uint64_t length)
	{
		uint64_t i = 0;
		uint32_t crc = 0;
		uint32_t mask = 0;

		i = 0;
		crc = 0xFFFFFFFF;
		while(i < length)
		{
			// Get next byte.
			uint32_t byte = message[i];

			crc = crc ^ byte;
			for(int j = 0; j < 8; j++)
			{
				// Do eight times.
				mask = -(crc & 1);
				crc = (crc >> 1) ^ (0xEDB88320 & mask);
			}
			i++;
		}
		return ~crc;
	}

	void SendPacket(Devices::NIC::GenericNIC* interface, void* data, uint16_t length, EtherType type, EUI48Address destmac)
	{
		if(!interface)
			interface = (Devices::NIC::GenericNIC*) Devices::DeviceManager::GetDevice(Devices::DeviceType::EthernetNIC);

		assert(interface);
		uint32_t buffersize = sizeof(EthernetFrameHeader) + ((length + 3) & ~3) + 4;
		uint8_t* newbuf = new uint8_t[buffersize];

		EthernetFrameHeader* header = (EthernetFrameHeader*) newbuf;
		for(int i = 0; i < 6; i++)
		{
			header->destmac.mac[i] = destmac.mac[i];
			header->sourcemac.mac[i] = interface->GetMAC()[i];
		}

		header->ethertype = SwapEndian16((uint16_t) type);
		Memory::Copy((void*) (newbuf + sizeof(EthernetFrameHeader)), data, length);

		*((uint32_t*) (newbuf + buffersize - 4)) = CalculateCRC(newbuf, sizeof(EthernetFrameHeader) + length);
		IO::Write(interface, 0, (uint64_t) newbuf, buffersize);
	}

	void HandlePacket(Devices::NIC::GenericNIC* interface, void* packet, uint64_t length)
	{
		// it's our job (why?!) to check what's in the packet, then send it to the relevant authorities
		// first 12 bytes will be the MAC address.
		// strip the ethernet frame.
		// keep the ethertype.

		uint16_t ethertype = SwapEndian16(*(uint16_t*)((uint64_t) packet + 12));

		switch((EtherType) ethertype)
		{
			case EtherType::ARP:
				ARP::HandlePacket(interface, (void*) ((uint64_t) packet + 12 + 2), length - 2 - 12);
				break;

			case EtherType::IPv4:
				IP::HandleIPv4Packet(interface, (void*) ((uint64_t) packet + 12 + 2), length - 2 - 12);
				break;

			case EtherType::IPv6:
				break;
		}
	}
}
}
}
}
