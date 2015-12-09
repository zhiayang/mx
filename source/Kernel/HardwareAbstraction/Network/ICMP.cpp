// // ICMP.cpp
// // Copyright (c) 2014 - 2016, zhiayang@gmail.com
// // Licensed under the Apache License Version 2.0.

// #include <Kernel.hpp>
// #include <HardwareAbstraction/Network.hpp>
// #include <StandardIO.hpp>
// using namespace Library::StandardIO;

// namespace Kernel {
// namespace HardwareAbstraction {
// namespace Network {
// namespace ICMP
// {
// 	void SendPacket(Devices::NIC::GenericNIC* interface, ICMPPacketType type, Library::IPv4Address dest)
// 	{
// 		uint16_t len = 0;
// 		switch(type)
// 		{
// 			case EchoReply:
// 			case EchoRequest:
// 				len = 8;
// 				break;

// 			default:
// 				return;
// 		}

// 		uint8_t* icmp = new uint8_t[len];
// 		ICMPBasicPacket* packet = (ICMPBasicPacket*) icmp;
// 		packet->type = (uint8_t) type;
// 		packet->code = 0;
// 		packet->checksum = 0;

// 		ICMPEchoPacket* echo = (ICMPEchoPacket*) packet;
// 		echo->identifier = SwapEndian16(49144);
// 		echo->basic.checksum = SwapEndian16(IP::CalculateIPChecksum((void*) packet, len));

// 		// random id number (419)
// 		// PrintFmt("sent ICMP\n");
// 		IP::SendIPv4Packet(interface, echo, len, 419, dest, IP::ProtocolType::ICMP);
// 	}


// 	void HandlePacket(Devices::NIC::GenericNIC* interface, void* packet, uint64_t length, Library::IPv4Address source)
// 	{
// 		UNUSED(interface);
// 		UNUSED(packet);
// 		UNUSED(length);
// 		UNUSED(source);

// 		return;
// 	}
// }
// }
// }
// }
