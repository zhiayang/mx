// Stdlog.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#include <Kernel.hpp>
#include <Console.hpp>
#include <HardwareAbstraction/Filesystems.hpp>

namespace Kernel {
namespace HardwareAbstraction {
namespace Filesystems
{
	// initialise with null partition, because we don't have one.
	FSDriverStdlog::FSDriverStdlog() : FSDriver(nullptr, FSDriverType::Virtual)
	{
		// nothing to do here.
		this->_seekable = false;
	}

	FSDriverStdlog::~FSDriverStdlog()
	{
		// same here
	}

	bool FSDriverStdlog::Create(VFS::vnode*, const char*, uint64_t, uint64_t)
	{
		return false;
	}
	bool FSDriverStdlog::Delete(VFS::vnode*, const char*)
	{
		return false;
	}

	bool FSDriverStdlog::Traverse(VFS::vnode*, const char*, char**)
	{
		return true;
	}

	size_t FSDriverStdlog::Read(VFS::vnode*, void*, off_t, size_t)
	{
		return 0;
	}

	size_t FSDriverStdlog::Write(VFS::vnode*, const void* buf, off_t, size_t length)
	{
		assert(buf);
		// offset is ignored.
		if(length == 0)
			return 0;

		Log(0, (uint8_t*) buf);
		return length;
	}

	void FSDriverStdlog::Stat(VFS::vnode*, struct stat*, bool)
	{
	}

	void FSDriverStdlog::Flush(VFS::vnode*)
	{
		TTY::FlushTTY(1);
	}

	rde::vector<VFS::vnode*> FSDriverStdlog::ReadDir(VFS::vnode*)
	{
		return rde::vector<VFS::vnode*>();
	}
}
}
}
