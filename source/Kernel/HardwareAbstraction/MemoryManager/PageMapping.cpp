// PageMapping.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <HardwareAbstraction/MemoryManager/Virtual.hpp>


namespace Kernel {
namespace HardwareAbstraction {
namespace MemoryManager {
namespace Virtual
{
	static PageMapStructure* CurrentPML4T;

	// this stuff deals with the non-allocator part of the vmm
	// simply the page mapping bits.


	void Initialise()
	{
		PageMapStructure* OriginalPml4 = (PageMapStructure*) GetKernelCR3();

		OriginalPml4->Entry[I_RECURSIVE_SLOT] = (uint64_t) OriginalPml4 | I_Present | I_ReadWrite;
		CurrentPML4T = OriginalPml4;
	}

	void ChangeRawCR3(uint64_t newval)
	{
		asm volatile("mov %[nv], %%rax; mov %%rax, %%cr3" :: [nv]"g"(newval) : "memory", "rax");
	}

	uint64_t GetRawCR3()
	{
		uint64_t ret = 0;
		asm volatile("mov %%cr3, %%rax; mov %%rax, %[r]" : [r]"=g"(ret) :: "memory", "rax");

		return ret;
	}

	void SwitchPML4T(PageMapStructure* PML4T)
	{
		CurrentPML4T = PML4T;
	}

	PageMapStructure* GetCurrentPML4T()
	{
		return CurrentPML4T;
	}

	void invlpg(PageMapStructure* p)
	{
		asm volatile("invlpg (%0)" : : "a" ((uint64_t) p));
	}




	void MapAddress(uint64_t VirtAddr, uint64_t PhysAddr, uint64_t Flags)
	{
		MapAddress(VirtAddr, PhysAddr, Flags, GetCurrentPML4T(), false);
	}

	void MapAddress(uint64_t VirtAddr, uint64_t PhysAddr, uint64_t Flags, PageMapStructure* PML4)
	{
		MapAddress(VirtAddr, PhysAddr, Flags, PML4, false);
	}

	void MapAddress(uint64_t VirtAddr, uint64_t PhysAddr, uint64_t Flags, bool DoNotUnmap)
	{
		MapAddress(VirtAddr, PhysAddr, Flags, GetCurrentPML4T(), DoNotUnmap);
	}



	void UnmapAddress(uint64_t VirtAddr)
	{
		UnmapAddress(VirtAddr, GetCurrentPML4T(), false);
	}

	void UnmapAddress(uint64_t VirtAddr, PageMapStructure* PML4)
	{
		UnmapAddress(VirtAddr, PML4, false);
	}

	void UnmapAddress(uint64_t VirtAddr, bool DoNotUnmap)
	{
		UnmapAddress(VirtAddr, GetCurrentPML4T(), DoNotUnmap);
	}









	void MapAddress(uint64_t VirtAddr, uint64_t PhysAddr, uint64_t Flags, PageMapStructure* PML4, bool DoNotUnmap)
	{
		uint64_t PageTableIndex					= I_PT_INDEX(VirtAddr);
		uint64_t PageDirectoryIndex				= I_PD_INDEX(VirtAddr);
		uint64_t PageDirectoryPointerTableIndex	= I_PDPT_INDEX(VirtAddr);
		uint64_t PML4TIndex						= I_PML4_INDEX(VirtAddr);

		assert(PageTableIndex < 512);
		assert(PageDirectoryIndex < 512);
		assert(PageDirectoryPointerTableIndex < 512);
		assert(PML4TIndex < 512);

		if(PML4TIndex == I_RECURSIVE_SLOT)
		{
			// We can't map that, we need that for our recursive mapping.
			HALT("Tried to map to PML4 (RESTRICTED, KERNEL USE)");
		}

		(void) DoNotUnmap;


		PageMapStructure* PML = (PML4 == 0 ? GetCurrentPML4T() : PML4);
		bool other = (PML != GetCurrentPML4T());

		assert(PML);

		if(other)
			Virtual::MapAddress((uint64_t) PML, (uint64_t) PML, 0x7);



		if(!(PML->Entry[PML4TIndex] & I_Present))
		{
			PML->Entry[PML4TIndex] = Physical::AllocateFromReserved() | (Flags | 0x1);
			invlpg(PML);
		}
		PageMapStructure* PDPT = (PageMapStructure*) (PML->Entry[PML4TIndex] & I_AlignMask);
		if(other)
			Virtual::MapAddress((uint64_t) PDPT, (uint64_t) PDPT, 0x7);



		if(!(PDPT->Entry[PageDirectoryPointerTableIndex] & I_Present))
		{
			PDPT->Entry[PageDirectoryPointerTableIndex] = Physical::AllocateFromReserved() | (Flags | 0x1);
			invlpg(PML);
			invlpg(PDPT);
		}
		PageMapStructure* PageDirectory = (PageMapStructure*) (PDPT->Entry[PageDirectoryPointerTableIndex] & I_AlignMask);
		if(other)
			Virtual::MapAddress((uint64_t) PageDirectory, (uint64_t) PageDirectory, 0x7);


		if(!(PageDirectory->Entry[PageDirectoryIndex] & I_Present))
		{
			PageDirectory->Entry[PageDirectoryIndex] = Physical::AllocateFromReserved() | (Flags | 0x1);
			invlpg(PML);
			invlpg(PDPT);
			invlpg(PageDirectory);
		}

		PageMapStructure* PageTable = (PageMapStructure*) (PageDirectory->Entry[PageDirectoryIndex] & I_AlignMask);
		if(other)
			Virtual::MapAddress((uint64_t) PageTable, (uint64_t) PageTable, 0x7);


		PageTable->Entry[PageTableIndex] = PhysAddr | Flags;

		if(other)
		{
			Virtual::UnmapAddress((uint64_t) PageTable);
			Virtual::UnmapAddress((uint64_t) PageDirectory);
			Virtual::UnmapAddress((uint64_t) PDPT);
			Virtual::UnmapAddress((uint64_t) PML);
		}
	}





	void UnmapAddress(uint64_t VirtAddr, PageMapStructure* PML4, bool DoNotUnmap)
	{
		bool DidMapPML4 = false;

		if(PML4 == 0)
			PML4 = GetCurrentPML4T();

		else if(PML4 != GetCurrentPML4T())
		{
			DidMapPML4 = true;
			MapAddress((uint64_t) PML4, (uint64_t) PML4, 0x03);
		}

		uint64_t PageTableIndex					= I_PT_INDEX(VirtAddr);
		uint64_t PageDirectoryIndex				= I_PD_INDEX(VirtAddr);
		uint64_t PageDirectoryPointerTableIndex	= I_PDPT_INDEX(VirtAddr);
		uint64_t PML4TIndex						= I_PML4_INDEX(VirtAddr);


		PageMapStructure* PML = (PageMapStructure*)(((PageMapStructure*) PML4)->Entry[I_RECURSIVE_SLOT] & I_AlignMask);

		if(PML)
		{
			PageMapStructure* PDPT = (PageMapStructure*)(PML->Entry[PML4TIndex] & I_AlignMask);

			if(PDPT)
			{
				PageMapStructure* PageDirectory = (PageMapStructure*)(PDPT->Entry[PageDirectoryPointerTableIndex] & I_AlignMask);

				if(PageDirectory)
				{
					PageMapStructure* PageTable = (PageMapStructure*)(PageDirectory->Entry[PageDirectoryIndex] & I_AlignMask);

					PageTable->Entry[PageTableIndex] = 0;
					invlpg((PageMapStructure*) VirtAddr);
				}
			}
		}


		if(DidMapPML4 && !DoNotUnmap)
		{
			DidMapPML4 = false;
			if((uint64_t) PML4 != GetKernelCR3()){ UnmapAddress((uint64_t) PML4); }
		}
	}



	void MapRegion(uint64_t VirtAddr, uint64_t PhysAddr, uint64_t LengthInPages, uint64_t Flags, PageMapStructure* PML4)
	{
		for(uint64_t i = 0; i < LengthInPages; i++)
			MapAddress(VirtAddr + (i * 0x1000), PhysAddr + (i * 0x1000), Flags, PML4);
	}

	void UnmapRegion(uint64_t VirtAddr, uint64_t LengthInPages, PageMapStructure* PML4)
	{
		for(uint64_t i = 0; i < LengthInPages; i++)
			UnmapAddress(VirtAddr + (i * 0x1000), PML4);
	}


	uint64_t* GetPageTableEntry(uint64_t va, PageMapStructure* VAS, PageMapStructure** pdpt, PageMapStructure** pd, PageMapStructure** pt)
	{

		uint64_t PageTableIndex					= I_PT_INDEX(va);
		uint64_t PageDirectoryIndex				= I_PD_INDEX(va);
		uint64_t PageDirectoryPointerTableIndex	= I_PDPT_INDEX(va);
		uint64_t PML4TIndex						= I_PML4_INDEX(va);

		PageMapStructure* PML = (VAS == 0 ? GetCurrentPML4T() : VAS);
		bool other = (PML != GetCurrentPML4T());

		assert(PML);
		{
			if(other)
				Virtual::MapAddress((uint64_t) PML, (uint64_t) PML, 0x7);

			PageMapStructure* PDPT = (PageMapStructure*) (PML->Entry[PML4TIndex] & I_AlignMask);

			assert(PDPT);
			{
				if(other)
					Virtual::MapAddress((uint64_t) PDPT, (uint64_t) PDPT, 0x7);

				PageMapStructure* PageDirectory = (PageMapStructure*) (PDPT->Entry[PageDirectoryPointerTableIndex] & I_AlignMask);

				assert(PageDirectory);
				{
					if(other)
						Virtual::MapAddress((uint64_t) PageDirectory, (uint64_t) PageDirectory, 0x7);

					PageMapStructure* PageTable = (PageMapStructure*) (PageDirectory->Entry[PageDirectoryIndex] & I_AlignMask);

					assert(PageTable);
					{
						if(other)
							Virtual::MapAddress((uint64_t) PageTable, (uint64_t) PageTable, 0x7);

						*pdpt = PDPT;
						*pd = PageDirectory;
						*pt = PageTable;

						uint64_t* ret = &PageTable->Entry[PageTableIndex];
						return ret;
					}
				}
			}
		}
	}


	uint64_t GetMapping(uint64_t VirtAddr, PageMapStructure* VAS)
	{
		PageMapStructure* pdpt = 0;
		PageMapStructure* pd = 0;
		PageMapStructure* pt = 0;

		uint64_t ret = *GetPageTableEntry(VirtAddr, VAS, &pdpt, &pd, &pt);

		if(VAS != GetCurrentPML4T())
		{
			UnmapAddress((uintptr_t) pdpt);
			UnmapAddress((uintptr_t) pd);
			UnmapAddress((uintptr_t) pt);
		}

		return ret;
	}


	static void ChangeCOWFlag(uint64_t virt, PageMapStructure* pml, bool cow)
	{
		PageMapStructure* pdpt = 0;
		PageMapStructure* pd = 0;
		PageMapStructure* pt = 0;


		uint64_t* pg = GetPageTableEntry(virt, pml, &pdpt, &pd, &pt);

		uint64_t* pdptv = (uint64_t*) pdpt;
		uint64_t* pdv = (uint64_t*) pd;
		uint64_t* ptv = (uint64_t*) pt;

		// Log("virt: %x, pdpt: %x, pd: %x, pt: %x, pg: %x", virt, pdptv, pdv, ptv, pg);
		// UHALT();

		if(cow)
		{
			*pdptv |= I_CopyOnWrite;
			*pdptv &= ((uint64_t) ~I_ReadWrite);

			*pdv |= I_CopyOnWrite;
			*pdv &= ((uint64_t) ~I_ReadWrite);

			*ptv |= I_CopyOnWrite;
			*ptv &= ((uint64_t) ~I_ReadWrite);

			*pg |= I_CopyOnWrite;
			*pg &= ((uint64_t) ~I_ReadWrite);
		}
		else
		{
			*pdptv &= ((uint64_t) ~I_CopyOnWrite);
			*pdptv |= I_ReadWrite;

			*pdv &= ((uint64_t) ~I_CopyOnWrite);
			*pdv |= I_ReadWrite;

			*ptv &= ((uint64_t) ~I_CopyOnWrite);
			*ptv |= I_ReadWrite;

			*pg &= ((uint64_t) ~I_CopyOnWrite);
			*pg |= I_ReadWrite;
		}


		if(pml != GetCurrentPML4T())
		{
			UnmapAddress((uintptr_t) pdpt);
			UnmapAddress((uintptr_t) pd);
			UnmapAddress((uintptr_t) pt);
		}
	}

	void MarkCOW(uint64_t virt, PageMapStructure* pml)
	{
		ChangeCOWFlag(virt, pml, 1);
	}

	void UnmarkCOW(uint64_t virt, PageMapStructure* pml)
	{
		ChangeCOWFlag(virt, pml, 0);
	}

	void MarkCOW(uint64_t virt, VirtualAddressSpace* vas)
	{
		MarkCOW(virt, vas ? vas->PML4 : GetCurrentPML4T());
	}

	void UnmarkCOW(uint64_t virt, VirtualAddressSpace* vas)
	{
		MarkCOW(virt, vas ? vas->PML4 : GetCurrentPML4T());
	}
}
}
}
}
