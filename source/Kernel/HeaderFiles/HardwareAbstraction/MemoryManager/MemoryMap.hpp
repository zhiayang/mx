// MemoryMap.hpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#pragma once
#include <stdint.h>
#include <Multiboot.hpp>

namespace Kernel {
namespace HardwareAbstraction {
namespace MemoryManager
{
	namespace MemoryMap
	{
		#define G_MemoryTypeAvailable		1
		#define G_MemoryTypeReserved		2
		#define G_MemoryACPI				3
		#define G_MemoryNVS					4
		#define G_MemoryBadRam				5

		struct MemoryMapEntry_type
		{
			uint64_t BaseAddress;
			uint64_t Length;
			uint8_t MemoryType;
		};

		struct MemoryMap_type
		{
			uint16_t NumberOfEntries;
			uint32_t SizeOfThisStructure;

			MemoryMapEntry_type Entries[512];
		};

		bool IsMemoryValid(uint64_t Address);
		void GetGlobalMemoryMap(MemoryMap_type* MemoryMap);
		uint64_t GetTotalSystemMemory();
		uint64_t CheckForMoreAvailableMemory(uint64_t CurrentAddr);
		void PrintKernelMemoryMap();
		void Initialise(Multiboot::Info_type* MBTStruct);
	}
}
}
}
