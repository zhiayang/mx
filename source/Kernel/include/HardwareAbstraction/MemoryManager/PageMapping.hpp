// PageMapping.hpp
// Copyright (c) 2014 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

// Virtual.hpp
// Copyright (c) 2013 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#pragma once
#include <stdint.h>
#include <Synchro.hpp>
#include <rdestl/rdestl.h>

#define I_Present		0x01
#define I_ReadWrite		0x02
#define I_UserAccess	0x04
#define I_AlignMask		0xFFFFFFFFFFFFF000
#define I_NoExecute		0
#define I_CopyOnWrite	0x800	// bit 11
#define I_SwappedPage	0x400	// bit 10


#define I_RECURSIVE_SLOT	500

// Convert an address into array index of a structure
// E.G. int index = I_PML4_INDEX(0xFFFFFFFFFFFFFFFF); // index = 511
#define I_PML4_INDEX(addr)		((((uintptr_t)(addr)) >> 39) & 0x1FF)
#define I_PDPT_INDEX(addr)		((((uintptr_t)(addr)) >> 30) & 0x1FF)
#define I_PD_INDEX(addr)		((((uintptr_t)(addr)) >> 21) & 0x1FF)
#define I_PT_INDEX(addr)		((((uintptr_t)(addr)) >> 12) & 0x1FF)


namespace Kernel {
namespace HardwareAbstraction {
namespace MemoryManager {
namespace Virtual
{
	struct PageMapStructure
	{
		uint64_t Entry[512];
	};

	struct VirtualAddressSpace;



	void Initialise();
	uint64_t GetRawCR3();
	void ChangeRawCR3(uint64_t newval);
	void invlpg(PageMapStructure* p);
	void SwitchPML4T(PageMapStructure* PML4T);
	PageMapStructure* GetCurrentPML4T();

	void MapAddress(uint64_t VirtAddr, uint64_t PhysAddr, uint64_t Flags, PageMapStructure* PML4, bool DoNotUnmap);
	void UnmapAddress(uint64_t VirtAddr, PageMapStructure* PML4, bool DoNotUnmap);

	void MapAddress(uint64_t VirtAddr, uint64_t PhysAddr, uint64_t Flags);
	void MapAddress(uint64_t VirtAddr, uint64_t PhysAddr, uint64_t Flags, PageMapStructure* PML4);
	void MapAddress(uint64_t VirtAddr, uint64_t PhysAddr, uint64_t Flags, bool DoNotUnmap);
	void UnmapAddress(uint64_t VirtAddr);
	void UnmapAddress(uint64_t VirtAddr, PageMapStructure* PML4);
	void UnmapAddress(uint64_t VirtAddr, bool DoNotUnmap);

	void MapRegion(uint64_t VirtAddr, uint64_t PhysAddr, uint64_t LengthInPages, uint64_t Flags, PageMapStructure* PML4 = 0);
	void UnmapRegion(uint64_t VirtAddr, uint64_t LengthInPages, PageMapStructure* PML4 = 0);

	void MarkCOW(uint64_t VirtAddr, VirtualAddressSpace* vas = 0);
	void UnmarkCOW(uint64_t VirtAddr, VirtualAddressSpace* vas = 0);

	void MarkCOW(uint64_t VirtAddr, PageMapStructure* pml);
	void UnmarkCOW(uint64_t VirtAddr, PageMapStructure* pml);

	uint64_t GetMapping(uint64_t VirtAddr, PageMapStructure* VAS);
	uint64_t* GetPageTableEntry(uint64_t va, PageMapStructure* VAS, PageMapStructure** pdpt, PageMapStructure** pd, PageMapStructure** pt);

}
}
}
}
