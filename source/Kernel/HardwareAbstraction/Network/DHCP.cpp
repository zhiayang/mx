// DHCP.cpp
// Copyright (c) 2014 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <HardwareAbstraction/Network.hpp>
#include <Memory.hpp>

using namespace Library;

namespace Kernel {
namespace HardwareAbstraction {
namespace Network {
namespace DHCP
{
	// DHCP
	struct DHCPPacket
	{
		uint8_t operation;
		uint8_t hardwaretype;
		uint8_t hardwareaddrlength;
		uint8_t hopcount;

		uint32_t transactionid;
		uint16_t seconds;
		uint16_t flags;

		Library::IPv4Address thisip;
		Library::IPv4Address givenip;
		Library::IPv4Address serverip;
		Library::IPv4Address relayip;

		uint8_t hardwareaddr[16];
		uint8_t servername[64];
		uint8_t bootfilename[128];

		uint8_t cookie[4];

	} __attribute__((packed));

	namespace Options
	{
		enum class Message : uint8_t
		{
			DHCPDiscover = 1,
			DHCPOffer = 2,
			DHCPRequest = 3,
			DHCPDecline = 4,
			DHCPAck = 5,
			DHCPNak = 6,
			DHCPRelease = 7,
			DHCPInform = 8
		};

		enum class OptionCode : uint8_t
		{
			SubnetMask = 1,
			Router = 3,
			DNSServer = 6,
			DomainName = 15,
			LeaseTime = 51,
			MessageType = 53,
			DHCPServer = 54,
			ParameterRequest = 55
		};

		struct Option
		{
		};

		struct SubnetMask : public Option
		{
			SubnetMask() : code(OptionCode::SubnetMask), length(4) {}
			OptionCode code;
			uint8_t length;

			Library::IPv4Address mask;

		} __attribute__((packed));

		struct Router : public Option
		{
			Router() : code(OptionCode::Router) {}
			OptionCode code;
			uint8_t length = 0;

			Library::IPv4Address mainrouter;

		} __attribute__((packed));

		struct DNSServer : public Option
		{
			DNSServer() : code(OptionCode::DNSServer) {}
			OptionCode code;
			uint8_t length = 0;

			Library::IPv4Address mainserver;

		} __attribute__((packed));

		struct DomainName : public Option
		{
			DomainName() : code(OptionCode::DomainName) {}
			OptionCode code;
			uint8_t length = 0;

			const char* str = 0;

		} __attribute__((packed));

		struct LeaseTime : public Option
		{
			LeaseTime() : code(OptionCode::LeaseTime), length(4) {}
			OptionCode code;
			uint8_t length;

			uint32_t time = 0;

		} __attribute__((packed));

		struct MessageType : public Option
		{
			MessageType() : code(OptionCode::MessageType), length(1) {}
			OptionCode code;
			uint8_t length;

			uint8_t type = 0;

		} __attribute__((packed));

		struct DHCPServer : public Option
		{
			DHCPServer() : code(OptionCode::DHCPServer), length(4) {}
			OptionCode code;
			uint8_t length;

			Library::IPv4Address addr;

		} __attribute__((packed));

		struct ParameterRequest : public Option
		{
			ParameterRequest() : code(OptionCode::ParameterRequest) {}

			OptionCode code;
			uint8_t length = 0;
			uint8_t options[16] = { 0 };

		} __attribute__((packed));
	}


















	static fd_t socket = 0;
	static uint8_t* packetbuffer = 0;
	static uint64_t socketlength = 0;
	static uint32_t transid = 0;
	static volatile bool receivedpacket = false;

	static uint64_t LeaseTime = 0;
	static int CurrentStage = 0;
	static IPv4Address* dhcpserver;

	void HandleOffer(Devices::NIC::GenericNIC* interface);
	void SendRequest(Devices::NIC::GenericNIC* interface, IPv4Address serverip);
	void HandleAck(Devices::NIC::GenericNIC* interface);

	void MonitorThread()
	{
		while(true)
		{
			uint64_t bytes = 0;
			if((bytes = GetSocketBufferFill(socket)) > 0)
			{
				packetbuffer = new uint8_t[bytes];
				socketlength = bytes;
				receivedpacket = true;

				ReadSocket(socket, packetbuffer, bytes);

				auto iface = (Devices::NIC::GenericNIC*) Devices::DeviceManager::GetDevice(Devices::DeviceType::EthernetNIC);
				switch(CurrentStage)
				{
					case 0:
						CurrentStage++;
						HandleOffer(iface);
						break;

					case 1:
						CurrentStage++;
						HandleAck(iface);
						break;
				}
				delete[] packetbuffer;
			}
			else
			{
				YieldCPU();
			}
		}
	}

	// sets up a dhcp packet, with x bytes reserved for options.
	uint8_t* SetupPacket(Devices::NIC::GenericNIC* interface, uint64_t optionbytes)
	{
		// send the first packet, dhcpdiscover.
		uint8_t* raw = new uint8_t[sizeof(DHCPPacket) + optionbytes];
		Memory::Set(raw, 0x0, sizeof(DHCPPacket) + optionbytes);

		DHCPPacket* dhcp = (DHCPPacket*) raw;

		dhcp->operation = 0x1;			// always one for sending.
		dhcp->hardwaretype = 0x1;		// 0x1 for 10mbit ethernet
		dhcp->hardwareaddrlength = 0x6;	// 6 byte MAC48 address
		dhcp->hopcount = 0;

		dhcp->transactionid = SwapEndian32(transid);
		dhcp->seconds = 0;
		dhcp->flags = 0;

		dhcp->thisip.raw = 0;
		dhcp->givenip.raw = 0;
		dhcp->serverip.raw = 0;
		dhcp->relayip.raw = 0;

		dhcp->hardwareaddr[0] = interface->GetMAC()[0];
		dhcp->hardwareaddr[1] = interface->GetMAC()[1];
		dhcp->hardwareaddr[2] = interface->GetMAC()[2];
		dhcp->hardwareaddr[3] = interface->GetMAC()[3];
		dhcp->hardwareaddr[4] = interface->GetMAC()[4];
		dhcp->hardwareaddr[5] = interface->GetMAC()[5];

		dhcp->cookie[0] = 0x63;
		dhcp->cookie[1] = 0x82;
		dhcp->cookie[2] = 0x53;
		dhcp->cookie[3] = 0x63;

		return (uint8_t*) ((uint64_t) dhcp + sizeof(DHCPPacket));
	}

	uint8_t* SetupOption(uint8_t* addr, Options::Option* option, uint64_t optionlength)
	{
		// note that 'optionlength' here does not include the first 2 bytes
		// therefore:
		optionlength += 2;

		Memory::Copy(addr, option, optionlength);
		return (uint8_t*) ((uint64_t) addr + (((optionlength + 3) / 4)) * 4);
	}


	void Initialise()
	{
		using namespace Devices;
		Log("Initialising DHCP...");

		NIC::GenericNIC* interface = (NIC::GenericNIC*) DeviceManager::GetDevice(DeviceType::EthernetNIC);
		assert(interface);

		// setup our socket.
		socket = OpenSocket(SocketProtocol::UDP, 0);
		BindSocket(socket, 0, 67);
		ConnectSocket(socket, 0xFFFFFFFF, 68);

		// setup transaction id
		transid = Kernel::KernelRandom->Generate32();

		// discover
		using namespace Options;
		uint8_t* packet = SetupPacket(interface, 128);
		uint8_t* options = packet;

		MessageType* mtype = new MessageType();
		mtype->type = (uint8_t) Message::DHCPDiscover;
		options = SetupOption(options, mtype, 1);
		delete mtype;

		ParameterRequest* params = new ParameterRequest();
		params->length = 4;
		params->options[0] = (uint8_t) OptionCode::Router;
		params->options[1] = (uint8_t) OptionCode::SubnetMask;
		params->options[2] = (uint8_t) OptionCode::DomainName;
		params->options[3] = (uint8_t) OptionCode::DNSServer;

		options = SetupOption(options, params, 4);
		delete params;

		options[0] = 0xFF;
		options[1] = 0;
		options[2] = 0;
		options[3] = 0;

		options += 4;



		// setup the monitor thread
		Multitasking::AddToQueue(Multitasking::CreateKernelThread(MonitorThread, 2));


		uint64_t length = sizeof(DHCPPacket) + ((uintptr_t) options - (uintptr_t) packet);
		UDP::SendIPv4Packet(interface, packet - sizeof(DHCPPacket), length, IPv4Address { 0xFFFFFFFF }, 68, 67);
		Log("DHCP discover sent, awaiting reply");
		delete (packet - sizeof(DHCPPacket));
	}


	void HandleOffer(Devices::NIC::GenericNIC* interface)
	{
		// 8 byte UDP header has to go
		receivedpacket = true;
		uint8_t* packet = packetbuffer;
		uint64_t length =  socketlength;

		assert(packet[0] == 0x2);	// reply op
		assert(packet[1] == 0x1);	// hardware = ethernet
		assert(packet[2] == 0x6);	// 6 byte EUI48 address

		// check that it's the same transaction
		assert(*((uint32_t*) (packet + 4)) == SwapEndian32(transid));


		// at this point we might have received a potential IP address, but we won't do anything about it yet.
		// grab the DHCP server's IP from this though, we'll need it for the reply.

		assert(length >= sizeof(DHCPPacket));
		uint8_t* dhcp = (uint8_t*) packet;
		uint8_t* options = dhcp + sizeof(DHCPPacket);
		assert(options[0] == 53);

		uint64_t optionlength = length - sizeof(DHCPPacket);

		IPv4Address serverip;
		for(uint64_t i = 0; i < optionlength;)
		{
			if(options[0] != (uint8_t) Options::OptionCode::DHCPServer)
			{
				if(options[0] == 0x0)
				{
					options++;
					continue;
				}

				else if(options[0] == 0xFF)
					HALT("DHCP server didn't send identification in DHCPOFFER, cannot continue");

				else
				{
					options += (2 + options[1]);
				}
			}
			else
			{
				serverip.raw = ((IPv4Address*) &(options[2]))->raw;
				break;
			}
		}

		Log("Received DHCP Offer from server at %d.%d.%d.%d", serverip.b1, serverip.b2, serverip.b3, serverip.b4);
		SendRequest(interface, serverip);
	}

	void SendRequest(Devices::NIC::GenericNIC* interface, IPv4Address serverip)
	{
		Log("Sending DHCP Request to DHCP server at %d.%d.%d.%d", serverip.b1, serverip.b2, serverip.b3, serverip.b4);

		// discover
		using namespace Options;
		uint8_t* packet = SetupPacket(interface, 128);
		uint8_t* options = packet;

		MessageType* mtype = new MessageType();
		mtype->type = (uint8_t) Message::DHCPRequest;
		options = SetupOption(options, mtype, 1);
		delete mtype;

		DHCPServer* serv = new DHCPServer();
		serv->addr = serverip;
		options = SetupOption(options, serv, 4);
		delete serv;

		// fixup the server address field in the actual dhcp packet.
		DHCPPacket* dhcp = (DHCPPacket*) (packet - sizeof(DHCPPacket));
		dhcp->serverip = serverip;

		options[0] = 0xFF;
		options[1] = 0;
		options[2] = 0;
		options[3] = 0;

		options += 4;

		uint64_t length = sizeof(DHCPPacket) + ((uintptr_t) options - (uintptr_t) packet);
		Log("DHCP Request sent, awaiting reply");
		UDP::SendIPv4Packet(interface, packet - sizeof(DHCPPacket), length, IPv4Address { 0xFFFFFFFF }, 68, 67);
		delete (packet - sizeof(DHCPPacket));
	}

	void HandleAck(Devices::NIC::GenericNIC* interface)
	{
		// 8 byte UDP header has to go
		receivedpacket = true;
		uint8_t* packet = packetbuffer;
		uint64_t length =  socketlength;

		assert(packet[0] == 0x2);	// reply op
		assert(packet[1] == 0x1);	// hardware = ethernet
		assert(packet[2] == 0x6);	// 6 byte EUI48 address

		// check that it's the same transaction
		assert(*((uint32_t*) (packet + 4)) == SwapEndian32(transid));


		// this is where we can confirm our allocated IP address.

		assert(length >= sizeof(DHCPPacket));
		uint8_t* options = packet + sizeof(DHCPPacket);
		assert(options[0] == 53);

		uint64_t optionlength = length - sizeof(DHCPPacket);

		for(uint64_t i = 0; i < optionlength;)
		{
			switch(options[0])
			{
				case 0x0:
					options++;
					continue;

				case 0xFF:
					break;


				// DHCP server ident
				case (uint8_t) Options::OptionCode::DHCPServer:
					dhcpserver = new IPv4Address;
					dhcpserver->raw = ((IPv4Address*) (&options[2]))->raw;
					break;

				case (uint8_t) Options::OptionCode::SubnetMask:
					IP::SetSubnetMask(*((IPv4Address*) (&options[2])));
					break;

				case (uint8_t) Options::OptionCode::Router:
					IP::SetGatewayIP(*((IPv4Address*) (&options[2])));
					break;

				case (uint8_t) Options::OptionCode::DNSServer:
					DNS::SetDNSServer(*((IPv4Address*) (&options[2])));
					break;

				case (uint8_t) Options::OptionCode::LeaseTime:
					LeaseTime = SwapEndian32(((Options::LeaseTime*) options)->time);
					break;
			}

			if(options[0] == 0xFF)
				break;

			options += (2 + options[1]);
		}

		DHCPPacket* dhcp = (DHCPPacket*) packet;
		IP::SetIPv4Address(dhcp->givenip);

		IPv4Address thisip = IP::GetIPv4Address();
		Log("Received DHCP Ack, obtained IP address %d.%d.%d.%d for %d seconds", thisip.b1, thisip.b2, thisip.b3, thisip.b4, LeaseTime);

		// get/set the gateway MAC.
		ARP::SetGatewayMAC(ARP::SendQuery(interface, IP::GetGatewayIP()));
	}
}
}
}
}
