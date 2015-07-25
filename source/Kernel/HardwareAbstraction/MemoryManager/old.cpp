// // Virtual.cpp
// // Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// // Licensed under the Apache License Version 2.0.


// #include <Kernel.hpp>
// #include <StandardIO.hpp>
// #include <stddef.h>

// #include <rdestl/rdestl.h>

// using namespace Kernel;
// using namespace Kernel::HardwareAbstraction::MemoryManager;
// using namespace Kernel::HardwareAbstraction::MemoryManager::Virtual;

// namespace Kernel {
// namespace HardwareAbstraction {
// namespace MemoryManager {
// namespace Virtual
// {

// 	// this models (exactly) the FPL pair system of the physical allocator.
// 	uint64_t AllocateVirtual(uint64_t size, uint64_t addr, VirtualAddressSpace* v, uint64_t phys)
// 	{
// 		Multitasking::Process* proc = Multitasking::GetCurrentProcess();
// 		assert(proc);

// 		VirtualAddressSpace* vas = v ? v : &proc->VAS;
// 		assert(vas);

// 		if(vas->pairs.size() == 0)
// 		{
// 			Log(3, "Virtual address space exhausted, WTF are you doing?!");
// 			Multitasking::Kill(proc);
// 			UHALT();
// 			return 0;
// 		}

// 		// do a cleanup.
// 		for(auto x : vas->used)
// 		{
// 			if(x.length == 0 || x.phys == 0)
// 				vas->used.remove(x);
// 		}

// 		// look for address first.
// 		if(addr > 0)
// 		{
// 			uint64_t end = addr + (size * 0x1000);
// 			AddressLengthPair* found = 0;
// 			for(auto& pair : vas->pairs)
// 			{
// 				if(pair.start <= addr && pair.start + (pair.length * 0x1000) >= end)
// 				{
// 					found = &pair;
// 					break;
// 				}
// 			}

// 			if(found)
// 			{
// 				// exact match
// 				if(found->start == addr && found->length == size)
// 				{
// 					vas->pairs.remove(*found);
// 				}
// 				else if(found->start == addr)
// 				{
// 					// longer than what we need, create a new pair

// 					AddressLengthPair np = AddressLengthPair(end, found->length - size);
// 					vas->pairs.remove(*found);
// 					vas->pairs.push_back(np);
// 				}
// 				else
// 				{
// 					// make a new pair at the beginning
// 					AddressLengthPair np = AddressLengthPair(found->start, (addr - found->start) / 0x1000);
// 					vas->pairs.push_back(np);

// 					found->start = end;
// 					found->length = found->length - size;
// 				}

// 				vas->used.push_back(ALPTuple(addr, size, phys));
// 				// Log("virt(%x) mapped to phys(%x) [%d] in vas %x (%x)", addr, phys, size, GetCurrentPML4T(), __builtin_return_address(0));
// 				return addr;
// 			}

// 			// else we just continue as normal.
// 			Log(1, "Couldn't satisfy request to allocate virtual address at %x, return address(0) = %x", addr, __builtin_return_address(0));
// 		}













// 		uint64_t ret = 0;
// 		AddressLengthPair& pair = vas->pairs.front();
// 		// assert(pair);

// 		if(pair.length > size)
// 		{
// 			ret = pair.start;
// 			pair.start += (size * 0x1000);
// 			pair.length -= size;

// 			LOCK(vas->mtx);
// 			vas->used.push_back(ALPTuple(ret, size, phys));
// 			UNLOCK(vas->mtx);
// 		}
// 		else if(pair.length == size)
// 		{
// 			ret = pair.start;
// 			// delete vas->pairs.front();
// 			vas->pairs.erase(vas->pairs.begin());

// 			LOCK(vas->mtx);
// 			vas->used.push_back(ALPTuple(ret, size, phys));
// 			UNLOCK(vas->mtx);
// 		}
// 		else
// 		{
// 			// we need to use the next pair.
// 			AddressLengthPair* found = 0;
// 			for(AddressLengthPair& p : vas->pairs)
// 			{
// 				if(p.length >= size)
// 				{
// 					found = &p;
// 					break;
// 				}
// 			}

// 			if(!found)
// 			{
// 				Log(3, "Virtual address space exhausted, WTF are you doing?! HUH?");
// 				Multitasking::Kill(proc);
// 				UHALT();
// 			}
// 			else
// 			{
// 				if(found->length > size)
// 				{
// 					ret = found->start;
// 					found->length -= size;
// 					found->start += (size * 0x1000);

// 					LOCK(vas->mtx);
// 					vas->used.push_back(ALPTuple(ret, size, phys));
// 					UNLOCK(vas->mtx);
// 				}
// 				else
// 				{
// 					ret = found->start;

// 					LOCK(vas->mtx);
// 					vas->used.push_back(ALPTuple(ret, size, phys));
// 					UNLOCK(vas->mtx);
// 				}
// 			}

// 			if(ret == 0)
// 			{
// 				HALT("something went wrong");
// 				return 0;
// 			}
// 		}

// 		// Log("\tvirt(%x) mapped to phys(%x) [%d] in vas %x (%x)", ret, phys, size, GetCurrentPML4T(), __builtin_return_address(0));
// 		return ret;
// 	}

// 	void FreeVirtual(uint64_t page, uint64_t size, VirtualAddressSpace* v)
// 	{
// 		Multitasking::Process* proc = Multitasking::GetCurrentProcess();
// 		assert(proc);

// 		VirtualAddressSpace* vas = v ? v : &proc->VAS;
// 		assert(vas);

// 		uint64_t end = page + (size * 0x1000);

// 		bool insertNew = true;
// 		for(AddressLengthPair& pair : vas->pairs)
// 		{
// 			// 3 basic conditions
// 			// 1. we find a match below a pair's baseaddr
// 			// 2. we find a match above a pair's baseaddr
// 			// 3. we don't find a match

// 			if(end == pair.start)
// 			{
// 				pair.start = page;
// 				pair.length += size;
// 				insertNew = false;
// 			}
// 			else if(pair.start + (pair.length * 0x1000) == page)
// 			{
// 				pair.length += (size * 0x1000);
// 				insertNew = false;
// 			}
// 		}


// 		if(insertNew)
// 		{
// 			vas->pairs.push_back(AddressLengthPair(page, size));
// 		}

// 		// Log("freeing %x, %d (%x, %x)", page, size, __builtin_return_address(0), __builtin_return_address(1));
// 		// for(size_t i = 0; i < vas->used.size(); i++)

// 		for(auto it = vas->used.begin(); it != vas->used.end(); it++)
// 		{
// 			// ALPTuple& pair = vas->used[i];
// 			// if(!pair) continue;

// 			ALPTuple& pair = *it;

// 			if((pair.start == page && pair.length == size) || (pair.start == 0 && pair.length == 0 && pair.phys == 0))
// 			{
// 				// Log("freeing (%x, %x, %d) (%x)", pair->start, pair->phys, pair->length, __builtin_return_address(0));
// 				// vas->used.erase(vas->used.begin() + i);
// 				// i = 0;

// 				vas->used.erase(it);
// 				it = vas->used.begin();
// 				continue;
// 			}
// 		}
// 	}

// 	void ForceInsertALPTuple(uint64_t addr, size_t sizeInPages, uint64_t phys, VirtualAddressSpace* v)
// 	{
// 		VirtualAddressSpace* vas = v ? v : &Multitasking::GetCurrentProcess()->VAS;
// 		assert(vas);

// 		LOCK(vas->mtx);
// 		vas->used.push_back(ALPTuple(addr, sizeInPages, phys));
// 		UNLOCK(vas->mtx);
// 	}

// 	uint64_t GetVirtualPhysical(uint64_t virt, VirtualAddressSpace* v)
// 	{
// 		VirtualAddressSpace* vas = v ? v : &Multitasking::GetCurrentProcess()->VAS;
// 		assert(vas);

// 		uint64_t ret = 0;
// 		for(ALPTuple& pair : vas->used)
// 		{
// 			uint64_t end = pair.start + pair.length * 0x1000;
// 			// Log("s: %x, p: %x, v: %x, l: %d -- %x", pair->start, pair->phys, virt, pair->length, vas->PML4);
// 			if(virt >= pair.start && virt <= end)
// 			{
// 				ret = pair.phys + (virt - pair.start);
// 			}
// 		}

// 		return ret;
// 	}

// 	uint64_t AllocatePage(uint64_t size, uint64_t addr, uint64_t flags)
// 	{
// 		uint64_t phys = Physical::AllocatePage(size);
// 		uint64_t virt = AllocateVirtual(size, addr, 0, phys);
// 		// Log("allocated v(%x) to p(%x), %d pages, %x", virt, phys, size, phys + (size * 0x1000));

// 		MapRegion(virt, phys, size, flags);

// 		return virt;
// 	}

// 	void FreePage(uint64_t addr, uint64_t size)
// 	{
// 		// freepage should only be called with a return from allocatepage
// 		// we got to do some work here
// 		// look through the vas pairs, looking for one that fits our description.
// 		ALPTuple* pair = 0;

// 		Multitasking::Process* proc = Multitasking::GetCurrentProcess();
// 		assert(proc);

// 		VirtualAddressSpace* vas = &proc->VAS;
// 		assert(vas);

// 		for(ALPTuple& p : vas->used)
// 		{
// 			// if(!p)
// 			// 	continue;

// 			if(p.start == addr)
// 			{
// 				if(p.length == size)
// 				{
// 					pair = &p;
// 					break;
// 				}
// 			}
// 		}

// 		if(!pair) return;
// 		assert(pair);

// 		uint64_t phys = pair->phys;
// 		if(phys == 0)
// 		{
// 			Log(1, "Virtual page had no physical page associated; falling back on manual resolution");
// 			return;

// 			phys = GetMapping(addr, vas->PML4);
// 		}
// 		assert(phys > 0);

// 		Physical::FreePage(phys, size);
// 		Virtual::FreeVirtual(addr, size);

// 		UnmapRegion(addr, size);
// 	}






// 	VirtualAddressSpace* SetupVAS(VirtualAddressSpace* vas)
// 	{
// 		// Max 48-bit virtual address space (current implementations)
// 		vas->pairs.push_back(AddressLengthPair(0x01000000, 0xFF000));
// 		vas->pairs.push_back(AddressLengthPair(0xFFFFF00000000000, 0x100000));

// 		vas->mtx = new Mutex();
// 		return vas;
// 	}

// 	VirtualAddressSpace* CopyVAS(VirtualAddressSpace* src, VirtualAddressSpace* dest)
// 	{
// 		for(auto p : src->pairs)
// 			dest->pairs.push_back(p);

// 		for(auto _pair : src->used)
// 		{
// 			// todo: workaround... investigate
// 			// if(!_pair)
// 			// 	continue;

// 			ALPTuple pair = ALPTuple(_pair);

// 			// dest->used.push_back(pair);

// 			uint64_t p = Physical::AllocatePage(pair.length);

// 			Virtual::MapRegion(pair.start, /*pair->phys*/ p, pair.length, 0x07, dest->PML4);
// 			Virtual::MapRegion(TemporaryVirtualMapping, p, pair.length, 0x07);
// 			Memory::CopyOverlap((void*) TemporaryVirtualMapping, (void*) pair.start, pair.length * 0x1000);


// 			Virtual::UnmapRegion(TemporaryVirtualMapping, pair.length);
// 			pair.phys = p;

// 			dest->used.push_back(pair);



// 			#if 0
// 				Virtual::MapRegion(pair->start, pair->phys, pair->length, 0x07, dest->PML4);
// 				for(uint64_t i = 0; i < pair->length; i++)
// 				{
// 					Log("Marking %x as COW", pair->start + (i * 0x1000));
// 					Virtual::MarkCOW(pair->start + (i * 0x1000), dest);
// 				}
// 			#endif
// 		}

// 		return dest;
// 	}

// 	void DestroyVAS(VirtualAddressSpace* vas)
// 	{
// 		// free every phys page in vas->used
// 		// delete pair objects from both lists
// 		// delete both lists

// 		assert(vas);
// 		// assert(vas->used);
// 		// assert(vas->pairs);

// 		for(ALPTuple& pair : vas->used)
// 		{
// 			// todo: workaround... investigate
// 			// if(!pair)
// 			// 	continue;

// 			if(pair.phys > 0)
// 				Physical::FreePage(pair.phys, pair.length);

// 			// delete pair;
// 		}


// 		// for(AddressLengthPair* pair : vas->pairs)
// 		// 	delete pair;
// 	}






















// 	bool HandlePageFault(uint64_t cr2, uint64_t cr3, uint64_t errorcode)
// 	{
// 		(void) cr3;
// 		(void) errorcode;

// 		PageMapStructure* pdpt = 0;
// 		PageMapStructure* pd = 0;
// 		PageMapStructure* pt = 0;

// 		// HALT("PF");

// 		// check if cow.
// 		uint64_t* value = GetPageTableEntry(cr2, Multitasking::GetCurrentProcess()->VAS.PML4, &pdpt, &pd, &pt);

// 		// conditions for cow:
// 		// bit 11 (0x800) for COW set, bit 1 (0x2, R/W bit) not set.
// 		if(value && (*value & I_CopyOnWrite) && !(*value & I_ReadWrite))
// 		{
// 			// allocate a page, copy existing data, then return.
// 			uint64_t np = Physical::AllocatePage();
// 			uint64_t old = *value & I_AlignMask;
// 			uint64_t oldflags = *value & ~I_AlignMask;

// 			*value = np | (oldflags | I_ReadWrite | I_CopyOnWrite);

// 			// we need to copy the old bytes to the new page.
// 			// _old_ should contain the virtual address of the parent
// 			Virtual::MapAddress(cr2 & I_AlignMask, np, 0x07);
// 			Virtual::MapAddress(TemporaryVirtualMapping, old, 0x07);

// 			Log(3, "Copying 0x1000 bytes from phys %x to %x (error: %x)", old, cr2 & I_AlignMask, errorcode);
// 			Memory::CopyOverlap((void*) (cr2 & I_AlignMask) , (void*) TemporaryVirtualMapping, 0x1000);
// 			Virtual::UnmapAddress(TemporaryVirtualMapping);

// 			Log("Successfully allocated new physical page at %x for COW purposes mapped to virtual page %x", np, cr2 & I_AlignMask);
// 			return true;
// 		}

// 		Log("Invalid access (%x:%x)", value, value ? *value : 0);
// 		return false;
// 	}


// 	void CopyBetweenAddressSpaces(uint64_t fromAddr, uint64_t toAddr, size_t bytes, VirtualAddressSpace* from, VirtualAddressSpace* to)
// 	{
// 		(void) fromAddr;
// 		(void) toAddr;
// 		(void) bytes;
// 		(void) from;
// 		(void) to;

// 		// this can only be done in the kernel
// 		// assert(Multitasking::GetProcess(0) == Multitasking::GetCurrentProcess());

// 		// size_t numPages				= (bytes + 0xFFF) / 0x1000;

// 		// uint64_t toAligned			= toAddr & I_AlignMask;
// 		// uint64_t toBeginOffset		= toAddr - toAligned;
// 		// uint64_t toPhys				= GetVirtualPhysical(toAligned, to);

// 		// uint64_t fromAligned		= fromAddr & I_AlignMask;
// 		// uint64_t fromBeginOffset	= fromAddr - fromAligned;
// 		// uint64_t fromPhys			= GetVirtualPhysical(fromAligned, from);

// 		// if(toPhys == 0)		Log(1, "Could not fetch physical address for %x in vas %x (to)", toAligned, to->PML4);
// 		// if(fromPhys == 0)	Log(1, "Could not fetch physical address for %x in vas %x (from)", fromAligned, from->PML4);

// 		// assert(toPhys > 0);
// 		// assert(fromPhys > 0);

// 		// // we need to map two sets, since fromAddr and toAddr might (and probably will)
// 		// // have different physical backing pages. Therefore, we map both the fromPhys and toPhys to our own
// 		// // address space, then do the memcpy.
// 		// uint64_t fromPhysMapped = AllocateVirtual(numPages, 0, from, fromPhys);
// 		// Virtual::MapRegion(fromPhysMapped, fromPhys, numPages, 0x3);

// 		// uint64_t toPhysMapped = AllocateVirtual(numPages, 0, to, toPhys);
// 		// Virtual::MapRegion(toPhysMapped, toPhys, numPages, 0x3);

// 		// // then copy it.
// 		// Log("Preparing to copy (%d bytes) from (%x - %x + %x) to (%x - %x + %x)", bytes, fromPhysMapped, fromPhys, fromBeginOffset, toPhysMapped, toPhys, toBeginOffset);

// 		// Memory::Copy((void*) (toPhysMapped + toBeginOffset), (void*) (fromPhysMapped + fromBeginOffset), bytes);

// 		// FreeVirtual(fromPhysMapped, numPages);
// 		// FreeVirtual(toPhysMapped, numPages);

// 		// Virtual::UnmapRegion(fromPhysMapped, numPages);
// 		// Virtual::UnmapRegion(toPhysMapped, numPages);
// 	}

// 	void CopyFromKernel(uint64_t fromAddr, uint64_t toAddr, size_t bytes, VirtualAddressSpace* to)
// 	{
// 		// this can only be done in the kernel
// 		assert(Multitasking::GetProcess(0) == Multitasking::GetCurrentProcess());

// 		VirtualAddressSpace* from = &Multitasking::GetProcess(0)->VAS;


// 		size_t numPages			= (bytes + 0xFFF) / 0x1000;
// 		uint64_t toAligned		= toAddr & I_AlignMask;
// 		uint64_t beginOffset	= toAddr - toAligned;
// 		uint64_t toPhys			= GetVirtualPhysical(toAligned, to);

// 		if(toPhys == 0)
// 		{
// 			Log(1, "Could not fetch physical address for %x in vas %x", toAligned, to->PML4);
// 			toPhys = GetMapping(toAligned, to->PML4) & I_AlignMask;
// 		}

// 		assert(toPhys > 0);

// 		// allocate some temporary virt pages in the local address space
// 		// then map the physical pages behind toAddr to our local space
// 		uint64_t toVirtTemp		= AllocateVirtual(numPages, 0, from, toPhys);
// 		Virtual::MapRegion(toVirtTemp, toPhys, numPages, 0x3);

// 		// then copy it.
// 		// Log("Copied %d bytes from %x to %x (%x, %x, %x)", bytes, fromAddr, toAddr, beginOffset, toPhys, to->PML4);
// 		Memory::Copy((void*) (toVirtTemp + beginOffset), (void*) fromAddr, bytes);

// 		FreeVirtual(toVirtTemp, numPages);
// 		Virtual::UnmapRegion(toVirtTemp, numPages);
// 	}

// 	void CopyToKernel(uint64_t fromAddr, uint64_t toAddr, size_t bytes, VirtualAddressSpace* from)
// 	{
// 		// this can only be done in the kernel
// 		assert(Multitasking::GetProcess(0) == Multitasking::GetCurrentProcess());

// 		VirtualAddressSpace* to = &Multitasking::GetProcess(0)->VAS;
// 		size_t numPages			= (bytes + 0xFFF) / 0x1000;

// 		uint64_t fromAligned	= fromAddr & I_AlignMask;
// 		uint64_t beginOffset	= fromAddr - fromAligned;
// 		uint64_t fromPhys		= GetVirtualPhysical(fromAligned, from);

// 		// special exception for kernel heap
// 		if(fromPhys == 0 && fromAddr >= KernelHeapAddress)
// 			fromPhys = GetVirtualPhysical(fromAligned, to);

// 		if(fromPhys == 0) Log(1, "Could not fetch physical address for %x in vas %x", fromAligned, from->PML4);
// 		assert(fromPhys > 0);

// 		// allocate some temporary virt pages in the local address space
// 		// then map the physical pages behind toAddr to our local space
// 		uint64_t fromPhysMapped = AllocateVirtual(numPages, 0, to, fromPhys);
// 		Virtual::MapRegion(fromPhysMapped, fromPhys, numPages, 0x3);

// 		// then copy it.
// 		Memory::Copy((void*) toAddr, (void*) (fromPhysMapped + beginOffset), bytes);

// 		FreeVirtual(fromPhysMapped, numPages);
// 		Virtual::UnmapRegion(fromPhysMapped, numPages);
// 	}

























// 	uint64_t CreateVAS()
// 	{
// 		/*
// 			256 TB
// 			512 GB
// 			1 GB
// 			2 MB

// 			bottom 1gb is kernel owned.
// 			top 1024gb is also kernel owned.
// 		*/

// 		PageMapStructure* _kernelpml4 = (PageMapStructure*) Kernel::GetKernelCR3();
// 		Virtual::MapAddress(TemporaryVirtualMapping, (uint64_t) _kernelpml4, 0x07);
// 		PageMapStructure* kernelpml4 = (PageMapStructure*) TemporaryVirtualMapping;

// 		PageMapStructure* _pdpt = (PageMapStructure*) kernelpml4->Entry[0];

// 		// bottom 1gb
// 		Virtual::MapAddress(TemporaryVirtualMapping + 0x1000, (uint64_t) _pdpt, 0x07);
// 		PageMapStructure* pdpt = (PageMapStructure*) (TemporaryVirtualMapping + 0x1000);
// 		PageMapStructure* pt = (PageMapStructure*) pdpt->Entry[0];

// 		// we need to throw this address (aka pointer) into the created things.
// 		PageMapStructure* PML4 = (PageMapStructure*) Physical::AllocateFromReserved();
// 		Memory::Set(PML4, 0, 0x1000);

// 		Virtual::MapAddress((uint64_t) PML4, (uint64_t) PML4, 0x03);
// 		PML4->Entry[I_RECURSIVE_SLOT] = (uint64_t) PML4 | I_Present | I_ReadWrite;

// 		// do it here.
// 		{
// 			if(!(PML4->Entry[0] & I_Present))
// 				PML4->Entry[0] = Physical::AllocateFromReserved() | I_Present | I_ReadWrite | I_UserAccess;

// 			else
// 				Log(3, "what: %x", PML4->Entry[0]);

// 			((PageMapStructure*) (PML4->Entry[0]))->Entry[0] = (uint64_t) pt;
// 			// that just gives us 1gb lower, now we need 512gb upper.

// 			PML4->Entry[510] = (uint64_t) kernelpml4->Entry[510] | 0x6;
// 			PML4->Entry[511] = (uint64_t) kernelpml4->Entry[511] | 0x6;
// 		}

// 		Virtual::MapAddress((uint64_t) PML4, (uint64_t) PML4, 0x07, PML4);

// 		// Map 16 MB, includes the kernel.
// 		Virtual::MapAddress(0x1000, 0x1000, 0x03, PML4);

// 		// map 0x2000 as supervisor only, since we store sensitive stuff there
// 		// namely TSS stuff.
// 		Virtual::MapRegion(0x2000, 0x2000, 0x2, 0x03, PML4);

// 		// todo: make it more robust, right now it's always 16mb identity mapped.
// 		for(uint64_t i = 0x4000; i < 0x01000000; i += 0x1000)
// 		{
// 			Virtual::MapAddress(i, i, 0x07, PML4);
// 			// MarkCOW(i, PML4);
// 		}

// 		// Map the LFB.
// 		for(uint64_t i = 0; i < Kernel::GetLFBLengthInPages(); i++)
// 			Virtual::MapAddress(Kernel::GetFramebufferAddress() + (i * 0x1000), Kernel::GetFramebufferAddress() + (i * 0x1000), 0x07, PML4);


// 		Virtual::UnmapAddress(TemporaryVirtualMapping);
// 		Virtual::UnmapAddress(TemporaryVirtualMapping + 0x1000);
// 		return (uint64_t) PML4;
// 	}
// }
// }
// }
// }























