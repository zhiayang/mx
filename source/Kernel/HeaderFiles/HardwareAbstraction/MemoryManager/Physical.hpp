// Physical.hpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#pragma once
#include <stdint.h>
#include <Multiboot.hpp>
#include "MemoryMap.hpp"

namespace Kernel {
namespace HardwareAbstraction {
namespace MemoryManager
{
	namespace Physical
	{
		extern uint64_t ReservedRegionForVMM;
		extern uint64_t LengthOfReservedRegion;
		extern uint64_t OpsSinceLastCoalesce;

		uint64_t AllocatePage(uint64_t size = 1);
		void FreePage(uint64_t Page, uint64_t size = 1);

		void Bootstrap();
		void Initialise();
		void InitialiseFPLs(MemoryMap::MemoryMap_type* MemoryMap);

		uint64_t AllocateDMA(uint64_t size, bool Below4Gb = true);
		void FreeDMA(uint64_t addr, uint64_t size);

		void CoalesceFPLs();
		uint64_t AllocateFromReserved(uint64_t size = 1);
	}
}
}
}
