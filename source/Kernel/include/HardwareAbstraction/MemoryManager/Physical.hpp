// Physical.hpp
// Copyright (c) 2013 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#pragma once
#include <stdint.h>
#include <Multiboot.hpp>
#include "MemoryMap.hpp"

namespace Kernel
{
	struct DMAAddr;
	namespace HardwareAbstraction {
	namespace MemoryManager
	{
		namespace Physical
		{
			extern uint64_t ReservedRegionForVMM;
			extern uint64_t LengthOfReservedRegion;
			extern uint64_t OpsSinceLastCoalesce;

			uint64_t AllocatePage(uint64_t size = 1, bool Below4Gb = false);
			void FreePage(uint64_t Page, uint64_t size = 1);

			void Bootstrap();
			void Initialise();
			void InitialiseFPLs(MemoryMap::MemoryMap_type* MemoryMap);

			DMAAddr AllocateDMA(uint64_t size, bool Below4Gb = true);
			void FreeDMA(DMAAddr addr, uint64_t size);

			void CoalesceFPLs();
			uint64_t AllocateFromReserved(uint64_t size = 1);
		}
	}
}
}
