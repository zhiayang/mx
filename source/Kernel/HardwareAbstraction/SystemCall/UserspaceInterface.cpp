// UserspaceInterface.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <IPC.hpp>
#include <HardwareAbstraction/Filesystems.hpp>
#include <HardwareAbstraction/Network.hpp>

using namespace Kernel::HardwareAbstraction::Filesystems;

namespace Kernel {
namespace HardwareAbstraction {
namespace SystemCalls
{
	extern "C" uint64_t Syscall_OpenAny(const char* path, uint64_t flags)
	{
		// handle only files for now.
		return OpenFile(path, (int) flags);
	}

	extern "C" uint64_t Syscall_OpenSocket(uint64_t domain, uint64_t type, uint64_t protocol)
	{
		return 0;
	}

	extern "C" void Syscall_CloseAny(uint64_t fd)
	{
		(void) fd;
	}

	extern "C" void Syscall_FlushAny(uint64_t fd)
	{
		Flush(fd);
	}

	extern "C" uint64_t Syscall_ReadAny(uint64_t fd, const void* dat, uint64_t size)
	{
		return Read(fd, (void*) dat, size);
	}

	extern "C" uint64_t Syscall_WriteAny(uint64_t fd, const void* dat, uint64_t size)
	{
		return Write(fd, (void*) dat, size);
	}

	extern "C" int Syscall_SeekAny(uint64_t fd, long offset, int whence)
	{
		return Seek(fd, offset, whence);
	}

	extern "C" int Syscall_StatAny(uint64_t fd, struct stat* st, bool statlink)
	{
		return Stat(fd, st, statlink);
	}

	extern "C" uint64_t Syscall_GetSeekPos(uint64_t fd)
	{
		return GetSeekPos(fd);
	}
}
}
}






