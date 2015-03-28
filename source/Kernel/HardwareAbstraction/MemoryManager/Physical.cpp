// Physical.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


// Implements functions to allocate physical memory.


#include <Kernel.hpp>
#include <Memory.hpp>
#include <StandardIO.hpp>
#include <rdestl/list.h>

#define CoalesceThreshold	256

using namespace Kernel;
using namespace Kernel::HardwareAbstraction::MemoryManager::Physical;
using namespace Kernel::HardwareAbstraction::MemoryManager;

using namespace Library;

namespace Kernel {
namespace HardwareAbstraction {
namespace MemoryManager {
namespace Physical
{
	struct Pair
	{
		uint64_t BaseAddr;
		uint64_t LengthInPages;
	};


	extern Kernel::HardwareAbstraction::MemoryManager::MemoryMap::MemoryMap_type* K_MemoryMap;

	static bool DidInit = false;
	static rde::vector<Pair*>* PageList;

	// Define a region of memory in which the VMM gets it's memory from, to create page strucures.
	uint64_t ReservedRegionForVMM = 0;
	uint64_t LengthOfReservedRegion = 0;
	uint64_t OpsSinceLastCoalesce = 0;

	static uint64_t ReservedRegionIndex = 0;
	// End legacy crud

	static Mutex* mtx;


	uint64_t PageAlignDown(uint64_t x)
	{
		return x & (uint64_t)~(0xFFF);
	}
	uint64_t PageAlignUp(uint64_t x)
	{
		return PageAlignDown(x + 0xFFF);
	}



	void Bootstrap()
	{
		ReservedRegionForVMM = 0x00800000;
		LengthOfReservedRegion = 0x01000000 - ReservedRegionForVMM;
		// PMemManagerLocation = ReservedRegionForVMM + LengthOfReservedRegion;
	}

	void Initialise()
	{
		assert(!DidInit);
		// See InitialiseFPLs() for an in-depth explanation on this
		// FPL system.
		mtx = new Mutex();
		// PageList = new LinkedList<Pair>();
		PageList = new rde::vector<Pair*>();

		InitialiseFPLs(Kernel::K_MemoryMap);
		DidInit = true;
	}


	uint64_t AllocatePage(uint64_t size, bool Below4Gb)
	{
		if(!DidInit)
			return AllocateFromReserved(size);

		auto mut = AutoMutex(mtx);
		OpsSinceLastCoalesce++;
		size_t trycount = 0;
		auto len = PageList->size();

		begin:

		if(PageList->size() == 0)
		{
			HALT("Out of physical pages");
		}

		Pair* p = PageList->front();
		if(Below4Gb && p->BaseAddr >= 0xFFFFFFFF)
		{
			trycount++;
			PageList->erase(PageList->begin());
			PageList->push_back(p);
			goto begin;
		}
		else if(p->LengthInPages > size)
		{
			p->LengthInPages -= size;
			uint64_t raddr = p->BaseAddr;

			p->BaseAddr += (size * 0x1000);
			return raddr;
		}
		else if(p->LengthInPages == size)
		{
			auto raddr = p->BaseAddr;
			PageList->erase(PageList->begin());

			delete p;
			return raddr;
		}
		else
		{
			if(trycount < len)
			{
				auto fr = PageList->front();
				PageList->erase(PageList->begin());
				PageList->push_back(fr);

				trycount++;
				goto begin;	// dirty
			}
			else
			{
				HALT("Out of physical pages");
				return 0;
			}
		}
	}

	void FreePage(uint64_t page, uint64_t size)
	{
		auto mut = AutoMutex(mtx);
		OpsSinceLastCoalesce++;

		uint64_t end = page + (size * 0x1000);

		bool ret = false;
		for(size_t i = 0; i < PageList->size(); i++)
		{
			Pair* pair = PageList->front();
			PageList->erase(PageList->begin());

			// 3 basic conditions
			// 1. we find a match below a pair's baseaddr
			// 2. we find a match above a pair's baseaddr
			// 3. we don't find a match

			if(end == pair->BaseAddr)
			{
				pair->BaseAddr = page;
				pair->LengthInPages += size;
				ret = true;
			}
			else if(pair->BaseAddr + (pair->LengthInPages * 0x1000) == page)
			{
				pair->LengthInPages += (size * 0x1000);
				ret = true;
			}

			PageList->push_back(pair);

			if(ret)
				return;
		}

		// if we reach here, we haven't found a match.
		Pair* np = new Pair;
		np->BaseAddr = page;
		np->LengthInPages = size;

		PageList->push_back(np);
	}


	uint64_t AllocateFromReserved(uint64_t size)
	{
		// we need this 'reserved' region especially to store page tables/directories etc.
		// if not, we'll end up with a super-screwed mapping.
		// possibly some overlap as well.

		uint64_t ret = ReservedRegionForVMM + ReservedRegionIndex;
		ReservedRegionIndex += (0x1000 * size);
		Memory::Set((void*) ret, 0x00, (size * 0x1000));
		return ret;
	}



	DMAAddr AllocateDMA(uint64_t size, bool Below4Gb)
	{
		DMAAddr ret;
		ret.phys = AllocatePage(size, Below4Gb);
		ret.virt = Virtual::AllocateVirtual(size, 0, 0, ret.phys);

		Virtual::MapRegion(ret.virt, ret.phys, size, 0x3);
		return ret;
	}

	void FreeDMA(DMAAddr addr, uint64_t size)
	{
		FreePage(addr.phys, size);
		Virtual::FreeVirtual(addr.virt);

		Virtual::UnmapRegion(addr.virt, size);
	}













	void InitialiseFPLs(MemoryMap::MemoryMap_type* MemoryMap)
	{
		// Concept:

		// Pseudo-stack based structure:
		// FIFO;

		// Free frames are stored in pairs:
		// [BaseAddr, Length (in 4K)]
		// 16 bytes per segment of memory.

		// Basically;
		// On Alloc(), check the size of bottom pointer (FIFO, remember)
		// If size > 1, decrement size. Store current BaseAddr, then increment it by 0x1000.
		// Return stored BaseAddr.

		// If size == 1, the segment is about to be emptied. Store current BaseAddr.
		// Change stack bottom pointer to next pair. If none exists, set flag; HALT() on next
		// Alloc().


		// On Free(), loop through each pair entry in the FPL strucutre.
		// If Addr == CurrentEntry->BaseAddr + (Size * 0x1000),
		// It's an entry at the end of an existing pair. Increment size, then return.

		// If Addr == CurrentEntry->BaseAddr - 0x1000,
		// It's an entry at the beginning of an existing pair.
		// Decrement the current BaseAddr by 0x1000, increment the Length by 1 and return.

		// If Addr < Lowest BaseAddr, make a new (BaseAddr, Length) pair, set the BaseAddr to Addr and Length to 1.
		// Set the stack bottom pointer, and return.

		// Else... Addr must either be at the top of the stack, (past last BaseAddr + Length pair)... or,
		// Since Alloc() and Free()'s don't need to be consecutive (ie. one Alloc, one Free, one Alloc, then one Free etc), we will end up with
		// gaps in the segments (similar to disk fragmentation).

		// As such, we need to periodically run a function to clean the FPLs. Alloc() is done in O(1) [constant time], while Free() is done in
		// O(m), where (m) is the number of segments existing (B,L) pairs. This opposed to O(n), which would be relative to the amount of RAM, and would
		// possibly be hideously slow.

		// Basically, what this method does is add a (B,L) pair to the stack for every 'Available' type memory in the E820 map.

		for(uint16_t i = 0; i < MemoryMap->NumberOfEntries; i++)
		{
			if(MemoryMap->Entries[i].MemoryType == G_MemoryTypeAvailable && MemoryMap->Entries[i].BaseAddress >= 0x00100000)
			{
				// It's available memory, add a (B,L) pair.
				// CurrentFPL->Pairs[NumberOfFPLPairs].BaseAddr = PageAlignUp(MemoryMap->Entries[i].BaseAddress);
				// CurrentFPL->Pairs[NumberOfFPLPairs].LengthInPages = (PageAlignDown(MemoryMap->Entries[i].Length) / 0x1000) - 1;

				Pair* p = new Pair();
				p->BaseAddr = PageAlignUp(MemoryMap->Entries[i].BaseAddress);
				p->LengthInPages = (PageAlignDown(MemoryMap->Entries[i].Length) / 0x1000) - 1;

				// PageList->push_back(p);
				PageList->push_back(p);
				Log("Physical memory pair: (%x, %d -- %x)", p->BaseAddr, p->LengthInPages, p->BaseAddr + (p->LengthInPages * 0x1000));
			}
		}

		// The bottom-most FPL would be from 1MB up.
		// However, we must set it to the top of our kernel.

		// uint64_t OldBaseAddr = PageList->front()->BaseAddr;
		// Pair* p = PageList->front();

		auto OldBaseAddr = PageList->front()->BaseAddr;
		auto p = PageList->front();

		p->BaseAddr = 0x01000000;
		p->LengthInPages = p->LengthInPages - ((p->BaseAddr - OldBaseAddr) / 0x1000);
	}



	void CoalesceFPLs()
	{
		if(!DidInit)
			return;

		// check if we even need to coalesce
		if(OpsSinceLastCoalesce < CoalesceThreshold)
			return;

		Log("Coalesced FPLs");
		auto mut = AutoMutex(mtx);
		OpsSinceLastCoalesce = 0;

		// O(n^2) time.
		// essentially, loop in a loop.
		// for each entry in the list, loop through all the other entries to see if there's anything suitable.
		// if so, merge.
		// to make sure we don't screw with the list while in the middle of it, go back to the beginning and loop again.
		// this is a background process anyway so.
		for(size_t i = 0; i < PageList->size(); i++)
		{
			bool delp = false;
			// Pair* p = PageList->pop_front();
			auto p = PageList->front();
			PageList->erase(PageList->begin());

			uint64_t base = p->BaseAddr;
			uint64_t end = p->BaseAddr + (p->LengthInPages * 0x1000);

			for(size_t k = 0; k < PageList->size(); k++)
			{
				// Pair* other = PageList->pop_front();
				auto other = PageList->front();
				PageList->erase(PageList->begin());

				if(other->BaseAddr == end)
				{
					p->LengthInPages += other->LengthInPages;
					delete other;
					break;
				}
				else if(other->BaseAddr + (other->LengthInPages * 0x1000) == base)
				{
					other->LengthInPages += p->LengthInPages;
					delp = true;
					break;
				}
			}

			if(delp)
			{
				delete p;
			}
			else
				PageList->push_back(p);
		}
	}
}
}
}
}





















