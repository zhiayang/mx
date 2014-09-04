// MessagingIPC.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <HardwareAbstraction/Filesystems.hpp>
#include <mqueue.h>

#define BUFSIZE	1024

namespace Kernel {
namespace HardwareAbstraction {
namespace Filesystems
{
	// initialise with null partition, because we don't have one.
	FSDriverIPCMsg::FSDriverIPCMsg() : FSDriver(nullptr, FSDriverType::Virtual)
	{
		// init the message queue.
		this->messagequeue = new rde::hash_map<pathid*, Library::CircularMemoryBuffer*>();
	}

	FSDriverIPCMsg::~FSDriverIPCMsg()
	{
		// same here
	}

	bool FSDriverIPCMsg::Create(VFS::vnode* node, const char* path, uint64_t flags, uint64_t perms)
	{
		assert(flags & O_CREATE);
		(void) perms;
		(void) node;

		// in reality, VFS should have already checked that this->Traverse() returned false.
		// therefore, reduce overhead. assume queue does not exist.


		// make a kernel-level copy
		char* newpth = new char[strlen(path) + 1];
		strcpy(newpth, path);

		pathid* str = new pathid;
		str->path = newpth;

		// id is a simple hash of the path.
		uint8_t* data = (uint8_t*) (newpth);
		uint64_t length = strlen(newpth);

		uint64_t hash = 0;
		for(uint64_t i = 0; i < length; i++)
			hash = (hash << 3) | (hash >> (29)) ^ data[i];

		str->id = hash;
		(*this->messagequeue)[str] = new Library::CircularMemoryBuffer(1024);

		return true;
	}

	bool FSDriverIPCMsg::Delete(VFS::vnode* node, const char* path)
	{
		(void) node;
		(void) path;

		return false;
	}

	bool FSDriverIPCMsg::Traverse(VFS::vnode* node, const char* path, char** symlink)
	{
		(void) node;
		(void) symlink;

		for(auto v : *this->messagequeue)
		{
			if(strcmp(path, v.first->path) == 0)
				return true;
		}
		return false;
	}

	size_t FSDriverIPCMsg::Read(VFS::vnode* node, void* buf, off_t offset, size_t length)
	{
		(void) node;
		(void) buf;
		(void) offset;
		(void) length;

		// line buffer it.
		return 0;
	}

	size_t FSDriverIPCMsg::Write(VFS::vnode* node, const void* buf, off_t offset, size_t length)
	{
		(void) node;
		(void) buf;
		(void) offset;
		(void) length;

		return 0;
	}

	void FSDriverIPCMsg::Stat(VFS::vnode* node, struct stat* st)
	{
		(void) node;
		(void) st;
	}

	rde::vector<VFS::vnode*>* FSDriverIPCMsg::ReadDir(VFS::vnode* node)
	{
		(void) node;
		return nullptr;
	}
}
}
}
