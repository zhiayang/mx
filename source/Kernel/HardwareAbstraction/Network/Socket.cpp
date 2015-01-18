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

	static vnode* getvnode(fd_t fd)
	{
		auto ctx = getctx();
		fileentry* fe = VFS::FileEntryFromFD(ctx, fd);
		if(fe == nullptr)
			return 0;

		assert(fe->node);

		if(fe->node->info->driver->GetType() != FSDriverType::Socket)
		{
			Log(1, "Cannot perform socket operations on a non-socket FD!");
			return 0;
			// todo: errno
		}

		return fe->node;
	}

	// file descriptor stuff.
	// TODO: clean up duplication
	fd_t OpenSocket(SocketProtocol prot, uint64_t flags)
	{
		(void) flags;

		if(prot == SocketProtocol::RawIPv6)
			return 0;

		Devices::NIC::GenericNIC* interface = (Devices::NIC::GenericNIC*) Devices::DeviceManager::GetDevice(Devices::DeviceType::EthernetNIC);
		assert(interface);


		// socket driver init
		Socket* socket = new Socket(interface, prot);

		vnode* node = VFS::CreateNode(socket);
		assert(node);

		node->type = VNodeType::Socket;
		fileentry* fe = VFS::Open(getctx(), node, 0);
		assert(fe);

		return fe->fd;
	}

	void CloseSocket(fd_t)
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


	err_t BindSocket(fd_t fd, Library::IPv4Address local, uint16_t port)
	{
		vnode* node = getvnode(fd);
		((Socket*) node->info->driver)->Bind(node, local, port);
		return 0;
	}

	err_t ConnectSocket(fd_t fd, Library::IPv4Address remote, uint16_t port)
	{
		vnode* node = getvnode(fd);
		((Socket*) node->info->driver)->Connect(node, remote, port);
		return 0;
	}

	size_t ReadSocket(fd_t socket, void* buf, size_t bytes)
	{
		vnode* node = getvnode(socket);
		return ((Socket*) node->info->driver)->Read(node, buf, 0, bytes);
	}

	size_t WriteSocket(fd_t socket, void* buf, size_t bytes)
	{
		vnode* node = getvnode(socket);
		return ((Socket*) node->info->driver)->Write(node, buf, 0, bytes);
	}

	size_t GetSocketBufferFill(fd_t socket)
	{
		vnode* node = getvnode(socket);
		return ((Socket*) node->info->driver)->recvbuffer.ByteCount();
	}

	// actual class implementation
	Socket::Socket(Devices::NIC::GenericNIC* iface, Library::SocketProtocol prot) : FSDriver(nullptr, Filesystems::FSDriverType::Socket), recvbuffer(GLOBAL_MTU * 8)
	{
		// this->ip4source = source;
		// this->ip4dest = dest;

		this->ip6source.high = 0;
		this->ip6source.low = 0;
		this->ip6dest.high = 0;
		this->ip6dest.low = 0;

		// this->clientport = spt;
		// this->serverport = dpt;
		this->protocol = prot;
		this->_seekable = false;
		this->interface = iface;
	}

	Socket::~Socket()
	{
	}

	// bind address to local
	void Socket::Bind(vnode* node, IPv4Address local, uint16_t localport)
	{
		(void) node;

		// cannot bind shit.
		// make sure port not already in use
		if(localport == 0)
		{
			// todo: handle UDP ephemeral port
			this->clientport = TCP::AllocateEphemeralPort();
		}
		else if(this->protocol == SocketProtocol::TCP ? !TCP::IsDuplicatePort(localport) : !UDP::IsDuplicatePort(localport))
		{
			this->clientport = localport;
		}
		else
		{
			Log(1, "Port already in use, cannot bind()!");

			// todo: set errno
			return;
		}

		// if source is zero, use local
		if(local.raw == 0)
		{
			this->ip4source = IP::GetIPv4Address();
		}
		else
		{
			this->ip4source = local;
		}
	}

	void Socket::Connect(vnode* node, IPv4Address remote, uint16_t remoteport)
	{
		(void) node;

		this->ip4dest = remote;
		this->serverport = remoteport;

		// ipv4
		switch(this->protocol)
		{
			case Library::SocketProtocol::RawIPv4:
				// no ports for this
				IP::MapIPv4Socket(SocketIPv4Mapping { this->ip4source, this->ip4dest }, this);
				break;

			case Library::SocketProtocol::TCP:
				TCP::MapSocket(SocketFullMappingv4 { IPv4PortAddress { this->ip4source, (uint16_t) this->clientport },
					IPv4PortAddress { this->ip4dest, (uint16_t) this->serverport } }, this);
				break;

			case Library::SocketProtocol::UDP:
				UDP::MapSocket(SocketFullMappingv4 { IPv4PortAddress { this->ip4source, (uint16_t) this->clientport },
					IPv4PortAddress { this->ip4dest, (uint16_t) this->serverport } }, this);
				break;

			default:
				Log(1, "Unknown protocol, ignoring connect()");
				break;
		}


		if(this->protocol == SocketProtocol::TCP)
			this->tcpconnection = new TCP::TCPConnection(this, this->ip4dest, (uint16_t) this->clientport, (uint16_t) this->serverport);
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
		(void) node;
		(void) offset;

		// tcp is special because we need to do stupid things in the TCPConnection class.
		if(this->protocol == SocketProtocol::TCP)
		{
			this->tcpconnection->SendUserPacket((uint8_t*) buf, (uint16_t) length);
		}
		else if(this->protocol == SocketProtocol::UDP)
		{
			// udp is simpler.
			if(length > GLOBAL_MTU)
			{
				Log(1, "Tried to send a UDP packet larger than the MTU, which is illegal");
				return 0;
			}

			UDP::SendIPv4Packet(this->interface, (uint8_t*) buf, (uint16_t) length, this->ip4dest,
				(uint16_t) this->clientport, (uint16_t) this->serverport);
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

	rde::vector<Filesystems::VFS::vnode*> Socket::ReadDir(Filesystems::VFS::vnode*)
	{
		return rde::vector<Filesystems::VFS::vnode*>();
	}
}
}
}
