// Virtual.hpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#pragma once
#include <stdint.h>
#include <Synchro.hpp>
#include <rdestl/rdestl.h>
#include <HardwareAbstraction/MemoryManager/PageMapping.hpp>

#include <Vector.hpp>

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

class Mutex;

namespace HardwareAbstraction {
namespace MemoryManager {
namespace Virtual
{
	struct MemRegion
	{
		uint64_t start;
		uint64_t length : 63;
		uint64_t used : 1;
		uint64_t phys;

		bool operator==(MemRegion& other)
		{
			return other.start == this->start && other.length == this->length && other.phys == this->phys;
		}
	};







	struct VirtualAddressSpace
	{
		VirtualAddressSpace(PageMapStructure* pml4)
		{
			this->PML4 = pml4;
			this->mtx = new Mutex();
		}

		// store the actual address of the pml4.
		rde::vector<MemRegion*>* regions;

		PageMapStructure* PML4;
		Mutex* mtx;
	};

	uint64_t AllocatePage(uint64_t size = 1, uint64_t addr = 0, uint64_t flags = 0x7);
	uint64_t AllocateVirtual(uint64_t size = 1, uint64_t addr = 0, VirtualAddressSpace* vas = 0, uint64_t phys = 0);

	void FreePage(uint64_t addr, uint64_t size);
	void FreeVirtual(uint64_t addr, uint64_t size, VirtualAddressSpace* vas = 0);


	VirtualAddressSpace* SetupVAS(VirtualAddressSpace* vas);
	void DestroyVAS(VirtualAddressSpace* vas);

	uint64_t GetVirtualPhysical(uint64_t virt, VirtualAddressSpace* vas = 0);
	void ForceInsertALPTuple(uint64_t addr, size_t sizeInPages, uint64_t phys, VirtualAddressSpace* vas = 0);
	VirtualAddressSpace* CopyVAS(VirtualAddressSpace* src, VirtualAddressSpace* dest);
	bool HandlePageFault(uint64_t cr2, uint64_t cr3, uint64_t errorcode);

	uint64_t CreateVAS();



	void CopyBetweenAddressSpaces(uint64_t fromAddr, uint64_t toAddr, size_t bytes, VirtualAddressSpace* from, VirtualAddressSpace* to);
	void CopyFromKernel(uint64_t fromAddr, uint64_t toAddr, size_t bytes, VirtualAddressSpace* to);
	void CopyToKernel(uint64_t fromAddr, uint64_t toAddr, size_t bytes, VirtualAddressSpace* from);

}
}
}
}
