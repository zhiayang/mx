// Virtual.cpp
// Copyright (c) 2013 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#include <Kernel.hpp>
#include <StandardIO.hpp>
#include <stddef.h>

#include <rdestl/rdestl.h>

namespace Kernel {
namespace HardwareAbstraction {
namespace MemoryManager {
namespace Virtual
{
	static uint64_t FinaliseRegion(MemRegion* region, uint64_t phys, void* pml4)
	{
		region->used = 1;
		region->phys = phys;

		// Log(1, "allocated %d pages at %x, in pml %x", region->length, region->start, pml4);

		return region->start;
	}



	uint64_t AllocateVirtual(uint64_t size, uint64_t addr, VirtualAddressSpace* _v, uint64_t phys)
	{
		VirtualAddressSpace* vas = (_v ? _v : &Multitasking::GetCurrentProcess()->VAS);

		assert(vas);
		assert(vas->mtx);
		// assert(vas->regions);

		AutoMutex mtx(*vas->mtx);

		for(MemRegion* region : vas->regions)
		{
			assert(region->length > 0);
			assert(region->start > 0);

			uint64_t regionEnd = (region->start + (region->length * 0x1000));

			// look for a free region
			if(region->used == 0)
			{
				if(addr != 0)
				{
					if(region->start <= addr && regionEnd >= (addr + (size * 0x1000)))
					{
						assert(region->length >= size);

						// do stuff.
						if(region->start == addr)
						{
							if(region->length == size)
							{
								return FinaliseRegion(region, phys, vas->PML4);
							}
							else
							{
								// create a new region bit.
								uint64_t newsz = region->length - size;
								uint64_t newst = region->start + (size * 0x1000);

								assert(newsz > 0);
								assert(newst > 0);

								MemRegion* newr = new MemRegion();
								newr->start = newst;
								newr->length = newsz;
								newr->used = 0;
								newr->phys = 0;

								vas->regions.push_back(newr);

								region->length = size;

								return FinaliseRegion(region, phys, vas->PML4);
							}
						}
						else
						{
							// might need to split into 3
							// definitely need to split off the front bit though.
							// (THE FRONT FELL OFF??!!)

							MemRegion* front = new MemRegion();
							front->start = region->start;
							front->length = (addr - region->start) / 0x1000;
							front->used = 0;
							front->phys = 0;
							vas->regions.push_back(front);

							region->start = addr;
							region->length -= front->length;


							assert(region->length >= size);

							// now see if we need to split off the back bit
							if(region->length > size)
							{
								MemRegion* back = new MemRegion();
								back->start = region->start + (size * 0x1000);
								back->length = region->length - size;
								back->used = 0;
								back->phys = 0;

								region->length = size;

								assert(back->start > 0);
								assert(back->length > 0);

								vas->regions.push_back(back);
							}

							return FinaliseRegion(region, phys, vas->PML4);
						}
					}
				}
				else
				{
					if(region->length == size)
					{
						// mark the whole thing as used.
						return FinaliseRegion(region, phys, vas->PML4);
					}
					else if(region->length > size)
					{
						// create a new region bit.
						uint64_t newsz = region->length - size;
						uint64_t newst = region->start + (size * 0x1000);

						assert(newsz > 0);
						assert(newst > 0);

						MemRegion* newr = new MemRegion();
						newr->start = newst;
						newr->length = newsz;
						newr->used = 0;
						newr->phys = 0;

						vas->regions.push_back(newr);

						region->length = size;

						return FinaliseRegion(region, phys, vas->PML4);
					}
				}
			}
		}


		// if we got here, then we're fucked
		if(addr != 0)
		{
			Log(LOG_CRIT, "Failed to allocate virtual memory at desired address (%x, %d pages), returning 0!", addr, size);
			UHALT();

			return 0;
		}
		else
		{
			Log(LOG_CRIT, "Virtual address space exhausted????");
			UHALT();

			return 0;
		}
	}

	uint64_t AllocatePage(uint64_t size, uint64_t addr, uint64_t flags)
	{
		uint64_t phys = Physical::AllocatePage(size);

		// store the 'flags' we used in the 'phys' field.
		uint64_t virt = AllocateVirtual(size, addr, 0, phys | flags);

		MapRegion(virt, phys & ((uint64_t) ~0xFFF), size, flags);

		return virt;
	}

	static MemRegion* _FreeVirtual(uint64_t addr, uint64_t size, VirtualAddressSpace* _v)
	{
		if(size == 0) return 0;

		VirtualAddressSpace* vas = (_v ? _v : &Multitasking::GetCurrentProcess()->VAS);

		assert(vas);
		assert(vas->mtx);
		// assert(vas->regions);

		AutoMutex mtx(*vas->mtx);


		MemRegion* offending = 0;

		for(MemRegion* region : vas->regions)
		{
			if(region->start == addr && region->length == size)
			{
				assert(region->used);
				vas->regions.remove(region);

				offending = region;
				break;
			}
		}

		if(offending == 0)
			return 0;

		assert(offending);
		offending->used = 1;

		// loop through again.
		for(MemRegion* region : vas->regions)
		{
			// simple things:
			// check if we can either fit in below, or above
			if(offending->start + (offending->length * 0x1000) == region->start && region->used == 0)
			{
				// fit in below.
				region->start = offending->start;
				region->length += offending->length;

				return offending;	// can delete.
			}
			else if(region->start + (region->length * 0x1000) == offending->start && region->used == 0)
			{
				// fit in above.
				region->length += offending->length;

				return offending;	// can delete.
			}
		}

		// else.
		offending->used = 0;
		vas->regions.push_back(offending);
		return offending;	// can't delete region.
	}

	void FreeVirtual(uint64_t addr, uint64_t size, VirtualAddressSpace* _v)
	{
		MemRegion* region = _FreeVirtual(addr, size, _v);

		assert(region);
		if(region->used) delete region;
	}

	void FreePage(uint64_t addr, uint64_t size)
	{
		// Log(1, "trying to free %d pages at %x (%x)", size, addr, __builtin_return_address(0));
		MemRegion* region = _FreeVirtual(addr, size, 0);
		assert(region);

		// note: we stored the flags we used to map in 'phys'. throw it away.
		uint64_t phys = region->phys & ((uint64_t) ~0xFFF);

		assert(phys > 0);
		assert(region->length > 0);

		UnmapRegion(region->start, region->length);
		Physical::FreePage(phys, region->length);

		if(region->used) delete region;
	}


	VirtualAddressSpace* SetupVAS(VirtualAddressSpace* vas)
	{
		assert(vas);
		// vas->regions = new stl::vector<MemRegion*>();
		vas->regions = rde::vector<MemRegion*>();

		// Max 48-bit virtual address space (current implementations)
		MemRegion* r1 = new MemRegion();
		MemRegion* r2 = new MemRegion();

		r1->start = 0x01000000;
		r1->length = 0xFF000;
		r1->used = 0;
		r1->phys = 0;


		r2->start = 0xFFFFF00000000000;
		r2->length = 0x100000;
		r2->used = 0;
		r2->phys = 0;

		vas->regions.push_back(r1);
		vas->regions.push_back(r2);

		// vas->mtx = new Mutex();
		return vas;
	}

	void DestroyVAS(VirtualAddressSpace* vas)
	{
		// todo

		(void) vas;
	}


	uint64_t GetVirtualPhysical(uint64_t virt, VirtualAddressSpace* v)
	{
		VirtualAddressSpace* vas = v ? v : &Multitasking::GetCurrentProcess()->VAS;
		assert(vas);
		// assert(vas->regions);

		for(MemRegion* region : vas->regions)
		{
			// assert(region);
			assert(region->start > 0);
			assert(region->length > 0);

			uint64_t end = region->start + (region->length * 0x1000);

			if(virt >= region->start && virt <= end)
			{
				assert(region->phys > 0);
				return region->phys + (virt - region->start);
			}
		}

		return 0;
	}

	void ForceInsertALPTuple(uint64_t addr, size_t sizeInPages, uint64_t phys, VirtualAddressSpace* vas)
	{
		// todo???
		(void) addr;
		(void) sizeInPages;
		(void) phys;
		(void) vas;
	}
















	void CopyFromKernel(uint64_t fromAddr, uint64_t toAddr, size_t bytes, VirtualAddressSpace* to)
	{
		// this can only be done in the kernel
		assert(Multitasking::GetProcess(0) == Multitasking::GetCurrentProcess());

		VirtualAddressSpace* from = &Multitasking::GetProcess(0)->VAS;


		size_t numPages			= (bytes + 0xFFF) / 0x1000;
		uint64_t toAligned		= toAddr & I_AlignMask;
		uint64_t beginOffset	= toAddr - toAligned;
		uint64_t toPhys			= GetVirtualPhysical(toAligned, to);

		if(toPhys == 0)
		{
			Log(1, "Could not fetch physical address for %x in vas %x", toAligned, to->PML4);
			toPhys = GetMapping(toAligned, to->PML4) & I_AlignMask;
		}

		assert(toPhys > 0);

		// allocate some temporary virt pages in the local address space
		// then map the physical pages behind toAddr to our local space
		uint64_t toVirtTemp		= AllocateVirtual(numPages, 0, from, toPhys);
		Virtual::MapRegion(toVirtTemp, toPhys, numPages, 0x3);

		// then copy it.
		// Log("Copied %d bytes from %x to %x (%x, %x, %x)", bytes, fromAddr, toAddr, beginOffset, toPhys, to->PML4);
		Memory::Copy((void*) (toVirtTemp + beginOffset), (void*) fromAddr, bytes);

		FreeVirtual(toVirtTemp, numPages);
		Virtual::UnmapRegion(toVirtTemp, numPages);
	}

	void CopyToKernel(uint64_t fromAddr, uint64_t toAddr, size_t bytes, VirtualAddressSpace* from)
	{
		// this can only be done in the kernel
		assert(Multitasking::GetProcess(0) == Multitasking::GetCurrentProcess());

		VirtualAddressSpace* to = &Multitasking::GetProcess(0)->VAS;
		size_t numPages			= (bytes + 0xFFF) / 0x1000;

		uint64_t fromAligned	= fromAddr & I_AlignMask;
		uint64_t beginOffset	= fromAddr - fromAligned;
		uint64_t fromPhys		= GetVirtualPhysical(fromAligned, from);

		// special exception for kernel heap
		if(fromPhys == 0 && fromAddr >= KernelHeapAddress)
			fromPhys = GetVirtualPhysical(fromAligned, to);

		if(fromPhys == 0) Log(1, "Could not fetch physical address for %x in vas %x", fromAligned, from->PML4);
		assert(fromPhys > 0);

		// allocate some temporary virt pages in the local address space
		// then map the physical pages behind toAddr to our local space
		uint64_t fromPhysMapped = AllocateVirtual(numPages, 0, to, fromPhys);
		Virtual::MapRegion(fromPhysMapped, fromPhys, numPages, 0x3);

		// then copy it.
		Memory::Copy((void*) toAddr, (void*) (fromPhysMapped + beginOffset), bytes);

		FreeVirtual(fromPhysMapped, numPages);
		Virtual::UnmapRegion(fromPhysMapped, numPages);
	}































	VirtualAddressSpace* CopyVAS(VirtualAddressSpace* src, VirtualAddressSpace* dest)
	{
		assert(src);
		assert(dest);

		// no need to lock source, but lock dest.
		LOCK(*dest->mtx);

		assert(dest->regions.size() == 0);

		// shitty, manual old-school stuff.
		// enter the matrix of the recursive thing
		// which i probably didn't do properly.
		{

			// I_RECURSIVE_SLOT
		}









		// proper, clean stuff.
		for(auto pair : src->regions)
		{
			MemRegion* reg = new MemRegion();
			reg->length	= pair->length;
			reg->phys	= pair->phys;
			reg->start	= pair->start;
			reg->used	= pair->used;

			if(pair->used)
			{
				uint64_t p = Physical::AllocatePage(pair->length);

				// todo: use Copy-on-write (COW) for this instead of allocating a new page
				Virtual::MapRegion(pair->start, p, pair->length, (pair->phys & 0xFFF) | 0x7, dest->PML4);

				Virtual::MapRegion(TemporaryVirtualMapping, p, pair->length, 0x07);

				// copy contents.
				Memory::CopyOverlap((void*) TemporaryVirtualMapping, (void*) pair->start, pair->length * 0x1000);

				Virtual::UnmapRegion(TemporaryVirtualMapping, pair->length);
				reg->phys = p | (pair->phys & 0xFFF);
			}

			dest->regions.push_back(reg);
		}


		UNLOCK(*dest->mtx);
		return dest;
	}

	bool HandlePageFault(uint64_t cr2, uint64_t cr3, uint64_t errorcode)
	{
		(void) cr3;
		(void) errorcode;

		PageMapStructure* pdpt = 0;
		PageMapStructure* pd = 0;
		PageMapStructure* pt = 0;

		// HALT("PF");

		// check if cow.
		uint64_t* value = GetPageTableEntry(cr2, Multitasking::GetCurrentProcess()->VAS.PML4, &pdpt, &pd, &pt);

		// conditions for cow:
		// bit 11 (0x800) for COW set, bit 1 (0x2, R/W bit) not set.
		if(value && (*value & I_CopyOnWrite) && !(*value & I_ReadWrite))
		{
			// allocate a page, copy existing data, then return.
			uint64_t np = Physical::AllocatePage();
			uint64_t old = *value & I_AlignMask;
			uint64_t oldflags = *value & ~I_AlignMask;

			*value = np | (oldflags | I_ReadWrite | I_CopyOnWrite);

			// we need to copy the old bytes to the new page.
			// _old_ should contain the virtual address of the parent
			Virtual::MapAddress(cr2 & I_AlignMask, np, 0x07);
			Virtual::MapAddress(TemporaryVirtualMapping, old, 0x07);

			Log(3, "Copying 0x1000 bytes from phys %x to %x (error: %x)", old, cr2 & I_AlignMask, errorcode);
			Memory::CopyOverlap((void*) (cr2 & I_AlignMask) , (void*) TemporaryVirtualMapping, 0x1000);
			Virtual::UnmapAddress(TemporaryVirtualMapping);

			Log("Successfully allocated new physical page at %x for COW purposes mapped to virtual page %x", np, cr2 & I_AlignMask);
			return true;
		}

		Log("Invalid access (%x:%x)", value, value ? *value : 0);
		return false;
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

		PageMapStructure* _kernelpml4 = (PageMapStructure*) Kernel::GetKernelCR3();
		Virtual::MapAddress(TemporaryVirtualMapping, (uint64_t) _kernelpml4, 0x07);
		PageMapStructure* kernelpml4 = (PageMapStructure*) TemporaryVirtualMapping;

		PageMapStructure* _pdpt = (PageMapStructure*) kernelpml4->Entry[0];

		// bottom 1gb
		Virtual::MapAddress(TemporaryVirtualMapping + 0x1000, (uint64_t) _pdpt, 0x07);
		PageMapStructure* pdpt = (PageMapStructure*) (TemporaryVirtualMapping + 0x1000);
		PageMapStructure* pt = (PageMapStructure*) pdpt->Entry[0];

		// we need to throw this address (aka pointer) into the created things.
		PageMapStructure* PML4 = (PageMapStructure*) Physical::AllocateFromReserved();
		Memory::Set(PML4, 0, 0x1000);

		Virtual::MapAddress((uint64_t) PML4, (uint64_t) PML4, 0x03);
		PML4->Entry[I_RECURSIVE_SLOT] = (uint64_t) PML4 | I_Present | I_ReadWrite;

		// do it here.
		{
			if(!(PML4->Entry[0] & I_Present))
				PML4->Entry[0] = Physical::AllocateFromReserved() | I_Present | I_ReadWrite | I_UserAccess;

			else
				Log(3, "what: %x", PML4->Entry[0]);

			((PageMapStructure*) (PML4->Entry[0]))->Entry[0] = (uint64_t) pt;
			// that just gives us 1gb lower, now we need 512gb upper.

			PML4->Entry[510] = (uint64_t) kernelpml4->Entry[510] | 0x6;
			PML4->Entry[511] = (uint64_t) kernelpml4->Entry[511] | 0x6;
		}

		Virtual::MapAddress((uint64_t) PML4, (uint64_t) PML4, 0x07, PML4);

		// Map 16 MB, includes the kernel.
		Virtual::MapAddress(0x1000, 0x1000, 0x03, PML4);

		// map 0x2000 as supervisor only, since we store sensitive stuff there
		// namely TSS stuff.
		Virtual::MapRegion(0x2000, 0x2000, 0x2, 0x03, PML4);

		// todo: make it more robust, right now it's always 16mb identity mapped.
		for(uint64_t i = 0x4000; i < 0x01000000; i += 0x1000)
		{
			Virtual::MapAddress(i, i, 0x07, PML4);
			// MarkCOW(i, PML4);
		}

		// Map the LFB.
		for(uint64_t i = 0; i < Kernel::GetLFBLengthInPages(); i++)
			Virtual::MapAddress(Kernel::GetFramebufferAddress() + (i * 0x1000), Kernel::GetFramebufferAddress() + (i * 0x1000), 0x07, PML4);


		Virtual::UnmapAddress(TemporaryVirtualMapping);
		Virtual::UnmapAddress(TemporaryVirtualMapping + 0x1000);
		return (uint64_t) PML4;
	}

}
}
}
}























