// FileIO.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <List.hpp>

namespace Kernel {
namespace HardwareAbstraction {
namespace SystemCalls
{
	// extern "C" uint64_t Syscall_OpenFile(const char* path, uint8_t mode)
	// {
	// 	// return Filesystems::VFS::OpenFile(path, mode);
	// }

	// extern "C" void Syscall_CloseFile(uint64_t fd)
	// {
	// 	// Filesystems::VFS::CloseFile(fd);
	// }

	// // deprecated (prefer ReadAny)
	// extern "C" uint8_t* Syscall_ReadFile(uint64_t fd, uint8_t* buffer, uint64_t length)
	// {
	// 	// Filesystems::VFS::ReadFile(fd, buffer, length);
	// 	return buffer;
	// }

	// // deprecated (prefer WriteAny)
	// extern "C" uint8_t* Syscall_WriteFile(uint64_t fd, uint8_t* buffer, uint64_t length)
	// {
	// 	// Filesystems::VFS::WriteFile(fd, buffer, length);
	// 	return buffer;
	// }

	// extern "C" uint64_t Syscall_OpenFolder(const char* path)
	// {
	// 	// return Filesystems::VFS::OpenFolder(path);
	// }

	// extern "C" void Syscall_CloseFolder(uint64_t fd)
	// {
	// 	// Filesystems::VFS::CloseFolder(fd);
	// }

	// extern "C" Library::LinkedList<char>* Syscall_ListObjectsInFolder(uint64_t fd, Library::LinkedList<char>* output, uint64_t* items)
	// {
	// 	// return Filesystems::VFS::ListObjects(fd, output, items);
	// }

	// extern "C" uint64_t Syscall_GetFileSize(uint64_t fd)
	// {
	// 	// // this is contrary to our usual model for system calls -- this one actually does some work.
	// 	// using namespace Kernel::HardwareAbstraction::Filesystems::VFS;
	// 	// if(fd > MaxDescriptors || fd < 4)
	// 	// 	return 0;

	// 	// FSObject* fso = Multitasking::GetCurrentProcess()->FileDescriptors[fd].Pointer;
	// 	// if(fso->Type != FSObjectTypes::File)
	// 	// {
	// 	// 	Log(1, "Error: Descriptor (%d) is not a file, cannot get size.", fd);
	// 	// 	return 0;
	// 	// }

	// 	// File* f = (File*) fso;
	// 	// uint64_t ret = f->FileSize();
	// 	// return ret;
	// }

	// extern "C" uint64_t Syscall_CheckFileExistence(const char* path)
	// {
	// 	// using namespace Kernel::HardwareAbstraction::Filesystems::VFS;
	// 	// File* f = new File(path);
	// 	// bool ret = f->Exists();
	// 	// delete f;

	// 	// return ret;
	// }

	// extern "C" uint64_t Syscall_CheckFolderExistence(const char* path)
	// {
	// 	// using namespace Kernel::HardwareAbstraction::Filesystems::VFS;
	// 	// Folder* f = new Folder(path);
	// 	// bool ret = f->Exists();
	// 	// delete f;

	// 	// return ret;
	// }

	// extern "C" uint64_t Syscall_MMapFile(uint64_t fd, uint64_t length, uint64_t offset, uint64_t prot, uint64_t flags)
	// {
	// 	UNUSED(fd);
	// 	UNUSED(length);
	// 	UNUSED(offset);
	// 	UNUSED(prot);
	// 	UNUSED(flags);

	// 	HALT("ENOTSUP");
	// 	return 0;
	// }
}
}
}


























