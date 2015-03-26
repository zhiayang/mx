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

	static void addProcToBlockingList(Socket* skt)
	{
		assert(skt);
		pid_t pid = Multitasking::GetCurrentProcess()->ProcessID;
		if(!skt->blockingProcs.contains(pid))
			skt->blockingProcs.push_back(pid);
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
		if(!node) return;

		((SocketVFS*) node->info->driver)->Close(node);
	}


	err_t BindSocket(fd_t fd, Library::IPv4Address local, uint16_t port)
	{
		vnode* node = getvnode(fd);
		if(!node) return -1;

		((SocketVFS*) node->info->driver)->Bind(node, local, port);
		return 0;
	}

	err_t ConnectSocket(fd_t fd, Library::IPv4Address remote, uint16_t port)
	{
		vnode* node = getvnode(fd);
		if(!node) return -1;

		((SocketVFS*) node->info->driver)->Connect(node, remote, port);
		return 0;
	}

	err_t BindSocket(fd_t fd, const char* path)
	{
		vnode* node = getvnode(fd);
		if(!node) return -1;

		((SocketVFS*) node->info->driver)->Bind(node, path);
		return 0;
	}

	err_t ConnectSocket(fd_t fd, const char* path)
	{
		vnode* node = getvnode(fd);
		if(!node) return -1;

		((SocketVFS*) node->info->driver)->Connect(node, path);
		return 0;
	}








	size_t ReadSocket(fd_t socket, void* buf, size_t bytes)
	{
		vnode* node = getvnode(socket);
		if(!node) return -1;

		return ((SocketVFS*) node->info->driver)->Read(node, buf, 0, bytes);
	}

	size_t ReadSocketBlocking(fd_t socket, void* buf, size_t bytes)
	{
		vnode* node = getvnode(socket);
		if(!node) return -1;

		return ((SocketVFS*) node->info->driver)->BlockingRead(node, buf, bytes);
	}

	size_t WriteSocket(fd_t socket, void* buf, size_t bytes)
	{
		vnode* node = getvnode(socket);
		if(!node) return -1;

		return ((SocketVFS*) node->info->driver)->Write(node, buf, 0, bytes);
	}

	size_t GetSocketBufferFill(fd_t socket)
	{
		vnode* node = getvnode(socket);
		if(!node) return -1;

		return ((Socket*) node->info->data)->recvbuffer.ByteCount();
	}













	// actual class implementation
	SocketVFS::SocketVFS() : FSDriver(nullptr, FSDriverType::Socket)
	{
		this->_seekable = false;
	}

	SocketVFS::~SocketVFS()
	{
	}

	bool SocketVFS::Create(VFS::vnode*, const char*, uint64_t, uint64_t)
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

			else if(skt->protocol == SocketProtocol::RawIPv4)
				; // nothing to do

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









	// note: implemented here since they do similar things
	static rde::hash_map<rde::string, Socket*>* ipcSocketMap = 0;

	void SocketVFS::Bind(VFS::vnode* node, const char* _path)
	{
		Socket* skt = getsockdata(node);

		// bind/connect is what actually opens the file on the VFS layer, for IPC sockets
		if(!ipcSocketMap) ipcSocketMap = new rde::hash_map<rde::string, Socket*>();
		assert(ipcSocketMap);

		rde::string path(_path);

		if(ipcSocketMap->find(path) != ipcSocketMap->end())
		{
			Log(1, "Socket with path '%s' already exists, aborting...", _path);
			Multitasking::SetThreadErrno(EADDRINUSE);
			return;
		}

		skt->ipcSocketPath = path;
		(*ipcSocketMap)[path] = skt;
	}

	void SocketVFS::Connect(VFS::vnode* node, const char* _path)
	{
		if(!ipcSocketMap) ipcSocketMap = new rde::hash_map<rde::string, Socket*>();
		assert(ipcSocketMap);

		rde::string path(_path);

		// connect to the socket.
		if(ipcSocketMap->find(path) == ipcSocketMap->end())
		{
			Log(1, "No such socket at path '%s'", _path);
			Multitasking::SetThreadErrno(EBADF);
			return;
		}

		Socket* skt = (*ipcSocketMap)[path];
		assert(skt);

		// will this work?
		node->info->data = (void*) skt;
	}















	void SocketVFS::Close(VFS::vnode* node)
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
				assert(skt->ipcSocketPath.length() > 0);
				ipcSocketMap->erase(skt->ipcSocketPath);
				break;

			default:
				HALT("???");
		}

		// this will close the TCP connection
		if(skt->protocol == SocketProtocol::TCP)
			delete skt->tcpconnection;

		delete skt;
		node->info->data = 0;
	}

	size_t SocketVFS::Read(VFS::vnode* node, void* buf, off_t offset, size_t length)
	{
		(void) offset;
		Socket* skt = getsockdata(node);
		if(!skt)
		{
			Log(1, "Invalid vnode");
			Multitasking::SetThreadErrno(EBADF);
			return -1;
		}

		uint64_t ret = __min(skt->recvbuffer.ByteCount(), length);
		skt->recvbuffer.Read((uint8_t*) buf, length);

		return ret;
	}

	size_t SocketVFS::BlockingRead(VFS::vnode* node, void* buf, size_t bytes)
	{
		Socket* skt = getsockdata(node);
		if(!skt)
		{
			Log(1, "Invalid vnode");
			Multitasking::SetThreadErrno(EBADF);
			return -1;
		}

		// only block if we have to
		if(skt->recvbuffer.ByteCount() == 0)
		{
			Log(3, "blocking");
			addProcToBlockingList(skt);
			BLOCK();
		}

		// now we have data.
		return this->Read(node, buf, 0, bytes);
	}

	size_t SocketVFS::Write(VFS::vnode* node, const void* buf, off_t offset, size_t length)
	{
		(void) offset;
		Socket* skt = getsockdata(node);
		if(!skt)
		{
			Log(1, "Invalid vnode");
			Multitasking::SetThreadErrno(EBADF);
			return -1;
		}

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
				Multitasking::SetThreadErrno(EMSGSIZE);
				return -1;
			}

			UDP::SendIPv4Packet(skt->interface, (uint8_t*) buf, (uint16_t) length, skt->ip4dest,
				(uint16_t) skt->clientport, (uint16_t) skt->serverport);
		}
		else if(skt->protocol == SocketProtocol::IPC)
		{
			skt->recvbuffer.Write((uint8_t*) buf, length);

			// TODO: do we want to wake all processes? if there's more than one, there's a fair chance
			// that it may have already read everything in the buffer, leaving the others with nothing
			for(pid_t pid : skt->blockingProcs)
			{
				Log(3, "waking pid %d", pid);
				Multitasking::WakeForMessage(Multitasking::GetProcess(pid));
			}
		}

		return length;
	}

	bool SocketVFS::Delete(VFS::vnode*, const char*)
	{
		return false;
	}

	bool SocketVFS::Traverse(VFS::vnode*, const char*, char**)
	{
		return false;
	}

	void SocketVFS::Flush(VFS::vnode*)
	{

	}

	void SocketVFS::Stat(VFS::vnode*, struct stat*, bool)
	{

	}

	rde::vector<VFS::vnode*> SocketVFS::ReadDir(VFS::vnode*)
	{
		return rde::vector<VFS::vnode*>();
	}
}
}
}
