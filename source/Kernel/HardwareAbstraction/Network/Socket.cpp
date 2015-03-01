// Socket.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <String.hpp>
#include <StandardIO.hpp>
#include <HardwareAbstraction/Network.hpp>

#include <defs/_errnos.h>

using namespace Library;
using namespace Kernel::HardwareAbstraction::Filesystems::VFS;

namespace Kernel {
namespace HardwareAbstraction {
namespace Network
{
	/*
		TODO: unfuck sockets.
		Sockets shouldn't be FSDrivers! There should be a SocketIPC FSDriver that handles data
		through vnode->fsref->info

		I always had a lingering suspicion that something was wrong when I wasn't using any of the vnode*s passed
		to the Socket::* functions.

		the Socket class itself can probably stay the same, with instance methods moving out into functions operating
		on vnodes. In this case vnode->fsref->info will probably be a Socket* casted as void*.

		contemplate
	*/













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
		{
			Log(1, "File descriptor ID '%ld' does not exist!", fd);
			Multitasking::SetThreadErrno(EBADF);
			return 0;
		}

		assert(fe->node);

		if(fe->node->info->driver->GetType() != FSDriverType::Socket)
		{
			Log(1, "Cannot perform socket operations on a non-socket FD!");
			Multitasking::SetThreadErrno(ENOTSOCK);

			return 0;
		}

		return fe->node;
	}

	static Socket* getsockdata(vnode* node)
	{
		assert(node);
		assert(node->info);
		assert(node->info->data);

		return (Socket*) node->info->data;
	}




	// file descriptor stuff.
	fd_t OpenSocket(SocketProtocol prot, uint64_t flags)
	{
		(void) flags;

		if(prot == SocketProtocol::RawIPv6)
			return 0;

		Devices::NIC::GenericNIC* interface = (Devices::NIC::GenericNIC*) Devices::DeviceManager::GetDevice(Devices::DeviceType::EthernetNIC);
		assert(interface);


		// socket driver init

		Filesystem* socketfs = VFS::GetFilesystemAtPath(VFS::FS_SOCKET_MOUNTPOINT);
		assert(socketfs);
		assert(socketfs->driver);

		assert(socketfs->driver->GetType() == FSDriverType::Socket);

		if(!((SocketVFS*) socketfs->driver)->interface)
			((SocketVFS*) socketfs->driver)->interface = interface;

		vnode* node = VFS::CreateNode(socketfs->driver);
		assert(node);

		node->type = VNodeType::Socket;
		fileentry* fe = VFS::Open(getctx(), node, 0);
		assert(fe);

		Socket* socket = new Socket(GLOBAL_MTU * 8, prot);
		node->info->data = (void*) socket;

		return fe->fd;
	}

	void CloseSocket(fd_t fd)
	{
		vnode* node = getvnode(fd);
		((SocketVFS*) node->info->driver)->Close(node);
	}


	err_t BindSocket(fd_t fd, Library::IPv4Address local, uint16_t port)
	{
		vnode* node = getvnode(fd);
		((SocketVFS*) node->info->driver)->Bind(node, local, port);
		return 0;
	}

	err_t ConnectSocket(fd_t fd, Library::IPv4Address remote, uint16_t port)
	{
		vnode* node = getvnode(fd);
		((SocketVFS*) node->info->driver)->Connect(node, remote, port);
		return 0;
	}

	size_t ReadSocket(fd_t socket, void* buf, size_t bytes)
	{
		vnode* node = getvnode(socket);
		return ((SocketVFS*) node->info->driver)->Read(node, buf, 0, bytes);
	}

	size_t WriteSocket(fd_t socket, void* buf, size_t bytes)
	{
		vnode* node = getvnode(socket);
		return ((SocketVFS*) node->info->driver)->Write(node, buf, 0, bytes);
	}

	size_t GetSocketBufferFill(fd_t socket)
	{
		vnode* node = getvnode(socket);
		return ((Socket*) node->info->data)->recvbuffer.ByteCount();
	}













	// actual class implementation
	SocketVFS::SocketVFS() : FSDriver(nullptr, Filesystems::FSDriverType::Socket)
	{
		this->_seekable = false;
	}

	SocketVFS::~SocketVFS()
	{
	}

	bool SocketVFS::Create(Filesystems::VFS::vnode*, const char*, uint64_t, uint64_t)
	{
		return false;
	}

	// bind address to local
	void SocketVFS::Bind(vnode* node, IPv4Address local, uint16_t localport)
	{
		Socket* skt = getsockdata(node);

		if(skt->ip4source.raw != 0 || skt->clientport != 0)
		{
			Log(1, "Socket is already bound, ignoring");
			Multitasking::SetThreadErrno(EINVAL);
			return;
		}


		// cannot bind shit.
		// make sure port not already in use
		if(localport == 0)
		{
			if(skt->protocol == SocketProtocol::TCP)
				skt->clientport = TCP::AllocateEphemeralPort();

			else if(skt->protocol == SocketProtocol::UDP)
				skt->clientport = UDP::AllocateEphemeralPort();

			else
				Log("Unsupported protocol, WTF are you doing?!");
		}
		else
		{
			skt->clientport = localport;
		}

		// if source is zero, use local
		if(local.raw == 0)
		{
			skt->ip4source = IP::GetIPv4Address();
		}
		else
		{
			skt->ip4source = local;
		}
	}

	void SocketVFS::Connect(vnode* node, IPv4Address remote, uint16_t remoteport)
	{
		Socket* skt = getsockdata(node);

		if(skt->ip4dest.raw != 0 || skt->serverport != 0)
		{
			Log(1, "Socket is already connected, ignoring");
			Multitasking::SetThreadErrno(EISCONN);
			return;
		}


		skt->ip4dest = remote;
		skt->serverport = remoteport;

		// ipv4
		switch(skt->protocol)
		{
			case SocketProtocol::RawIPv4:
				// no ports for this
				IP::MapIPv4Socket(SocketIPv4Mapping { skt->ip4source, skt->ip4dest }, skt);
				break;

			case SocketProtocol::TCP:
				TCP::MapSocket(SocketFullMappingv4 { IPv4PortAddress { skt->ip4source, (uint16_t) skt->clientport },
					IPv4PortAddress { skt->ip4dest, (uint16_t) skt->serverport } }, skt);
				break;

			case SocketProtocol::UDP:
				UDP::MapSocket(SocketFullMappingv4 { IPv4PortAddress { skt->ip4source, (uint16_t) skt->clientport },
					IPv4PortAddress { skt->ip4dest, (uint16_t) skt->serverport } }, skt);
				break;

			default:
				Log(1, "Unknown protocol, ignoring connect()");
				break;
		}

		if(skt->protocol == SocketProtocol::TCP)
		{
			skt->tcpconnection = new TCP::TCPConnection(skt, skt->ip4dest, (uint16_t) skt->clientport, (uint16_t) skt->serverport);
			skt->tcpconnection->Connect();
		}
	}


	// note: the overloads of Socket::Bind and Socket::Connect taking const char* paths (for AF_UNIX IPC sockets)
	// are implemented in HardwareAbstraction/Filesystems/FSDrivers/SocketIPC.cpp



	void SocketVFS::Close(Filesystems::VFS::vnode* node)
	{
		Socket* skt = getsockdata(node);

		switch(skt->protocol)
		{
			case Library::SocketProtocol::RawIPv4:
				// no ports for this
				IP::UnmapIPv4Socket(SocketIPv4Mapping { skt->ip4source, skt->ip4dest });
				break;

			case Library::SocketProtocol::TCP:
				TCP::UnmapSocket(SocketFullMappingv4 { IPv4PortAddress { skt->ip4source, (uint16_t) skt->clientport },
					IPv4PortAddress { skt->ip4dest, (uint16_t) skt->serverport } });
				break;

			case Library::SocketProtocol::UDP:
				UDP::UnmapSocket(SocketFullMappingv4 { IPv4PortAddress { skt->ip4source, (uint16_t) skt->clientport },
					IPv4PortAddress { skt->ip4dest, (uint16_t) skt->serverport } });
				break;

			case Library::SocketProtocol::IPC:
				// nothing to do
				break;

			default:
				HALT("???");
		}

		// this will close the TCP connection
		if(skt->protocol == SocketProtocol::TCP)
			delete skt->tcpconnection;
	}

	size_t SocketVFS::Read(Filesystems::VFS::vnode* node, void* buf, off_t offset, size_t length)
	{
		(void) offset;
		Socket* skt = getsockdata(node);

		uint64_t ret = __min(skt->recvbuffer.ByteCount(), length);
		skt->recvbuffer.Read((uint8_t*) buf, length);
		return ret;
	}

	size_t SocketVFS::Write(Filesystems::VFS::vnode* node, const void* buf, off_t offset, size_t length)
	{
		(void) offset;
		Socket* skt = getsockdata(node);

		// tcp is special because we need to do stupid things in the TCPConnection class.
		if(skt->protocol == SocketProtocol::TCP)
		{
			skt->tcpconnection->SendUserPacket((uint8_t*) buf, (uint16_t) length);
		}
		else if(skt->protocol == SocketProtocol::UDP)
		{
			// udp is simpler.
			if(length > GLOBAL_MTU)
			{
				Log(1, "Tried to send a UDP packet larger than the MTU, which is illegal");
				return 0;
			}

			UDP::SendIPv4Packet(skt->interface, (uint8_t*) buf, (uint16_t) length, skt->ip4dest,
				(uint16_t) skt->clientport, (uint16_t) skt->serverport);
		}
		else if(skt->protocol == SocketProtocol::IPC)
		{
			skt->recvbuffer.Write((uint8_t*) buf, length);
		}

		return length;
	}

	bool SocketVFS::Delete(Filesystems::VFS::vnode*, const char*)
	{
		return false;
	}

	bool SocketVFS::Traverse(Filesystems::VFS::vnode*, const char*, char**)
	{
		return false;
	}

	void SocketVFS::Flush(Filesystems::VFS::vnode*)
	{

	}

	void SocketVFS::Stat(Filesystems::VFS::vnode*, struct stat*, bool)
	{

	}

	rde::vector<Filesystems::VFS::vnode*> SocketVFS::ReadDir(Filesystems::VFS::vnode*)
	{
		return rde::vector<Filesystems::VFS::vnode*>();
	}
}
}
}
