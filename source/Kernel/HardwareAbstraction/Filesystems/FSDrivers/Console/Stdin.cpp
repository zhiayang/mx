// Stdin.cpp
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
	FSDriverStdin::FSDriverStdin() : FSDriver(nullptr, FSDriverType::Virtual)
	{
		// nothing to do here.
		this->_seekable = false;
	}

	FSDriverStdin::~FSDriverStdin()
	{
		// same here
	}

	bool FSDriverStdin::Create(VFS::vnode*, const char*, uint64_t, uint64_t)
	{
		return false;
	}
	bool FSDriverStdin::Delete(VFS::vnode*, const char*)
	{
		return false;
	}
	bool FSDriverStdin::Traverse(VFS::vnode*, const char*, char**)
	{
		return true;
	}

	size_t FSDriverStdin::Read(VFS::vnode*, void* buf, off_t, size_t length)
	{
		// line buffer it.
		return TTY::ReadTTY(0, (uint8_t*) buf, length);
	}

	size_t FSDriverStdin::Write(VFS::vnode*, const void*, off_t, size_t)
	{
		return 0;
	}

	void FSDriverStdin::Flush(VFS::vnode* node)
	{
		(void) node;
		TTY::FlushTTY(0);
	}

	void FSDriverStdin::Close(VFS::vnode*)
	{
	}

	void FSDriverStdin::Stat(VFS::vnode*, struct stat*, bool)
	{
	}

	rde::vector<VFS::vnode*> FSDriverStdin::ReadDir(VFS::vnode*)
	{
		return rde::vector<VFS::vnode*>();
	}
}
}
}
