// UserspaceInterface.cpp
// Copyright (c) 2014 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <errno.h>
#include <sys/socket.h>
#include <HardwareAbstraction/Filesystems.hpp>
#include <HardwareAbstraction/Network.hpp>

using namespace Kernel::HardwareAbstraction::Filesystems;

namespace Kernel {
namespace HardwareAbstraction {
namespace SystemCalls
{
	extern "C" fd_t Syscall_OpenAny(const char* path, uint64_t flags)
	{
		// handle only files for now.
		return OpenFile(path, (int) flags);
	}

	extern "C" fd_t Syscall_OpenSocket(uint64_t domain, uint64_t type, uint64_t protocol)
	{
		using namespace Library;

		SocketProtocol sockprot;

		if(protocol != 0)
			Log(1, "Protocol parameter currently not supported for socket(2), ignoring");


		// AF_* and PF_* have the same values
		if(domain == AF_INET && type == SOCK_DGRAM)			sockprot = SocketProtocol::UDP;
		else if(domain == AF_INET && type == SOCK_STREAM)	sockprot = SocketProtocol::TCP;
		else if(domain == AF_INET && type == SOCK_RAW)		sockprot = SocketProtocol::RawIPv4;
		else if(domain == AF_UNIX && type == SOCK_DGRAM)	sockprot = SocketProtocol::IPC;
		else
		{
			Log(1, "Invalid combination of domain(%d) and type(%d)", domain, type);
			Multitasking::SetThreadErrno(EAFNOSUPPORT);
			return -1;
		}

		return Network::OpenSocket(sockprot, 0);
	}

	extern "C" err_t Syscall_BindNetSocket(fd_t fd, uint32_t ipv4, uint16_t port)
	{
		return Network::BindSocket(fd, ipv4, port);
	}

	extern "C" err_t Syscall_ConnectNetSocket(fd_t fd, uint32_t ipv4, uint16_t port)
	{
		return Network::ConnectSocket(fd, ipv4, port);
	}

	extern "C" err_t Syscall_BindIPCSocket(fd_t fd, const char* path)
	{
		return Network::BindSocket(fd, path);
	}

	extern "C" err_t Syscall_ConnectIPCSocket(fd_t fd, const char* path)
	{
		return Network::ConnectSocket(fd, path);
	}







	extern "C" void Syscall_CloseAny(fd_t fd)
	{
		Close(fd);
	}

	extern "C" void Syscall_FlushAny(fd_t fd)
	{
		Flush(fd);
	}

	extern "C" uint64_t Syscall_ReadAny(fd_t fd, const void* dat, uint64_t size)
	{
		return Read(fd, (void*) dat, size);
	}

	extern "C" uint64_t Syscall_WriteAny(fd_t fd, const void* dat, uint64_t size)
	{
		return Write(fd, (void*) dat, size);
	}

	extern "C" int Syscall_SeekAny(fd_t fd, long offset, int whence)
	{
		return Seek(fd, (size_t) offset, whence);
	}

	extern "C" int Syscall_StatAny(fd_t fd, struct stat* st, bool statlink)
	{
		return Stat(fd, st, statlink);
	}

	extern "C" uint64_t Syscall_GetSeekPos(fd_t fd)
	{
		return GetSeekPos(fd);
	}
}
}
}






