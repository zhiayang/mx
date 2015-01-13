// MBR.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <Memory.hpp>
#include <StandardIO.hpp>

using namespace Kernel::HardwareAbstraction::Devices::Storage::ATA;

namespace Kernel {
namespace HardwareAbstraction {
namespace Filesystems {
namespace MBR
{
	void ReadPartitions(Devices::Storage::StorageDevice* atadev)
	{
		using Devices::Storage::Partition;
		// warning: will trash whatever the ATADevice's data buffer contains.

		// read the mbr
		uint64_t b = MemoryManager::Physical::AllocateDMA(1);
		IO::Read(atadev, 0, b, 512);

		// verify the MBR signature: 0x55, 0xAA
		// but since x86 is little-endian, it would really be 0xAA55
		if(((uint16_t*) b)[255] != 0xAA55)
			Library::StandardIO::PrintFormatted("\t\t\t- Invalid MBR signature! Expected [%x], got [%x] instead -- Check your disk.\n", 0xAA55, ((uint16_t*) b)[255]);


		// read the partition table at the 446'th byte
		uint8_t* mbr = (uint8_t*) b;


		// check for a GPT partition
		for(uint16_t o = 0x1BE; o < 0x1BE + 0x40; o += 0x10)
		{
			if(*((uint32_t*)(mbr + o + 12)) > 0)
			{
				// loop through each partition, looking for one with type = 0xEE. If this is so, we pass the
				// device to GPT::ReadPartitions instead (to intialise the list there) and return immediately.

				if(*(mbr + o + 4) == 0xEE)
				{
					GPT::ReadPartitions(atadev);
					return;
				}
			}
		}

		Log("MBR-partitioned drive detected, parsing...");

		// read the partition table
		for(uint16_t o = 0x1BE; o < 0x1BE + 0x40; o += 0x10)
		{
			if(*((uint32_t*)(mbr + o + 12)) > 0)
			{
				FSTypes fstype = FSTypes::fat32;
				switch(*(mbr + o + 4))
				{
					case 0xB:
					case 0xC:
						fstype = FSTypes::fat32;
						break;

					case 0xAF:
						fstype = FSTypes::hfsplus;
						break;
				}

				atadev->Partitions.push_back(new Partition(atadev, (uint8_t) (o - 0x1BE) / 0x10, *((uint32_t*) (mbr + o + 8)), *((uint32_t*) (mbr + o + 12)), fstype, 0, 0, 0, 0, (char*) "", *(mbr + o) & 0x80));
			}
		}

		Devices::Storage::AddDevice(atadev);
		Log("/dev/disk%d has %d partition%s", atadev->diskid, atadev->Partitions.size(), atadev->Partitions.size() != 1 ? "s" : "");
		MemoryManager::Physical::FreeDMA(b, 1);
	}
}
}
}
}
