// Socket.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <String.hpp>
#include <StandardIO.hpp>
#include <HardwareAbstraction/Network.hpp>

using namespace Library;
using namespace Kernel::HardwareAbstraction::Filesystems::VFS;

namespace Kernel {
namespace HardwareAbstraction {
namespace Network
{
	using namespace Filesystems;

	static IOContext* getctx()
	{
		auto proc = Multitasking::GetCurrentProcess();
		assert(proc);

		return &proc->iocontext;
	}



	fd_t OpenSocket(uint16_t destport, Library::SocketProtocol prot)
	{
		return OpenSocket(IPv4Address { 0 }, IPv4Address { 0xFFFFFFFF }, 0, destport, prot);
	}

	fd_t OpenSocket(uint16_t sourceport, uint16_t destport, Library::SocketProtocol prot)
	{
		return OpenSocket(IPv4Address { 0 }, IPv4Address { 0xFFFFFFFF }, sourceport, destport, prot);
	}

	fd_t OpenSocket(IPv4Address dest, uint16_t destport, SocketProtocol prot)
	{
		return OpenSocket(IP::GetIPv4Address(), dest, 0, destport, prot);
	}

	fd_t OpenSocket(IPv4Address dest, uint16_t sourceport, uint16_t destport, SocketProtocol prot)
	{
		return OpenSocket(IP::GetIPv4Address(), dest, sourceport, destport, prot);
	}

	// file descriptor stuff.
	// TODO: clean up duplication
	fd_t OpenSocket(IPv4Address source, IPv4Address dest, uint16_t sourceport, uint16_t destport, SocketProtocol prot)
	{
		if(prot == SocketProtocol::RawIPv6)
			return 0;

		Devices::NIC::GenericNIC* interface = (Devices::NIC::GenericNIC*) Devices::DeviceManager::GetDevice(Devices::DeviceType::EthernetNIC);
		assert(interface);


		// socket driver init
		Socket* socket = new Socket(interface, source, dest, sourceport, destport, prot);

		vnode* node = VFS::CreateNode(socket);
		assert(node);

		node->type = VNodeType::Socket;
		VFS::Open(getctx(), node, 0);



		// ipv4
		switch(prot)
		{
			case Library::SocketProtocol::RawIPv4:
				// no ports for this
				IP::MapIPv4Socket(SocketIPv4Mapping { source, dest }, socket);
				break;

			case Library::SocketProtocol::TCP:
				if(TCP::IsDuplicatePort(sourceport))
				{
					Log(1, "Attempted to open TCP connection on port %d, which is already in use", sourceport);
					return -1;
				}
				TCP::MapSocket(SocketFullMappingv4 { IPv4PortAddress { source, sourceport }, IPv4PortAddress { dest, destport } }, socket);
				break;

			case Library::SocketProtocol::UDP:
				// UDP::udpsocketmapv4->Put(SocketFullMappingv4 { IPv4PortAddress { source, sourceport }, IPv4PortAddress { dest, destport } }, *socket);
				break;

			default:
				break;
		}

		Log("Opened socket (%d.%d.%.d.%d : %d => %d.%d.%d.%d : %d)", source.b1, source.b2, source.b3, source.b4, sourceport, dest.b1, dest.b2,
			dest.b3, dest.b4, destport);
		// return ret;

		return 0;
	}





	void CloseSocket(fd_t ds)
	{
		// Multitasking::Process* proc = Multitasking::GetCurrentProcess();
		// Socket* socket = VerfiySocket(ds);

		// switch(socket->protocol)
		// {
		// 	case Library::SocketProtocol::RawIPv4:
		// 	{
		// 		IP::ipv4socketmap->Remove(SocketIPv4Mapping { socket->ip4source, socket->ip4dest });
		// 		break;
		// 	}

		// 	case Library::SocketProtocol::RawIPv6:
		// 		break;

		// 	case Library::SocketProtocol::TCP:
		// 	{
		// 		if(socket->ip4source.raw != 0)
		// 			TCP::tcpsocketmapv4->Remove(SocketFullMappingv4 { IPv4PortAddress { socket->ip4source, (uint16_t) socket->clientport },
		// 				IPv4PortAddress { socket->ip4dest, (uint16_t)  socket->serverport } });
		// 		else
		// 			TCP::tcpsocketmapv6->Remove(SocketFullMappingv6 { IPv6PortAddress { socket->ip6source, (uint16_t) socket->clientport },
		// 				IPv6PortAddress { socket->ip6dest, (uint16_t)  socket->serverport } });

		// 		// delete the socket's tcpconnection
		// 		delete socket->tcpconnection;

		// 		break;
		// 	}

		// 	case Library::SocketProtocol::UDP:
		// 	{
		// 		if(socket->ip4source.raw != 0)
		// 			UDP::udpsocketmapv4->Remove(SocketFullMappingv4 { IPv4PortAddress { socket->ip4source, (uint16_t) socket->clientport },
		// 				IPv4PortAddress { socket->ip4dest, (uint16_t)  socket->serverport } });

		// 		else
		// 			UDP::udpsocketmapv6->Remove(SocketFullMappingv6 { IPv6PortAddress { socket->ip6source, (uint16_t) socket->clientport },
		// 				IPv6PortAddress { socket->ip6dest, (uint16_t)  socket->serverport } });

		// 		break;
		// 	}
		// }

		// // delete the object.
		// proc->CurrentFDIndex = ds;
		// delete (Socket*) proc->FileDescriptors[ds].Pointer;
	}




	// actual class implementation
	Socket::Socket(Devices::NIC::GenericNIC* iface, IPv4Address source, IPv4Address dest, uint64_t spt, uint64_t dpt, Library::SocketProtocol prot) : FSDriver(nullptr, Filesystems::FSDriverType::Virtual), recvbuffer(GLOBAL_MTU * 8)
	{
		this->ip4source = source;
		this->ip4dest = dest;

		this->ip6source.high = 0;
		this->ip6source.low = 0;
		this->ip6dest.high = 0;
		this->ip6dest.low = 0;

		this->clientport = spt;
		this->serverport = dpt;
		this->protocol = prot;

		if(prot == SocketProtocol::TCP)
			this->tcpconnection = new TCP::TCPConnection(this, dest, (uint16_t) spt, (uint16_t) dpt);

		this->_seekable = false;
	}


	size_t Socket::Read(Filesystems::VFS::vnode* node, void* buf, off_t offset, size_t length)
	{
		(void) offset;
		(void) node;

		uint64_t ret = __min(this->recvbuffer.ByteCount(), length);
		this->recvbuffer.Read((uint8_t*) buf, length);
		return ret;
	}

	size_t Socket::Write(Filesystems::VFS::vnode* node, const void* buf, off_t offset, size_t length)
	{
		// tcp is special because we need to do stupid things in the TCPConnection class.
		if(this->protocol == SocketProtocol::TCP)
		{
			if(this->ip6source.high == 0 && this->ip6dest.low == 0 && this->ip6dest.high == 0 && this->ip6dest.low == 0)
			{
				this->tcpconnection->SendUserPacket((uint8_t*) buf, (uint16_t) length);
			}
		}
		else if(this->protocol == SocketProtocol::UDP)
		{
			// udp is simpler.
			if(length > GLOBAL_MTU)
				HALT("can't do that");

			if(this->ip6source.high == 0 && this->ip6dest.low == 0 && this->ip6dest.high == 0 && this->ip6dest.low == 0)
			{
				// UDP::SendIPv4Packet(this->interface, (uint8_t*) buf, (uint16_t) length, this->ip4dest, (uint16_t) this->clientport, (uint16_t) this->serverport);
			}
		}

		return length;
	}

	bool Socket::Create(Filesystems::VFS::vnode*, const char*, uint64_t, uint64_t)
	{
		return false;
	}

	bool Socket::Delete(Filesystems::VFS::vnode*, const char*)
	{
		return false;
	}

	bool Socket::Traverse(Filesystems::VFS::vnode*, const char*, char**)
	{
		return false;
	}

	void Socket::Flush(Filesystems::VFS::vnode*)
	{

	}

	void Socket::Stat(Filesystems::VFS::vnode*, struct stat*, bool)
	{

	}

	rde::vector<Filesystems::VFS::vnode*> Socket::ReadDir(Filesystems::VFS::vnode* node)
	{
		return rde::vector<Filesystems::VFS::vnode*>();
	}
}
}
}
