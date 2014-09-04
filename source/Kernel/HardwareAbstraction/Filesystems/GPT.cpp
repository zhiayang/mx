// GPT.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <StandardIO.hpp>
#include <Memory.hpp>
#include <HardwareAbstraction/Devices/StorageDevice.hpp>

using namespace Kernel::HardwareAbstraction::Devices::Storage::ATA;
using namespace Kernel::HardwareAbstraction::Devices::Storage;
using namespace Library::StandardIO;

namespace Kernel {
namespace HardwareAbstraction {
namespace Filesystems {
namespace GPT
{
	void ReadPartitions(StorageDevice* atadev)
	{
		// normally MBR::ReadPartitions will call this... so we can assume the list is uninitialised.
		atadev->Partitions = new rde::list<Partition*>();
		// warning: will trash whatever the ATADevice's data buffer contains.

		// read the gpt
		uint64_t b = MemoryManager::Physical::AllocateDMA(1);
		IO::Read(atadev, 1, b, 512);
		// atadev->Read(1, b, 512);


		// uint8_t* gpt = (uint8_t*) atadev->Data;
		uint8_t* gpt = (uint8_t*) b;

		// 0x5452415020494645
		if(*((uint64_t*) gpt) != 0x5452415020494645)
			PrintFormatted("Invalid GPT disk signature: expected [%x], got [%x] instead -- Check your disk.\n", 0x5452415020494645, *((uint64_t*) gpt));


		uint32_t number = 4;
		// atadev->SetIsGPT(true);
		atadev->PartitionTable = PartitionTableType::GuidPartitionTable;


		// if the header is fine, read LBA 2 to parse the partition table.
		// PIO::ReadSector(atadev, 2);
		// atadev->Read(2, b + 512, 512);
		IO::Read(atadev, 2, b + 512, 512);

		uint8_t* table = (uint8_t*) (b + 512);

		for(uint64_t p = 0; p < number; p++)
		{
			if(*((uint64_t*)(table + (p * 128) + 8)) != 0 && *((uint64_t*)(table + (p * 128) + 0)) != 0)
			{
				atadev->Partitions->push_back(new Partition(atadev, (uint8_t) p, *((uint64_t*)(table + (p * 128) + 32)), *((uint64_t*)(table + (p * 128) + 40)) - *((uint64_t*)(table + (p * 128) + 32)) + 1, FSTypes::hfsplus, *((uint64_t*)(table + (p * 128) + 24)), *((uint64_t*)(table + (p * 128) + 16)), *((uint64_t*)(table + (p * 128) + 0)), *((uint64_t*)(table + (p * 128) + 8)), (char*) "", false));
			}
		}

		MemoryManager::Physical::FreeDMA(b, 1);
	}
}
}
}
}








