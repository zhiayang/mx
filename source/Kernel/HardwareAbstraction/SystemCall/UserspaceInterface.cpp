// UserspaceInterface.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <IPC.hpp>
#include <HardwareAbstraction/Filesystems.hpp>
#include <HardwareAbstraction/Network.hpp>

using namespace Kernel::HardwareAbstraction::Filesystems::VFS;

namespace Kernel {
namespace HardwareAbstraction {
namespace SystemCalls
{
	// FSObject* VerifyFSO(Multitasking::Process* proc, uint64_t fd)
	// {
	// 	if(fd >= MaxDescriptors ||  proc->FileDescriptors[fd].Pointer == 0)
	// 	{
	// 		Log(1, "Error: Descriptor %d does not exist, or has not been opened.", fd);
	// 		return 0;
	// 	}

	// 	if(proc->FileDescriptors[fd].Pointer->Type == FSObjectTypes::Invalid)
	// 	{
	// 		Log(1, "Error: Descriptor %d contains a handle of invalid type.", fd);
	// 		return 0;
	// 	}

	// 	return proc->FileDescriptors[fd].Pointer;
	// }

	// extern "C" uint64_t Syscall_OpenAny(const char* path, uint64_t flags)
	// {
	// 	// handle only files for now.
	// 	// (void) flags;

	// 	// return OpenFile(path, 0);
	// }

	// extern "C" void Syscall_CloseAny(uint64_t fd)
	// {
	// 	// // check.
	// 	// auto fso = VerifyFSO(Multitasking::GetCurrentProcess(), fd);
	// 	// if(!fso || fso->Type == FSObjectTypes::Invalid)
	// 	// {
	// 	// 	Log(1, "Ignoring attempt to close invalid descriptor %d", fd);
	// 	// 	return;
	// 	// }

	// 	// switch(fso->Type)
	// 	// {
	// 	// 	case FSObjectTypes::File:
	// 	// 		CloseFile(fd);
	// 	// 		break;

	// 	// 	case FSObjectTypes::Socket:
	// 	// 		Network::Socket::CloseSocket(fd);
	// 	// 		break;

	// 	// 	case FSObjectTypes::IPCSocket:
	// 	// 		IPC::CloseIPCSocket(fd);
	// 	// 		break;


	// 	// 	case FSObjectTypes::Folder:
	// 	// 		CloseFolder(fd);
	// 	// 		break;

	// 	// 	case FSObjectTypes::Filesystem:
	// 	// 	case FSObjectTypes::Invalid:
	// 	// 		Log(1, "Error: cannot close descriptor %d when it is a filesystem.", fd);
	// 	// 		break;
	// 	// }
	// }

	// extern "C" uint64_t Syscall_ReadAny(uint64_t fd, const void* dat, uint64_t size)
	// {
	// 	// // check.
	// 	// auto fso = VerifyFSO(Multitasking::GetCurrentProcess(), fd);
	// 	// if(!fso || fso->Type == FSObjectTypes::Invalid)
	// 	// 	return 0;

	// 	// switch(fso->Type)
	// 	// {
	// 	// 	case FSObjectTypes::File:
	// 	// 		return ReadFile(fd, (uint8_t*) dat, size);

	// 	// 	case FSObjectTypes::Socket:
	// 	// 		return Network::Socket::ReadFromSocket(fd, (uint8_t*) dat, size);

	// 	// 	case FSObjectTypes::IPCSocket:
	// 	// 		return IPC::ReadFromIPCSocket(fd, (uint8_t*) dat, size);


	// 	// 	case FSObjectTypes::Folder:
	// 	// 	case FSObjectTypes::Filesystem:
	// 	// 	case FSObjectTypes::Invalid:
	// 	// 		Log(1, "Error: cannot read from descriptor %d when it is a folder or filesystem.", fd);
	// 	// 		return 0;
	// 	// }
	// }

	// extern "C" uint64_t Syscall_WriteAny(uint64_t fd, const void* dat, uint64_t size)
	// {
	// 	// // check.
	// 	// auto fso = VerifyFSO(Multitasking::GetCurrentProcess(), fd);
	// 	// if(!fso || fso->Type == FSObjectTypes::Invalid)
	// 	// 	return 0;

	// 	// switch(fso->Type)
	// 	// {
	// 	// 	case FSObjectTypes::File:
	// 	// 		return WriteFile(fd, (uint8_t*) dat, size);

	// 	// 	case FSObjectTypes::Socket:
	// 	// 		return Network::Socket::WriteToSocket(fd, (uint8_t*) dat, size);

	// 	// 	case FSObjectTypes::IPCSocket:
	// 	// 		return IPC::WriteToIPCSocket(fd, (uint8_t*) dat, size);


	// 	// 	case FSObjectTypes::Folder:
	// 	// 	case FSObjectTypes::Filesystem:
	// 	// 	case FSObjectTypes::Invalid:
	// 	// 		Log(1, "Error: cannot write to descriptor %d when it is a folder or filesystem.", fd);
	// 	// 		return 0;
	// 	// }
	// }
}
}
}
