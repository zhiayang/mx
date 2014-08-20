// Virtual.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


// Sets up the paging structures left by our bootstrap in (boot.s).

#include <Kernel.hpp>
#include <List.hpp>
#include <StandardIO.hpp>
#include <stddef.h>

using namespace Kernel;
using namespace Kernel::HardwareAbstraction::MemoryManager;
using namespace Kernel::HardwareAbstraction::MemoryManager::Virtual;

namespace Kernel {
namespace HardwareAbstraction {
namespace MemoryManager {
namespace Virtual
{
	static PageMapStructure* CurrentPML4T;
	static bool IsPaging;

	#define I_RECURSIVE_SLOT      510


	// Convert an address into array index of a structure
	// E.G. int index = I_PML4_INDEX(0xFFFFFFFFFFFFFFFF); // index = 511
	#define I_PML4_INDEX(addr)		((((uintptr_t)(addr))>>39) & 511)
	#define I_PDPT_INDEX(addr)		((((uintptr_t)(addr))>>30) & 511)
	#define I_PD_INDEX(addr)		((((uintptr_t)(addr))>>21) & 511)
	#define I_PT_INDEX(addr)		((((uintptr_t)(addr))>>12) & 511)




	void Initialise()
	{
		PageMapStructure* const OriginalPml4 = (PageMapStructure* const) GetKernelCR3();

		OriginalPml4->Entry[I_RECURSIVE_SLOT] = (uint64_t) OriginalPml4 | I_Present | I_ReadWrite;
		CurrentPML4T = OriginalPml4;
		IsPaging = true;
	}



	// this models (exactly) the FPL pair system of the physical allocator.
	uint64_t AllocateVirtual(uint64_t size, uint64_t addr, VirtualAddressSpace* v)
	{
		Multitasking::Process* proc = Multitasking::GetCurrentProcess();
		assert(proc);

		VirtualAddressSpace* vas = v ? v : proc->VAS;
		assert(vas);

		if(vas->pairs->Size() == 0)
		{
			Log(3, "Virtual address space exhausted, WTF are you doing?!");
			Multitasking::Kill(proc);
			UHALT();
			return 0;
		}

		// look for address first.
		if(addr > 0)
		{
			uint64_t end = addr + (size * 0x1000);
			// Log("Trying to allocate %d page%s at %x -- end = %x", size, size > 1 ? "s" : "", addr, end);
			AddressLengthPair* found = 0;
			for(auto pair : *vas->pairs)
			{
				// Log("Found pair: %x - %x, overflow check: %.016x", pair->start, pair->length, pair->start + (pair->length * 0x1000));
				if(pair->start <= addr && pair->start + (pair->length * 0x1000) >= end)
				{
					found = pair;
					break;
				}
			}

			if(found)
			{
				if(found->start == addr && found->length == size)
				{
					vas->pairs->Remove(found);
					delete found;
				}
				else if(found->start == addr)
				{
					AddressLengthPair* np = new AddressLengthPair(end, found->length - size);
					vas->pairs->InsertBack(np);
					delete found;
				}
				else
				{
					// make a new pair.
					AddressLengthPair* np = new AddressLengthPair(found->start, (addr - found->start) / 0x1000);
					vas->pairs->InsertBack(np);

					found->start = end;
					found->length = found->length - size;
				}

				// Log("Allocated %x virtual as requested", addr);
				return addr;
			}

			// else we just continue as normal.
			Log(1, "Couldn't satisfy request to allocate virtual address at %x", addr);
		}














		AddressLengthPair* pair = vas->pairs->Front();
		if(pair->length > size)
		{
			uint64_t ret = pair->start;
			pair->start += (size * 0x1000);
			pair->length -= size;
			return ret;
		}
		else if(pair->length == size)
		{
			uint64_t ret = pair->start;
			delete vas->pairs->RemoveFront();
			return ret;
		}
		else
		{
			// we need to use the next pair.
			AddressLengthPair* found = 0;
			for(AddressLengthPair* p : *vas->pairs)
			{
				if(p->length >= size)
				{
					found = p;
					break;
				}
			}

			if(!found)
			{
				Log(3, "Virtual address space exhausted, WTF are you doing?! HUH?");
				Multitasking::Kill(proc);
				UHALT();
			}
			else
			{
				if(found->length > size)
				{
					uint64_t ret = found->start;
					found->length -= size;
					found->start += (size * 0x1000);
					return ret;
				}
				else
				{
					uint64_t ret = found->start;
					delete found;
					return ret;
				}
			}

			HALT("something went wrong");
			return 0;
		}
	}

	void FreeVirtual(uint64_t page, uint64_t size, VirtualAddressSpace* v)
	{
		Multitasking::Process* proc = Multitasking::GetCurrentProcess();
		assert(proc);

		VirtualAddressSpace* vas = v ? v : proc->VAS;
		assert(vas);

		uint64_t end = page + (size * 0x1000);

		for(AddressLengthPair* pair : *vas->pairs)
		{
			// 3 basic conditions
			// 1. we find a match below a pair's baseaddr
			// 2. we find a match above a pair's baseaddr
			// 3. we don't find a match

			if(end == pair->start)
			{
				pair->start = page;
				pair->length += size;
				return;
			}
			else if(pair->start + (pair->length * 0x1000) == page)
			{
				pair->length += (size * 0x1000);
				return;
			}
		}

		// if we reach here, we haven't found a match.
		vas->pairs->InsertBack(new AddressLengthPair(page, size));
	}

	VirtualAddressSpace* SetupVAS(VirtualAddressSpace* vas)
	{
		// Max 48-bit virtual address space (current implementations)
		vas->pairs->InsertBack(new AddressLengthPair(0x01000000, 0xFF000));
		vas->pairs->InsertBack(new AddressLengthPair(0xFFFFF00000000000, 0x100000));

		return vas;
	}

	uint64_t AllocatePage(uint64_t size, uint64_t addr, uint64_t flags)
	{
		uint64_t phys = Physical::AllocatePage(size);
		uint64_t virt = AllocateVirtual(size, addr);

		MapRegion(virt, phys, size, flags);
		return virt;
	}

	void FreePage(uint64_t addr, uint64_t size)
	{
		uint64_t phys = GetMapping(addr, 0);
		Physical::FreePage(phys, size);
		UnMapRegion(addr, size);
	}































	void SwitchPML4T(PageMapStructure* PML4T)
	{
		CurrentPML4T = PML4T;
	}

	PageMapStructure* GetCurrentPML4T()
	{
		return CurrentPML4T;
	}

	static void invlpg(PageMapStructure* p)
	{
		if(Multitasking::CurrentProcessInRing3())
		{
			asm volatile("mov $0x8, %%r10; mov %[c], %%rdi; int $0xF8" :: [c]"r"((uint64_t) p) : "%r10");
		}
		else
		{
			asm volatile("invlpg (%0)" : : "a" ((uint64_t) p));
		}
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



	void UnMapAddress(uint64_t VirtAddr)
	{
		UnMapAddress(VirtAddr, GetCurrentPML4T(), false);
	}

	void UnMapAddress(uint64_t VirtAddr, PageMapStructure* PML4)
	{
		UnMapAddress(VirtAddr, PML4, false);
	}

	void UnMapAddress(uint64_t VirtAddr, bool DoNotUnmap)
	{
		UnMapAddress(VirtAddr, GetCurrentPML4T(), DoNotUnmap);
	}










	void MapAddress(uint64_t VirtAddr, uint64_t PhysAddr, uint64_t Flags, PageMapStructure* PML4, bool DoNotUnmap)
	{
		uint64_t PageTableIndex			= I_PT_INDEX(VirtAddr);
		uint64_t PageDirectoryIndex			= I_PD_INDEX(VirtAddr);
		uint64_t PageDirectoryPointerTableIndex	= I_PDPT_INDEX(VirtAddr);
		uint64_t PML4TIndex				= I_PML4_INDEX(VirtAddr);

		assert(PageTableIndex < 512);
		assert(PageDirectoryIndex < 512);
		assert(PageDirectoryPointerTableIndex < 512);
		assert(PML4TIndex < 512);

		if(PML4TIndex == I_RECURSIVE_SLOT)
		{
			// We can't map 510, we need that for our recursive mapping.
			HALT("Tried to map to PML4[510]! (RESTRICTED, KERNEL USE)");
		}


		// Because these are indexes into structures, we need to use MODULO '%'
		// To change them to relative indexes, not absolute ones.

		(void) DoNotUnmap;


		PageMapStructure* PML = (PML4 == 0 ? GetCurrentPML4T() : PML4);
		bool other = (PML != GetCurrentPML4T());
		// bool other = false;
		assert(PML);

		if(other)
			Virtual::MapAddress((uint64_t) PML, (uint64_t) PML, 0x7);



		if(!(PML->Entry[PML4TIndex] & I_Present))
		{
			PML->Entry[PML4TIndex] = Physical::AllocateFromReserved() | (Flags | 0x1);
			invlpg(PML);
		}
		PageMapStructure* PDPT = (PageMapStructure*)(PML->Entry[PML4TIndex] & I_AlignMask);
		if(other)
			Virtual::MapAddress((uint64_t) PDPT, (uint64_t) PDPT, 0x7);



		if(!(PDPT->Entry[PageDirectoryPointerTableIndex] & I_Present))
		{
			PDPT->Entry[PageDirectoryPointerTableIndex] = Physical::AllocateFromReserved() | (Flags | 0x1);
			invlpg(PML);
			invlpg(PDPT);
		}
		PageMapStructure* PageDirectory = (PageMapStructure*)(PDPT->Entry[PageDirectoryPointerTableIndex] & I_AlignMask);
		if(other)
			Virtual::MapAddress((uint64_t) PageDirectory, (uint64_t) PageDirectory, 0x7);



		if(!(PageDirectory->Entry[PageDirectoryIndex] & I_Present))
		{
			PageDirectory->Entry[PageDirectoryIndex] = Physical::AllocateFromReserved() | (Flags | 0x1);
			invlpg(PML);
			invlpg(PDPT);
			invlpg(PageDirectory);
		}
		PageMapStructure* PageTable = (PageMapStructure*)(PageDirectory->Entry[PageDirectoryIndex] & I_AlignMask);
		if(other)
			Virtual::MapAddress((uint64_t) PageTable, (uint64_t) PageTable, 0x7);

		PageTable->Entry[PageTableIndex] = PhysAddr | Flags;
		invlpg(PML);
		invlpg(PDPT);
		invlpg(PageDirectory);
		invlpg(PageTable);

		if(other)
		{
			Virtual::UnMapAddress((uint64_t) PageTable);
			Virtual::UnMapAddress((uint64_t) PageDirectory);
			Virtual::UnMapAddress((uint64_t) PDPT);
			Virtual::UnMapAddress((uint64_t) PML);
		}
		return;



























		// // FIX FOR ALLOCATERESERVED:
		// // change the system to reserve a region in the virtual address space for temporary mappings.
		// // when creating a new strucutre, allocate_nomap a page, then map it to this temporary region
		// // to modify it.

		// bool DidMapPML4 = false;

		// if(PML4 == 0)
		// 	PML4 = GetCurrentPML4T();

		// else if((uint64_t) PML4 != (uint64_t) GetCurrentPML4T())
		// {
		// 	DidMapPML4 = true;
		// 	MapAddress((uint64_t) PML4, (uint64_t) PML4, 0x03, DoNotUnmap);
		// }

		// VirtAddr &= I_AlignMask;
		// PhysAddr &= I_AlignMask;


		// // First, find out which page we will need.

		// uint64_t PageTableIndex			= I_PT_INDEX(VirtAddr);
		// uint64_t PageDirectoryIndex			= I_PD_INDEX(VirtAddr);
		// uint64_t PageDirectoryPointerTableIndex	= I_PDPT_INDEX(VirtAddr);
		// uint64_t PML4TIndex				= I_PML4_INDEX(VirtAddr);

		// assert(PageTableIndex < 512);
		// assert(PageDirectoryIndex < 512);
		// assert(PageDirectoryPointerTableIndex < 512);
		// assert(PML4TIndex < 512);

		// if(PML4TIndex == I_RECURSIVE_SLOT)
		// {
		// 	// We can't map 510, we need that for our recursive mapping.
		// 	HALT("Tried to map to PML4[510]! (RESTRICTED, KERNEL USE)");
		// }

		// if(!PML4)
		// 	HALT("PML4 Corrupted!");


		// // Now we know where all the stuff is at, let's start mapping.
		// // First, we check if the desired PDPT is present:

		// if(!(PML4->Entry[PML4TIndex] & I_Present))
		// {
		// 	PML4->Entry[PML4TIndex] = Physical::AllocateFromReserved() | (Flags | 0x1);
		// 	invlpg(PML4);
		// }

		// // Continue with our business here.
		// // Check the PD:
		// PageMapStructure* PDPT = (PageMapStructure*)(PML4->Entry[PML4TIndex] & I_AlignMask);

		// if(!(PDPT->Entry[PageDirectoryPointerTableIndex] & I_Present))
		// {
		// 	PDPT->Entry[PageDirectoryPointerTableIndex] = Physical::AllocateFromReserved() | (Flags | 0x1);
		// 	invlpg(PDPT);
		// }

		// // Next, we must check if the Page Table is present:
		// PageMapStructure* PageDirectory = (PageMapStructure*)(PDPT->Entry[PageDirectoryPointerTableIndex] & I_AlignMask);

		// if(!(PageDirectory->Entry[PageDirectoryIndex] & I_Present))
		// {
		// 	PageDirectory->Entry[PageDirectoryIndex] = Physical::AllocateFromReserved() | (Flags | 0x1);
		// 	invlpg(PageDirectory);
		// }

		// PageMapStructure* PageTable = (PageMapStructure*)(PageDirectory->Entry[PageDirectoryIndex] & I_AlignMask);
		// // we can only use actual flags for the page-entry -- if we decide to map a guard page, the higher-level structures still
		// // need to be marked present.

		// PageTable->Entry[PageTableIndex] = (PhysAddr & I_AlignMask) | Flags;
		// invlpg(PageTable);

		// if(DidMapPML4)
		// {
		// 	if((uint64_t) PML4 != GetKernelCR3() && !DoNotUnmap)
		// 	{
		// 		UnMapAddress((uint64_t) PML4);
		// 	}
		// }

		// return;
	}





	void UnMapAddress(uint64_t VirtAddr, PageMapStructure* PML4, bool DoNotUnmap)
	{
		bool DidMapPML4 = false;

		if(PML4 == 0)
			PML4 = GetCurrentPML4T();

		else if(PML4 != GetCurrentPML4T())
		{
			DidMapPML4 = true;
			MapAddress((uint64_t) PML4, (uint64_t) PML4, 0x03);
		}


		// First, find out which page we will need.
		uint64_t PageTableIndex = VirtAddr / 0x1000;

		// Next, which Page Table does that page reside in?
		// Let's find out:
		uint64_t PageDirectoryIndex = PageTableIndex / 512;

		// Page Directory:
		uint64_t PageDirectoryPointerTableIndex = PageDirectoryIndex / 512;

		// Finally, which PDPT is it in?
		uint64_t PML4TIndex = PageDirectoryPointerTableIndex / 512;



		// Because these are indexes into structures, we need to use MODULO '%'
		// To change them to relative indexes, not absolute ones.

		PageTableIndex %= 512;
		PageDirectoryIndex %= 512;
		PageDirectoryPointerTableIndex %= 512;


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
			if((uint64_t) PML4 != GetKernelCR3()){ UnMapAddress((uint64_t) PML4); }
		}
	}



	void MapRegion(uint64_t VirtAddr, uint64_t PhysAddr, uint64_t LengthInPages, uint64_t Flags, PageMapStructure* PML4)
	{
		for(uint64_t i = 0; i < LengthInPages; i++)
		{
			MapAddress(VirtAddr + (i * 0x1000), PhysAddr + (i * 0x1000), Flags, PML4);
		}
	}

	void UnMapRegion(uint64_t VirtAddr, uint64_t LengthInPages, PageMapStructure* PML4)
	{
		for(uint64_t i = 0; i < LengthInPages; i++)
		{
			UnMapAddress(VirtAddr + (i * 0x1000), PML4);
		}
	}


	uint64_t GetMapping(uint64_t VirtAddr, PageMapStructure* VAS)
	{
		// First, find out which page we will need.
		uint64_t PageTableIndex = VirtAddr / 0x1000;

		// Next, which Page Table does that page reside in?
		// Let's find out:
		uint64_t PageDirectoryIndex = PageTableIndex / 512;

		// Page Directory:
		uint64_t PageDirectoryPointerTableIndex = PageDirectoryIndex / 512;

		// Finally, which PDPT is it in?
		uint64_t PML4TIndex = PageDirectoryPointerTableIndex / 512;



		// Because these are indexes into structures, we need to use MODULO '%'
		// To change them to relative indexes, not absolute ones.



		PML4TIndex %= 512;
		PageTableIndex %= 512;
		PageDirectoryIndex %= 512;
		PageDirectoryPointerTableIndex %= 512;

		PageMapStructure* PML = (VAS == 0 ? GetCurrentPML4T() : VAS);
		bool other = (PML != GetCurrentPML4T());

		if(PML)
		{
			if(other)
				Virtual::MapAddress((uint64_t) PML, (uint64_t) PML, 0x7);

			PageMapStructure* PDPT = (PageMapStructure*)(PML->Entry[PML4TIndex] & I_AlignMask);

			if(PDPT)
			{
				if(other)
					Virtual::MapAddress((uint64_t) PDPT, (uint64_t) PDPT, 0x7);

				PageMapStructure* PageDirectory = (PageMapStructure*)(PDPT->Entry[PageDirectoryPointerTableIndex] & I_AlignMask);

				if(PageDirectory)
				{
					if(other)
						Virtual::MapAddress((uint64_t) PageDirectory, (uint64_t) PageDirectory, 0x7);

					PageMapStructure* PageTable = (PageMapStructure*)(PageDirectory->Entry[PageDirectoryIndex] & I_AlignMask);

					if(PageTable)
					{
						if(other)
							Virtual::MapAddress((uint64_t) PageTable, (uint64_t) PageTable, 0x7);

						uint64_t ret = (uint64_t)(PageTable->Entry[PageTableIndex] & I_AlignMask);

						if(other)
						{
							Virtual::UnMapAddress((uint64_t) PageTable);
							Virtual::UnMapAddress((uint64_t) PageDirectory);
							Virtual::UnMapAddress((uint64_t) PDPT);
							Virtual::UnMapAddress((uint64_t) PML);
						}
						return ret;
					}
					else
					{
						if(other)
						{
							Virtual::UnMapAddress((uint64_t) PageDirectory);
							Virtual::UnMapAddress((uint64_t) PDPT);
							Virtual::UnMapAddress((uint64_t) PML);
						}
						return 0;
					}
				}
				else
				{
					if(other)
					{
						Virtual::UnMapAddress((uint64_t) PDPT);
						Virtual::UnMapAddress((uint64_t) PML);
					}
					return 0;
				}
			}
			else
			{
				if(other)
				{
					Virtual::UnMapAddress((uint64_t) PML);
				}
				return 0;
			}
		}
		else
		{
			return 0;
		}
	}

	bool GetPagingFlag()
	{
		return IsPaging;
	}












	uint64_t CreateVAS()
	{
		/*
			256 TB
			512 GB
			1 GB
			2 MB

			bottom 1gb is kernel owned.
			top 1024gb is also kernel owned.
		*/
		PageMapStructure* kernelpml4 = (PageMapStructure*) Kernel::GetKernelCR3();
		PageMapStructure* pdpt = (PageMapStructure*) kernelpml4->Entry[0];
		// pdpt is guaranteed to exist.

		// bottom 1gb
		PageMapStructure* pt = (PageMapStructure*) pdpt->Entry[0];

		// we need to throw this address (aka pointer) into the created things.




		PageMapStructure* PML4 = (PageMapStructure*) Physical::AllocateFromReserved();
		Virtual::MapAddress((uint64_t) PML4, (uint64_t) PML4, 0x07, PML4);
		Virtual::MapAddress((uint64_t) PML4, (uint64_t) PML4, 0x03, true);
		PML4->Entry[I_RECURSIVE_SLOT] = (uint64_t) PML4 | I_Present | I_ReadWrite;

		// do it here.
		{
			if(!(PML4->Entry[0] & I_Present))
				PML4->Entry[0] = Physical::AllocateFromReserved() | I_Present | I_ReadWrite | I_UserAccess;

			((PageMapStructure*) (PML4->Entry[0]))->Entry[0] = (uint64_t) pt;


			// that just gives us 1gb lower, now we need 512gb upper.

			// if(!(PML4->Entry[511] & I_Present))
			// 	PML4->Entry[511] = Physical::AllocateFromReserved() | I_Present | I_ReadWrite | I_UserAccess;

			PML4->Entry[511] = (uint64_t) kernelpml4->Entry[511] | 0x6;
		}


		// Map 8 MB, includes the kernel.
		for(uint64_t i = 0; i < (8 * 0x01000000); i += 0x1000)
			Virtual::MapAddress(i, i, 0x07, PML4);

		// Map the LFB.
		for(uint64_t i = 0; i < Kernel::GetLFBLengthInPages(); i++)
			Virtual::MapAddress(Kernel::GetFramebufferAddress() + (i * 0x1000), Kernel::GetFramebufferAddress() + (i * 0x1000), I_Present | I_ReadWrite | I_UserAccess, PML4);


		// map the physical FPL.
		// Virtual::MapAddress(FPLAddress, Physical::GetPhysicalFPLAddress(), I_Present | I_ReadWrite | I_UserAccess, PML4);
		return (uint64_t) PML4;
	}


	void MapToAllProcesses(uint64_t v, uint64_t p, uint64_t f)
	{
		for(uint64_t d = 0; d < Multitasking::ProcessList->Size(); d++)
		{
			MapAddress(v, p, f, (PageMapStructure*) Multitasking::ProcessList->Get(d)->CR3);
		}
	}
}
}
}
}






















