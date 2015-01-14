// Network.hpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <stdint.h>
#include <CircularBuffer.hpp>
#include <HardwareAbstraction/Devices/NIC.hpp>
#include <HardwareAbstraction/Filesystems.hpp>
#include <Iris/HeaderFiles/PacketNetwork.hpp>
#pragma once

#define SwapEndian16(Val)	((uint16_t)(((Val) & 0xFF) << 8) | (((Val) >> 8) & 0xFF))
#define SwapEndian32(Val)	((((Val) & 0xFF) << 24) | (((Val) & 0xFF00) << 8) | (((Val) >> 8) & 0xFF00) | (((Val) >> 24) & 0xFF))

#define GLOBAL_MTU		2048


namespace Kernel {
namespace HardwareAbstraction {
namespace Network
{
	/*
		the contents of this file are ordered based on the level of the protocol.

		Level 0:
		1. Ethernet

		Level 1:
		2. ARP
		3. IPv4
		4. IPv6 (TODO)

		Level 2:
		5. UDP
		6. TCP
		7. ICMP

		Level 3:
		8. DHCP
	*/

	struct EUI48Address
	{
		uint8_t mac[6];

	} __attribute__ ((packed));

	struct SocketFullMappingv4
	{
		Library::IPv4PortAddress source;
		Library::IPv4PortAddress dest;
	};

	struct SocketFullMappingv6
	{
		Library::IPv6PortAddress source;
		Library::IPv6PortAddress dest;
	};

	struct SocketIPv4Mapping
	{
		Library::IPv4Address source;
		Library::IPv4Address dest;
	};


	namespace Socket
	{
		class SocketObj;
	}


	namespace Ethernet
	{
		// Ethernet
		enum class EtherType : uint16_t
		{
			IPv4 = 0x0800,
			ARP = 0x0806,
			IPv6 = 0x86DD
		};

		struct EthernetFrameHeader
		{
			EUI48Address destmac;
			EUI48Address sourcemac;
			uint16_t ethertype;

		} __attribute__ ((packed));

		void SendPacket(Devices::NIC::GenericNIC* interface, void* data, uint16_t length, EtherType type, EUI48Address destmac);
		void HandlePacket(Devices::NIC::GenericNIC* interface, void* packet, uint64_t length);
	}


	namespace ARP
	{
		// ARP
		struct ARPPacket
		{
			uint16_t HardwareType;
			uint16_t ProtocolType;

			uint8_t HardwareAddressLength;
			uint8_t ProtocolAddressLength;

			uint16_t Operation;

			EUI48Address SenderMacAddress;
			Library::IPv4Address SenderIPv4;

			EUI48Address TargetMacAddress;
			Library::IPv4Address TargetIPv4;

			ARPPacket();

		} __attribute__ ((packed));

		extern EUI48Address* GatewayMAC;
		EUI48Address* SendQuery(Library::IPv4Address addr);
		void HandlePacket(Devices::NIC::GenericNIC* interface, void* PacketAddr, uint64_t length);
		void SendPacket(Devices::NIC::GenericNIC* interface, Library::IPv4Address addr);
	}















	// namespace IP
	// {
	// 	enum class ProtocolType : uint8_t
	// 	{
	// 		ICMP = 0x1,
	// 		TCP = 0x6,
	// 		UDP = 0x11
	// 	};

	// 	// IPv4
	// 	struct IPv4Packet
	// 	{
	// 		struct
	// 		{
	// 			// Spec says Version is first, but stupid bit ordering
	// 			uint8_t HeaderLength:		4;	// in 4-byte chunks
	// 			uint8_t Version:		4;	// = 4

	// 		} __attribute__((packed));

	// 		uint8_t DSCPAndECN;
	// 		uint16_t TotalLength;

	// 		uint16_t Identification;
	// 		uint16_t FlagsAndFragmentOffset;

	// 		uint8_t HopCount;
	// 		uint8_t Protocol;
	// 		uint16_t HeaderChecksum;

	// 		Library::IPv4Address SourceIPAddress;
	// 		Library::IPv4Address DestIPAddress;

	// 	} __attribute__ ((packed));

	// 	struct PseudoIPv4Header
	// 	{
	// 		Library::IPv4Address source;
	// 		Library::IPv4Address dest;
	// 		uint8_t zeroes;

	// 		uint8_t protocol;
	// 		uint16_t length;

	// 	} __attribute__ ((packed));

	// 	uint16_t CalculateIPChecksum(const void* buf, uint64_t Length);
	// 	uint16_t CalculateIPChecksum_Partial(uint16_t prev, const void* buf, uint64_t length);
	// 	uint16_t CalculateIPChecksum_Finalise(uint16_t Value);

	// 	Library::IPv4Address* GetIPv4Address();
	// 	Library::IPv4Address* GetSubnetMask();
	// 	Library::IPv4Address* GetGatewayIP();
	// 	void SetIPv4Address(Library::IPv4Address newaddr);
	// 	void SetSubnetMask(Library::IPv4Address addr);
	// 	void SetGatewayIP(Library::IPv4Address addr);


	// 	void SendIPv4Packet(Devices::NIC::GenericNIC* interface, void* packet, uint16_t length, uint16_t id, Library::IPv4Address dest, ProtocolType prot);
	// 	void HandleIPv4Packet(Devices::NIC::GenericNIC* interface, void* packet, uint64_t length);

	// 	extern Library::HashMap<SocketIPv4Mapping, Socket::SocketObj>* ipv4socketmap;
	// 	extern Library::HashMap<SocketIPv4Mapping, Socket::SocketObj>* ipv6socketmap;
	// }

	// namespace ICMP
	// {
	// 	// ICMP
	// 	struct ICMPBasicPacket
	// 	{
	// 		uint8_t type;
	// 		uint8_t code;
	// 		uint16_t checksum;

	// 	} __attribute__ ((packed));

	// 	struct ICMPEchoPacket
	// 	{
	// 		ICMPBasicPacket basic;
	// 		uint16_t identifier;
	// 		uint16_t sequence;

	// 	} __attribute__ ((packed));

	// 	struct ICMPDestUnreachablePacket
	// 	{
	// 		ICMPBasicPacket basic;
	// 		uint32_t unused;

	// 	} __attribute__ ((packed));

	// 	enum ICMPPacketType
	// 	{
	// 		EchoReply = 0,
	// 		DestinationUnreachable = 3,
	// 		EchoRequest = 8
	// 	};

	// 	void SendPacket(Devices::NIC::GenericNIC* interface, ICMPPacketType type, Library::IPv4Address dest);
	// 	void HandlePacket(Devices::NIC::GenericNIC* interface, void* packet, uint64_t length, Library::IPv4Address source);
	// }


	// namespace UDP
	// {
	// 	// UDP
	// 	struct UDPPacket
	// 	{
	// 		uint16_t sourceport;
	// 		uint16_t destport;
	// 		uint16_t length;
	// 		uint16_t checksum;

	// 	} __attribute__ ((packed));

	// 	void SendIPv4Packet(Devices::NIC::GenericNIC* interface, uint8_t* packet, uint64_t length, Library::IPv4Address dest, uint16_t sourceport, uint16_t destport);
	// 	void HandleIPv4Packet(Devices::NIC::GenericNIC* interface, void* packet, uint64_t length, Library::IPv4Address source, Library::IPv4Address destip);
	// 	void HandleIPv6Packet(Devices::NIC::GenericNIC* interface, void* packet, uint64_t length, Library::IPv6Address source, Library::IPv6Address destip);

	// 	uint16_t AllocateEphemeralPort();
	// 	void ReleaseEphemeralPort(uint16_t port);

	// 	extern Library::HashMap<SocketFullMappingv4, Socket::SocketObj>* udpsocketmapv4;
	// 	extern Library::HashMap<SocketFullMappingv6, Socket::SocketObj>* udpsocketmapv6;
	// }

	// namespace TCP
	// {
	// 	// Because TCP is a connection-oriented interface,
	// 	// it makes more sense to create a 'TCPConnection' object
	// 	// that handles things such as sequence numbers, ack numbers
	// 	// and opening/closing the connection.

	// 	enum class ConnectionError
	// 	{
	// 		NoError,
	// 		Timeout
	// 	};

	// 	enum class ConnectionState;

	// 	class TCPConnection
	// 	{
	// 		friend void TCPAckMonitor(TCPConnection*);

	// 		public:
	// 			TCPConnection(Socket::SocketObj* socket, Library::IPv4Address dest, uint16_t srcport, uint16_t destport);
	// 			TCPConnection(Socket::SocketObj* socket, Library::IPv6Address dest, uint16_t srcport, uint16_t destport);
	// 			~TCPConnection();

	// 			uint16_t clientport;
	// 			uint16_t serverport;

	// 			Socket::SocketObj* socket;

	// 			Library::IPv4Address sourceip4;
	// 			Library::IPv4Address destip4;

	// 			Library::IPv6Address sourceip6;
	// 			Library::IPv6Address destip6;

	// 			void SendUserPacket(uint8_t* packet, uint64_t bytes);

	// 			void HandleIncoming(uint8_t* packet, uint64_t bytes, uint64_t HeaderSize);
	// 			void SendPacket(uint8_t* packet, uint64_t bytes);

	// 			ConnectionError Connect();
	// 			void Disconnect();

	// 		private:
	// 			TCPConnection(Socket::SocketObj* socket, uint16_t srcport, uint16_t destport);

	// 			ConnectionError error;
	// 			uint32_t localcumlsequence;
	// 			uint32_t localsequence;
	// 			uint32_t serversequence;
	// 			uint32_t servercumlsequence;
	// 			uint64_t uuid;
	// 			uint16_t maxsegsize;
	// 			uint32_t nextack;
	// 			uint64_t bufferfill;
	// 			uint64_t lastpackettime;
	// 			uint64_t workertid;

	// 			bool AlreadyAcked;
	// 			bool PacketReceived;

	// 			ConnectionState state;
	// 			Library::CircularMemoryBuffer* packetbuffer;

	// 			void SendPacket(uint8_t* packet, uint64_t length, uint8_t flags);
	// 			void SendIPv4Packet(uint8_t* packet, uint64_t length, uint8_t flags);
	// 			void SendIPv6Packet(uint8_t* packet, uint64_t length, uint8_t flags);
	// 	};

	// 	struct TCPPacket
	// 	{
	// 		uint16_t clientport;
	// 		uint16_t serverport;
	// 		uint32_t sequence;
	// 		uint32_t ackid;

	// 		uint8_t HeaderLength;
	// 		uint8_t Flags;
	// 		uint16_t WindowSize;

	// 		uint16_t Checksum;
	// 		uint16_t UrgentPointer;

	// 	} __attribute__ ((packed));

	// 	extern Library::HashMap<SocketFullMappingv4, Socket::SocketObj>* tcpsocketmapv4;
	// 	extern Library::HashMap<SocketFullMappingv6, Socket::SocketObj>* tcpsocketmapv6;

	// 	// much simpler, because much of it is abstracted behind TCPConnection.
	// 	void SendIPv4Packet(TCPConnection* connection, uint8_t* packet, uint64_t length);

	// 	// the IPv4 layer doesn't know about the TCPConnection, so we need to use traditional arguments.
	// 	void HandleIPv4Packet(Devices::NIC::GenericNIC* interface, void* packet, uint64_t length, Library::IPv4Address source, Library::IPv4Address destip);

	// 	uint16_t AllocateEphemeralPort();
	// 	void ReleaseEphemeralPort(uint16_t port);
	// }




	// namespace DHCP
	// {
	// 	// DHCP
	// 	struct DHCPPacket
	// 	{
	// 		uint8_t operation;
	// 		uint8_t hardwaretype;
	// 		uint8_t hardwareaddrlength;
	// 		uint8_t hopcount;

	// 		uint32_t transactionid;
	// 		uint16_t seconds;
	// 		uint16_t flags;

	// 		Library::IPv4Address thisip;
	// 		Library::IPv4Address givenip;
	// 		Library::IPv4Address serverip;
	// 		Library::IPv4Address relayip;

	// 		uint8_t hardwareaddr[16];
	// 		uint8_t servername[64];
	// 		uint8_t bootfilename[128];

	// 		uint8_t cookie[4];

	// 	} __attribute__ ((packed));

	// 	void MonitorThread();
	// 	void Initialise();

	// 	namespace Options
	// 	{
	// 		enum class Message : uint8_t
	// 		{
	// 			DHCPDiscover = 1,
	// 			DHCPOffer = 2,
	// 			DHCPRequest = 3,
	// 			DHCPDecline = 4,
	// 			DHCPAck = 5,
	// 			DHCPNak = 6,
	// 			DHCPRelease = 7,
	// 			DHCPInform = 8
	// 		};

	// 		enum class OptionCode : uint8_t
	// 		{
	// 			SubnetMask = 1,
	// 			Router = 3,
	// 			DNSServer = 6,
	// 			DomainName = 15,
	// 			LeaseTime = 51,
	// 			MessageType = 53,
	// 			DHCPServer = 54,
	// 			ParameterRequest = 55
	// 		};

	// 		struct Option
	// 		{
	// 		};

	// 		struct SubnetMask : public Option
	// 		{
	// 			SubnetMask() : code(OptionCode::SubnetMask), length(4) {}
	// 			OptionCode code;
	// 			uint8_t length;

	// 			Library::IPv4Address mask;

	// 		} __attribute__ ((packed));

	// 		struct Router : public Option
	// 		{
	// 			Router() : code(OptionCode::Router) {}
	// 			OptionCode code;
	// 			uint8_t length;

	// 			Library::IPv4Address mainrouter;

	// 		} __attribute__ ((packed));

	// 		struct DNSServer : public Option
	// 		{
	// 			DNSServer() : code(OptionCode::DNSServer) {}
	// 			OptionCode code;
	// 			uint8_t length;

	// 			Library::IPv4Address mainserver;

	// 		} __attribute__ ((packed));

	// 		struct DomainName : public Option
	// 		{
	// 			DomainName() : code(OptionCode::DomainName) {}
	// 			OptionCode code;
	// 			uint8_t length;

	// 			const char* str;

	// 		} __attribute__ ((packed));

	// 		struct LeaseTime : public Option
	// 		{
	// 			LeaseTime() : code(OptionCode::LeaseTime), length(4) {}
	// 			OptionCode code;
	// 			uint8_t length;

	// 			uint32_t time;

	// 		} __attribute__ ((packed));

	// 		struct MessageType : public Option
	// 		{
	// 			MessageType() : code(OptionCode::MessageType), length(1) {}
	// 			OptionCode code;
	// 			uint8_t length;

	// 			uint8_t type;

	// 		} __attribute__ ((packed));

	// 		struct DHCPServer : public Option
	// 		{
	// 			DHCPServer() : code(OptionCode::DHCPServer), length(4) {}
	// 			OptionCode code;
	// 			uint8_t length;

	// 			Library::IPv4Address addr;

	// 		} __attribute__ ((packed));

	// 		struct ParameterRequest : public Option
	// 		{
	// 			ParameterRequest() : code(OptionCode::ParameterRequest) {}

	// 			OptionCode code;
	// 			uint8_t length;
	// 			uint8_t options[16];

	// 		} __attribute__ ((packed));
	// 	}
	// }

	// namespace DNS
	// {
	// 	struct DNSMessageHeader
	// 	{
	// 		uint16_t id;
	// 		struct
	// 		{
	// 			uint8_t userecursion : 1;
	// 			uint8_t truncated : 1;
	// 			uint8_t authoritative : 1;
	// 			uint8_t opcode : 4;
	// 			uint8_t query : 1;

	// 		} __attribute__ ((packed));

	// 		struct
	// 		{
	// 			uint8_t responsecode : 4;
	// 			uint8_t reserved : 3;
	// 			uint8_t hasrecursion : 1;

	// 		} __attribute__ ((packed));

	// 		uint16_t questions;
	// 		uint16_t answers;
	// 		uint16_t authorities;
	// 		uint16_t extras;

	// 	} __attribute__ ((packed));

	// 	enum class RecordType
	// 	{
	// 		A		= 0x1,
	// 		CNAME		= 0x5,
	// 		AAAA		= 0x1C
	// 	};

	// 	enum class RecordClass
	// 	{
	// 		Internet	= 0x1
	// 	};

	// 	Library::IPv4Address* GetDNSServer();
	// 	void SetDNSServer(Library::IPv4Address addr);

	// 	void Initialise();
	// 	void MonitorThread();
	// 	Library::IPv4Address* QueryDNSv4(Library::string hostname);
	// }



















	// // overall network stuff.
	// // Network.cpp

	// void Initialise();










	// namespace Socket
	// {
	// 	class SocketObj : public Filesystems::VFS::FSObject
	// 	{
	// 		friend class TCP::TCPConnection;

	// 		public:
	// 			SocketObj(Library::IPv4Address source, Library::IPv4Address dest, uint64_t SourcePort, uint64_t DestPort, Library::SocketProtocol prot);
	// 			SocketObj(Library::IPv6Address source, Library::IPv6Address dest, uint64_t SourcePort, uint64_t DestPort, Library::SocketProtocol prot);

	// 			virtual const char* Name() override;
	// 			virtual const char* Path() override;
	// 			virtual FSObject* Parent() override;
	// 			virtual Filesystems::VFS::Filesystem* RootFS() override;
	// 			virtual uint8_t Attributes() override;
	// 			virtual bool Exists() override;

	// 			uint64_t Read(uint8_t* buffer, uint64_t bytes);
	// 			uint64_t Write(uint8_t* buffer, uint64_t bytes);

	// 			Library::CircularMemoryBuffer* recvbuffer;
	// 			Library::IPv4Address ip4source;
	// 			Library::IPv4Address ip4dest;

	// 			Library::IPv6Address ip6source;
	// 			Library::IPv6Address ip6dest;

	// 			Library::SocketProtocol protocol;
	// 			uint64_t clientport;
	// 			uint64_t serverport;

	// 			TCP::TCPConnection* tcpconnection;
	// 			uint64_t packetcount;
	// 			Library::CircularBuffer<uint64_t>* packetsizes;
	// 	};


	// 	uint64_t OpenSocket(uint16_t serverport, Library::SocketProtocol prot);
	// 	uint64_t OpenSocket(uint16_t clientport, uint16_t serverport, Library::SocketProtocol prot);

	// 	uint64_t OpenSocket(Library::IPv4Address dest, uint16_t serverport, Library::SocketProtocol prot);
	// 	uint64_t OpenSocket(Library::IPv4Address dest, uint16_t clientport, uint16_t serverport, Library::SocketProtocol prot);
	// 	uint64_t OpenSocket(Library::IPv4Address source, Library::IPv4Address dest, uint16_t clientport, uint16_t serverport, Library::SocketProtocol prot);

	// 	void ConnectSocket(uint64_t ds);
	// 	void DisconnectSocket(uint64_t ds);

	// 	void CloseSocket(uint64_t ds);

	// 	uint64_t ReadFromSocket(uint64_t ds, uint8_t* buf, uint64_t bytes);
	// 	uint64_t WriteToSocket(uint64_t ds, uint8_t* buf, uint64_t bytes);

	// 	uint64_t BytesInSocket(uint64_t ds);

	// 	uint64_t PacketsInSocket(uint64_t ds);
	// 	uint64_t NextPacketSize(uint64_t ds);
	// }
}
}
}




