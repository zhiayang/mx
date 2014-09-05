// Virtual.hpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#pragma once
#include <stdint.h>
#include <rdestl/vector.h>

namespace Kernel {
namespace HardwareAbstraction {
namespace MemoryManager {
namespace Virtual
{
	struct PageMapStructure
	{
		uint64_t Entry[512];
	};

	struct AddressLengthPair
	{
		AddressLengthPair(uint64_t s, uint64_t l) : start(s), length(l) { }
		uint64_t start;
		uint64_t length;
	};

	struct VirtualAddressSpace
	{
		VirtualAddressSpace(PageMapStructure* pml4)
		{
			this->PML4 = pml4;
			this->pairs = new rde::vector<AddressLengthPair*>();
		}

		// book-keeping for allocations.
		rde::vector<AddressLengthPair*>* pairs;

		// store the actual address of the pml4.
		PageMapStructure* PML4;
	};


	#define I_Present		0x01
	#define I_ReadWrite		0x02
	#define I_UserAccess		0x04
	#define I_AlignMask		0xFFFFFFFFFFFFF000
	#define I_NoExecute		0



	void Initialise();
	void SwitchPML4T(PageMapStructure* PML4T);
	PageMapStructure* GetCurrentPML4T();

	VirtualAddressSpace* SetupVAS(VirtualAddressSpace* vas);
	uint64_t AllocateVirtual(uint64_t size = 1, uint64_t addr = 0, VirtualAddressSpace* vas = 0);
	void FreeVirtual(uint64_t addr, uint64_t size = 1, VirtualAddressSpace* vas = 0);

	uint64_t AllocatePage(uint64_t size = 1, uint64_t addr = 0, uint64_t flags = 0x3);
	void FreePage(uint64_t addr, uint64_t size = 1);


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

	void SetupTempMappings(PageMapStructure* PML4);
	uint64_t CreateVAS();
	void MapToAllProcesses(uint64_t v, uint64_t p, uint64_t f);

	uint64_t GetMapping(uint64_t VirtAddr, PageMapStructure* VAS);

	bool GetPagingFlag();
}
}
}
}
