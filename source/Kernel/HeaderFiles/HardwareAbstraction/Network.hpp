// Network.hpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <stdint.h>
#include <CircularBuffer.hpp>
#include <HardwareAbstraction/Devices/NIC.hpp>
#include <HardwareAbstraction/Filesystems.hpp>
#include <HardwareAbstraction/Filesystems/FSUtil.hpp>
#include <orionx/PacketNetwork.hpp>
#pragma once

#define SwapEndian16(Val)	((uint16_t)(((Val) & 0xFF) << 8) | (((Val) >> 8) & 0xFF))
#define SwapEndian32(Val)	((((Val) & 0xFF) << 24) | (((Val) & 0xFF00) << 8) | (((Val) >> 8) & 0xFF00) | (((Val) >> 24) & 0xFF))

#define GLOBAL_MTU				2048
#define EPHEMERAL_PORT_RANGE	49152

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
		uint8_t mac[6] = { 0 };
		bool isZero() { return (this->mac[0] == 0) && (this->mac[1] == 0) && (this->mac[2] == 0)
							&& (this->mac[3] == 0) && (this->mac[4] == 0) && (this->mac[5] == 0); }

	} __attribute__ ((packed));

	struct SocketIPv4Mapping
	{
		Library::IPv4Address source;
		Library::IPv4Address dest;

		explicit SocketIPv4Mapping() { }
		SocketIPv4Mapping(Library::IPv4Address s, Library::IPv4Address d) { this->source = s; this->dest = d; }

		operator uint32_t () const
		{
			return (this->source & this->dest);
		}

		bool operator==(SocketIPv4Mapping other) const
		{
			return this->source == other.source && this->dest == other.dest;
		}
	};



	class Socket;
	namespace Ethernet
	{
		// Ethernet
		enum class EtherType : uint16_t
		{
			IPv4	= 0x0800,
			ARP		= 0x0806,
			IPv6	= 0x86DD
		};

		struct EthernetFrameHeader
		{
			EUI48Address destmac;
			EUI48Address sourcemac;
			uint16_t ethertype;

		} __attribute__((packed));

		void SendPacket(Devices::NIC::GenericNIC* interface, void* data, uint16_t length, EtherType type, EUI48Address destmac);
		void HandlePacket(Devices::NIC::GenericNIC* interface, void* packet, uint64_t length);
	}


	namespace ARP
	{
		EUI48Address GetGatewayMAC();
		void SetGatewayMAC(EUI48Address addr);
		EUI48Address SendQuery(Devices::NIC::GenericNIC* interface, Library::IPv4Address addr);
		void HandlePacket(Devices::NIC::GenericNIC* interface, void* PacketAddr, uint64_t length);
		void SendPacket(Devices::NIC::GenericNIC* interface, Library::IPv4Address addr);

		void Initialise();
	}






	struct SocketFullMappingv4
	{
		Library::IPv4PortAddress source;
		Library::IPv4PortAddress dest;

		operator uint32_t () const
		{
			return this->source.ip.raw + this->dest.ip.raw;
		}

		bool operator==(SocketFullMappingv4 other) const
		{
			return (this->source.ip.raw == other.source.ip.raw) && (this->source.port == other.source.port)
					&& (this->dest.ip.raw == other.dest.ip.raw) && (this->dest.port == other.dest.port);
		}
	};








	namespace IP
	{
		enum class ProtocolType : uint8_t
		{
			ICMP = 0x1,
			TCP = 0x6,
			UDP = 0x11
		};

		// IPv4
		struct IPv4Packet
		{
			struct
			{
				// Spec says Version is first, but stupid bit ordering
				uint8_t HeaderLength:		4;	// in 4-byte chunks
				uint8_t Version:			4;	// = 4

			} __attribute__((packed));

			uint8_t DSCPAndECN;
			uint16_t TotalLength;

			uint16_t Identification;
			uint16_t FlagsAndFragmentOffset;

			uint8_t HopCount;
			uint8_t Protocol;
			uint16_t HeaderChecksum;

			Library::IPv4Address SourceIPAddress;
			Library::IPv4Address DestIPAddress;

		} __attribute__((packed));

		struct PseudoIPv4Header
		{
			Library::IPv4Address source;
			Library::IPv4Address dest;
			uint8_t zeroes;

			uint8_t protocol;
			uint16_t length;

		} __attribute__((packed));

		uint16_t CalculateIPChecksum(const void* buf, uint64_t Length);
		uint16_t CalculateIPChecksum_Partial(uint16_t prev, const void* buf, uint64_t length);
		uint16_t CalculateIPChecksum_Finalise(uint16_t Value);

		Library::IPv4Address GetIPv4Address();
		Library::IPv4Address GetSubnetMask();
		Library::IPv4Address GetGatewayIP();
		void SetIPv4Address(Library::IPv4Address newaddr);
		void SetSubnetMask(Library::IPv4Address addr);
		void SetGatewayIP(Library::IPv4Address addr);


		void SendIPv4Packet(Devices::NIC::GenericNIC* interface, void* packet, uint16_t length, uint16_t id, Library::IPv4Address dest, ProtocolType prot);
		void HandleIPv4Packet(Devices::NIC::GenericNIC* interface, void* packet, uint64_t length);

		void MapIPv4Socket(SocketIPv4Mapping addr, Socket* s);
		void UnmapIPv4Socket(SocketIPv4Mapping addr);

		void Initialise();
	}

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


	namespace UDP
	{
		void SendIPv4Packet(Devices::NIC::GenericNIC* interface, uint8_t* packet, uint64_t length, Library::IPv4Address dest, uint16_t sourceport, uint16_t destport);
		void HandleIPv4Packet(Devices::NIC::GenericNIC* interface, void* packet, uint64_t length, Library::IPv4Address source, Library::IPv4Address destip);

		void MapSocket(SocketFullMappingv4 addr, Socket* s);
		void UnmapSocket(SocketFullMappingv4 addr);

		uint16_t AllocateEphemeralPort();
		void ReleaseEphemeralPort(uint16_t port);
		bool IsDuplicatePort(uint16_t port);

		void Initialise();
	}



	namespace TCP
	{
		// Because TCP is a connection-oriented interface,
		// it makes more sense to create a 'TCPConnection' object
		// that handles things such as sequence numbers, ack numbers
		// and opening/closing the connection.

		enum class ConnectionError
		{
			NoError,
			Timeout
		};

		enum class ConnectionState;

		class TCPConnection
		{
			friend void TCPAckMonitor(TCPConnection*);

			public:
				TCPConnection(Socket* socket, Library::IPv4Address dest, uint16_t srcport, uint16_t destport);
				TCPConnection(Socket* socket, Library::IPv6Address dest, uint16_t srcport, uint16_t destport);
				~TCPConnection();

				uint16_t clientport;
				uint16_t serverport;

				Socket* socket;

				Library::IPv4Address sourceip4;
				Library::IPv4Address destip4;

				Library::IPv6Address sourceip6;
				Library::IPv6Address destip6;

				void SendUserPacket(uint8_t* packet, uint64_t bytes);

				void HandleIncoming(uint8_t* packet, uint64_t bytes, uint64_t HeaderSize);
				void SendPacket(uint8_t* packet, uint64_t bytes);

				ConnectionError Connect();
				void Disconnect();

			private:
				TCPConnection(Socket* socket, uint16_t srcport, uint16_t destport);

				ConnectionError error;
				uint32_t localcumlsequence;
				uint32_t localsequence;
				uint32_t serversequence;
				uint32_t servercumlsequence;
				uint64_t uuid;
				uint16_t maxsegsize;
				uint32_t nextack;
				uint64_t bufferfill;
				uint64_t lastpackettime;
				uint64_t workertid;

				bool AlreadyAcked;
				bool PacketReceived;

				ConnectionState state;
				Library::CircularMemoryBuffer packetbuffer;

				void SendPacket(uint8_t* packet, uint64_t length, uint8_t flags);
				void SendIPv4Packet(uint8_t* packet, uint64_t length, uint8_t flags);
				void SendIPv6Packet(uint8_t* packet, uint64_t length, uint8_t flags);
		};

		struct TCPPacket
		{
			uint16_t clientport;
			uint16_t serverport;
			uint32_t sequence;
			uint32_t ackid;

			uint8_t HeaderLength;
			uint8_t Flags;
			uint16_t WindowSize;

			uint16_t Checksum;
			uint16_t UrgentPointer;

		} __attribute__ ((packed));

		// much simpler, because much of it is abstracted behind TCPConnection.
		void SendIPv4Packet(TCPConnection* connection, uint8_t* packet, uint64_t length);

		// the IPv4 layer doesn't know about the TCPConnection, so we need to use traditional arguments.
		void HandleIPv4Packet(Devices::NIC::GenericNIC* interface, void* packet, uint64_t length, Library::IPv4Address source, Library::IPv4Address destip);

		void MapSocket(SocketFullMappingv4 addr, Socket* s);
		void UnmapSocket(SocketFullMappingv4 addr);

		uint16_t AllocateEphemeralPort();
		void ReleaseEphemeralPort(uint16_t port);
		bool IsDuplicatePort(uint16_t port);

		void Initialise();
	}




	namespace DHCP
	{
		void MonitorThread();
		void Initialise(Devices::NIC::GenericNIC* interface);
	}

	namespace DNS
	{
		Library::IPv4Address GetDNSServer();
		void SetDNSServer(Library::IPv4Address addr);

		void Initialise();
		void MonitorThread();
		Library::IPv4Address QueryDNSv4(rde::string hostname);
	}



















	// // overall network stuff.
	// // Network.cpp

	// void Initialise();











	class Socket : public Filesystems::FSDriver
	{
		public:
			Socket(Devices::NIC::GenericNIC* interface, Library::SocketProtocol prot);

			virtual ~Socket() override;
			virtual bool Create(Filesystems::VFS::vnode* node, const char* path, uint64_t flags, uint64_t perms) override;
			virtual bool Delete(Filesystems::VFS::vnode* node, const char* path) override;
			virtual bool Traverse(Filesystems::VFS::vnode* node, const char* path, char** symlink) override;
			virtual size_t Read(Filesystems::VFS::vnode* node, void* buf, off_t offset, size_t length) override;
			virtual size_t Write(Filesystems::VFS::vnode* node, const void* buf, off_t offset, size_t length) override;
			virtual void Flush(Filesystems::VFS::vnode* node) override;
			virtual void Stat(Filesystems::VFS::vnode* node, struct stat* stat, bool statlink) override;

			// returns a list of items inside the directory, as vnodes.
			virtual rde::vector<Filesystems::VFS::vnode*> ReadDir(Filesystems::VFS::vnode* node) override;


			// unix-isms that we'll just have to implement for an easier time in userspace
			void Connect(Filesystems::VFS::vnode* node, Library::IPv4Address remote, uint16_t remoteport);		// remote address
			void Bind(Filesystems::VFS::vnode* node, Library::IPv4Address local, uint16_t localport);			// local address

			void Connect(Filesystems::VFS::vnode* node, const char* path);
			void Bind(Filesystems::VFS::vnode* node, const char* path);

			void Close(Filesystems::VFS::vnode* node);


			Library::CircularMemoryBuffer recvbuffer;
			Library::IPv4Address ip4source;
			Library::IPv4Address ip4dest;

			Library::IPv6Address ip6source;
			Library::IPv6Address ip6dest;

			Library::SocketProtocol protocol;
			uint64_t clientport;
			uint64_t serverport;

			TCP::TCPConnection* tcpconnection;
			Devices::NIC::GenericNIC* interface;
	};

	fd_t OpenSocket(Library::SocketProtocol prot, uint64_t flags);
	void CloseSocket(fd_t ds);

	err_t BindSocket(fd_t socket, Library::IPv4Address local, uint16_t port);
	err_t ConnectSocket(fd_t socket, Library::IPv4Address remote, uint16_t port);

	size_t ReadSocket(fd_t socket, void* buf, size_t bytes);
	size_t WriteSocket(fd_t socket, void* buf, size_t bytes);
	size_t GetSocketBufferFill(fd_t socket);
}
}
}
















