// FSDriver.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <HardwareAbstraction/Devices/StorageDevice.hpp>

using namespace Library::StandardIO;
using namespace Kernel::HardwareAbstraction::Devices::Storage;

namespace Kernel {
namespace HardwareAbstraction {
namespace Filesystems
{
	// Internal FS types.
	/*
		// FAT family
		0xF1EF: exFAT
		0xF132: FAT32
		0xF116: FAT16
		0xF112: FAT12

		// Apple
		0x11F5: HFS+
		0x11F0: HFS

		// Linux
		0xEB12: ext2
		0xEB13: ext3
		0xEB14: ext4
	*/

	FSDriver::FSDriver(FSTypes type)
	{
		this->Type = type;
	}

	FSDriver::~FSDriver()
	{
	}

	void FSDriver::PrintInfo()
	{
	}

	Partition* FSDriver::GetPartition()
	{
		return this->ParentPartition;
	}

	VFS::Filesystem* FSDriver::RootFS()
	{
		return this->rootfs;
	}
}
}
}
