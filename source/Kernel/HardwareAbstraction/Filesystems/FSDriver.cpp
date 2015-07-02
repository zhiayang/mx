// FSDriver.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <HardwareAbstraction/Devices/StorageDevice.hpp>

using namespace Kernel::HardwareAbstraction::Devices::Storage;

namespace Kernel {
namespace HardwareAbstraction {
namespace Filesystems
{
	FSDriver::~FSDriver()
	{
	}

	bool FSDriver::Create(VFS::vnode*, const char*, uint64_t, uint64_t)
	{
		return 0;
	}

	bool FSDriver::Delete(VFS::vnode*, const char*)
	{
		return 0;
	}

	bool FSDriver::Traverse(VFS::vnode*, const char*, char**)
	{
		return 0;
	}

	size_t FSDriver::Read(VFS::vnode*, void*, off_t, size_t)
	{
		return 0;
	}

	size_t FSDriver::Write(VFS::vnode*, const void*, off_t, size_t)
	{
		return 0;
	}

	void FSDriver::Flush(VFS::vnode*)
	{
	}

	void FSDriver::Stat(VFS::vnode*, struct stat*, bool)
	{
	}

	void FSDriver::Close(VFS::vnode*)
	{
	}

	rde::vector<VFS::vnode*> FSDriver::ReadDir(VFS::vnode*)
	{
		return rde::vector<VFS::vnode*>();
	}
}
}
}
