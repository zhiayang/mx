// Physical.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


// Implements functions to allocate physical memory.


#include <Kernel.hpp>
#include <Memory.hpp>
#include <StandardIO.hpp>
#include <Colours.hpp>

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
	static LinkedList<Pair>* PageList;

	// Define a region of memory in which the VMM gets it's memory from, to create page strucures.
	uint64_t ReservedRegionForVMM = 0;
	uint64_t LengthOfReservedRegion = 0;
	uint64_t OpsSinceLastCoalesce = 0;

	static uint64_t ReservedRegionIndex = 0;
	static uint64_t PMemManagerLocation;
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
		ReservedRegionForVMM = PageAlignUp(Kernel::EndOfKernel);
		LengthOfReservedRegion = 0x01000000 - ReservedRegionForVMM;
		PMemManagerLocation = ReservedRegionForVMM + LengthOfReservedRegion;
	}

	void Initialise()
	{
		assert(!DidInit);
		// See InitialiseFPLs() for an in-depth explanation on this
		// FPL system.
		mtx = new Mutex();
		PageList = new LinkedList<Pair>();

		InitialiseFPLs(Kernel::K_MemoryMap);
		DidInit = true;
	}


	uint64_t AllocatePage(uint64_t size)
	{
		auto mut = AutoMutex(mtx);
		if(!DidInit)
		{
			// return AllocateViaPlacement();
			return AllocateFromReserved();
		}

		OpsSinceLastCoalesce++;
		uint64_t trycount = 0;
		auto len = PageList->Size();

		begin:

		Pair* p = PageList->Front();
		if(!p)
		{
			HALT("Out of physical pages");
		}

		if(p->LengthInPages > size)
		{
			p->LengthInPages -= size;
			uint64_t raddr = p->BaseAddr;

			p->BaseAddr += (size * 0x1000);
			return raddr;
		}
		else if(p->LengthInPages == size)
		{
			auto raddr = p->BaseAddr;
			PageList->RemoveFront();
			delete &p;

			return raddr;
		}
		else
		{
			if(trycount < len)
			{
				PageList->InsertBack(PageList->RemoveFront());
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
		for(uint64_t i = 0; i < PageList->Size(); i++)
		{
			// AddrLengthPair_type* pair = &CurrentFPL->Pairs[i];
			Pair* pair = PageList->RemoveFront();

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

			PageList->InsertBack(pair);

			if(ret)
				return;
		}

		// if we reach here, we haven't found a match.
		Pair* np = new Pair;
		np->BaseAddr = page;
		np->LengthInPages = size;

		PageList->InsertBack(np);
	}


	uint64_t AllocateFromReserved()
	{
		// we need this 'reserved' region especially to store page tables/directories etc.
		// if not, we'll end up with a super-screwed mapping.
		// possibly some overlap as well.

		uint64_t ret = ReservedRegionForVMM + ReservedRegionIndex;
		ReservedRegionIndex += 0x1000;
		Library::Memory::Set((void*) ret, 0x00, 0x1000);
		return ret;
	}


	uint64_t AllocateViaPlacement()
	{
		// This works perfectly, except it cannot free pages once allocted.

		PMemManagerLocation += 0x1000;

		if(!(MemoryMap::IsMemoryValid(PMemManagerLocation)))
		{
			// Check if there are any more 'Available' zones after this one.
			if(!MemoryMap::CheckForMoreAvailableMemory(PMemManagerLocation))
			{
				StandardIO::PrintFormatted("\n%wTried to allocate memory at: %w%x%w\nLast possible memory allocatable is: %x",
					Colours::Yellow, Colours::Cyan, PMemManagerLocation, Colours::Silver, PMemManagerLocation - 0x1000);

				HALT("Out of Memory!");
			}
			else
			{
				PMemManagerLocation = MemoryMap::CheckForMoreAvailableMemory(PMemManagerLocation);
			}
		}
		return PMemManagerLocation;
	}

	uint64_t AllocateDMA(uint64_t size, bool Below4Gb)
	{
		if(!Below4Gb)
		{
			uint64_t a = AllocatePage(size);
			Virtual::MapRegion(a, a, size, 0x3);
			return a;
		}

		auto mut = AutoMutex(mtx);
		OpsSinceLastCoalesce++;
		for(uint64_t i = 0; i < PageList->Size(); i++)
		{
			Pair* pair = PageList->RemoveFront();
			if(pair->BaseAddr + (size * 0x1000) < 0xFFFFFFFF && pair->LengthInPages >= size)
			{
				uint64_t ret = pair->BaseAddr;
				pair->BaseAddr = pair->LengthInPages == size ? 0 : ret + size * 0x1000;
				pair->LengthInPages -= size;

				Virtual::MapRegion(ret, ret, size, 0x3);
				if(pair->BaseAddr == 0)
					delete pair;

				else
					PageList->InsertBack(pair);

				return ret;
			}
			PageList->InsertBack(pair);
		}

		HALT("Could not satisfy DMA memory request");
		return 0;
	}

	void FreeDMA(uint64_t addr, uint64_t size)
	{
		FreePage(addr, size);
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

				PageList->InsertBack(p);
			}
		}

		// The bottom-most FPL would be from 1MB up.
		// However, we must set it to the top of our kernel.

		uint64_t OldBaseAddr = PageList->Front()->BaseAddr;
		Pair* p = PageList->Front();
		p->BaseAddr = 0x00800000;
		p->LengthInPages = p->LengthInPages - ((p->BaseAddr - OldBaseAddr) / 0x1000);
	}



	void CoalesceFPLs()
	{
		if(!DidInit)
			return;

		// check if we even need to coalesce
		if(OpsSinceLastCoalesce < CoalesceThreshold)
			return;

		auto mut = AutoMutex(mtx);
		OpsSinceLastCoalesce = 0;

		// O(n^2) time.
		// essentially, loop in a loop.
		// for each entry in the list, loop through all the other entries to see if there's anything suitable.
		// if so, merge.
		// to make sure we don't screw with the list while in the middle of it, go back to the beginning and loop again.
		// this is a background process anyway so.
		for(uint64_t i = 0; i < PageList->Size(); i++)
		{
			bool delp = false;
			Pair* p = PageList->RemoveFront();
			uint64_t base = p->BaseAddr;
			uint64_t end = p->BaseAddr + (p->LengthInPages * 0x1000);

			for(uint64_t k = 0; k < PageList->Size(); k++)
			{
				Pair* other = PageList->RemoveFront();
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
				PageList->InsertBack(p);
		}
	}
}
}
}
}





















