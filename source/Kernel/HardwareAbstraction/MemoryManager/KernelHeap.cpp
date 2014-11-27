// KernelHeap.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


// Heap things.

#include <Kernel.hpp>
#include <Memory.hpp>
#include <StandardIO.hpp>
#include <String.hpp>

using namespace Kernel;
using namespace Kernel::HardwareAbstraction::MemoryManager;

using namespace Library;

#define MaximumHeapSizeInPages		0xFFFFFFFF
#define Alignment					32
#define MetadataSize				32
#define Warn						0
#define MapFlags					0x7

static uint64_t NumberOfChunksInHeap;
static uint64_t SizeOfHeapInPages;
static uint64_t SizeOfMetadataInPages;
static uint64_t FirstFreeIndex;

namespace Kernel {
namespace HardwareAbstraction {
namespace MemoryManager {
namespace KernelHeap
{
	bool DidInitialise = false;

	struct Chunk_type
	{
		uint64_t Offset;
		uint64_t Size;
		// uint64_t uuid;
		// uint64_t magic;

	} __attribute__ ((packed));


	uint64_t GetSize(Chunk_type* c)
	{
		uint64_t f = c->Size & (uint64_t)(~0x1);
		return f;
	}

	uint64_t addr(Chunk_type* c)
	{
		return (uint64_t) c;
	}

	bool IsFree(Chunk_type* c)
	{
		return c->Size & 0x1;
	}

	void ExpandMetadata()
	{
		uint64_t d = KernelHeapMetadata + (SizeOfMetadataInPages * 0x1000);
		uint64_t p = Physical::AllocatePage();

		Virtual::MapAddress(d, p, MapFlags, (Virtual::PageMapStructure*) GetKernelCR3());
		SizeOfMetadataInPages++;
	}

	uint64_t CreateNewChunk(uint64_t offset, uint64_t size)
	{
		uint64_t s = KernelHeapMetadata + MetadataSize;
		bool found = false;

		for(uint64_t m = 0; m < NumberOfChunksInHeap; m++)
		{
			Chunk_type* ct = (Chunk_type*) s;
			if((ct->Offset + GetSize(ct) == offset && IsFree(ct)) || (ct->Offset == 0 && ct->Size == 0))
			{
				found = true;
				break;
			}

			s += sizeof(Chunk_type);
		}

		if(found)
		{
			Chunk_type* c1 = (Chunk_type*) s;

			if(c1->Offset == 0 && c1->Size == 0)
			{
				// this is one of those derelict chunks, repurpose it.
				c1->Offset = offset;
				c1->Size = size & (uint64_t)(~0x1);
				c1->Size |= 0x1;
			}
			else
			{
				c1->Size += size;
				c1->Size |= 0x1;
			}

			return (uint64_t) c1;
		}




		uint64_t p = MetadataSize + ((NumberOfChunksInHeap - 1) * sizeof(Chunk_type));

		if(p + sizeof(Chunk_type) == SizeOfMetadataInPages * 0x1000)
		{
			// if this is the last one before the page
			// create a new one.
			ExpandMetadata();
		}

		// create another one.
		Chunk_type* c1 = (Chunk_type*)(KernelHeapMetadata + p + sizeof(Chunk_type));
		NumberOfChunksInHeap++;

		c1->Offset = offset;
		c1->Size = size | 0x1;

		return (uint64_t) c1;
	}

	Chunk_type* GetChunkAtIndex(uint64_t i)
	{
		uint64_t p = KernelHeapMetadata + MetadataSize + (i * sizeof(Chunk_type));
		return (Chunk_type*) p;
	}

	uint64_t RoundSize(uint64_t s)
	{
		uint64_t remainder = s % Alignment;

		if(remainder == 0)
			return s;

		return s + Alignment - remainder;
	}













	void Initialise()
	{
		// the first page will contain bits of metadata.
		// including: number of chunks.
		// size of heap in pages.

		// let's start.
		// allocate one page for the heap.
		// will be mapped for now.
		uint64_t h = Physical::AllocatePage();
		uint64_t d = Physical::AllocatePage();

		// also map it.
		Virtual::MapAddress(KernelHeapMetadata, h, MapFlags);
		Virtual::MapAddress(KernelHeapAddress, d, MapFlags);



		NumberOfChunksInHeap = 1;
		SizeOfHeapInPages = 1;
		SizeOfMetadataInPages = 1;
		FirstFreeIndex = 0;


		Chunk_type* fc = (Chunk_type*)(KernelHeapMetadata + MetadataSize);
		fc->Offset = 0;

		// bit zero stores free/not: 1 is free, 0 is not. that means we can only do 2-byte granularity.
		fc->Size = 0x1000 | 0x1;

		DidInitialise = true;
	}

	uint64_t FindFirstFreeSlot(uint64_t Size)
	{
		// free chunks are more often than not found at the back,
		// so fuck convention and scan from the back.

		{
			uint64_t p = KernelHeapMetadata + MetadataSize + (FirstFreeIndex * sizeof(Chunk_type));

			for(uint64_t k = 0; k < NumberOfChunksInHeap - FirstFreeIndex; k++)
			{
				Chunk_type* c = (Chunk_type*) p;
				if(GetSize(c) > Size && IsFree(c))
					return p;

				else
					p += sizeof(Chunk_type);
			}

			// because the minimum offset (from the meta address) is 8, zero will do.
			return 0;
		}
	}


	void SplitChunk(Chunk_type* Header, uint64_t Size)
	{
		// round up to nearest two.
		uint64_t ns = ((Size / 2) + 1) * 2;

		assert(GetSize(Header) >= ns);

		// create a new chunk.
		// because of the inherently unordered nature of the list (ie. we can't insert arbitrarily into the middle of the
		// list, for it is a simple array), we cannot make assumptions (aka assert() s) about the Offset and Size fields.
		// because this function will definitely disorder the list, because CreateNewChunk() can only put
		// new chunks at the end of the list.

		uint64_t nsonc = GetSize(Header) - ns;
		CreateNewChunk(Header->Offset + ns, nsonc);

		Header->Size = ns | 0x1;
	}


	void ExpandHeap()
	{
		if(SizeOfHeapInPages == MaximumHeapSizeInPages)
		{
			asm volatile("cli");
			HALT("Heap out of memory! (Tried to expand beyond max amount of pages)");
		}


		uint64_t p = KernelHeapMetadata + MetadataSize;
		bool found = false;
		for(uint64_t m = 0; m < NumberOfChunksInHeap; m++)
		{
			Chunk_type* ct = (Chunk_type*) p;
			if(ct->Offset + GetSize(ct) == SizeOfHeapInPages * 0x1000 && IsFree(ct))
			{
				found = true;
				break;
			}

			p += sizeof(Chunk_type);
		}


		// check if the chunk before the new one is free.
		if(found)
		{
			Chunk_type* c1 = (Chunk_type*) p;

			// merge with that instead.
			c1->Size += 0x1000;
			c1->Size |= 0x1;
		}
		else
		{
			// nothing we can do.
			// create a new chunk.
			CreateNewChunk(SizeOfHeapInPages * 0x1000, 0x1000);
		}



		// create and map the actual space.
		uint64_t x = Physical::AllocatePage();

		Virtual::MapAddress(KernelHeapAddress + SizeOfHeapInPages * 0x1000, x, MapFlags, (Virtual::PageMapStructure*) GetKernelCR3());
		SizeOfHeapInPages++;
	}

	void* AllocateChunk(uint64_t s)
	{
		uint64_t Size = (uint64_t) s;

		// AutoMutex lock = AutoMutex(Mutexes::KernelHeap);

		// round to alignment.
		uint64_t as = RoundSize(Size);

		if(as == 0)
			return nullptr;

		// find first matching chunk.
		uint64_t in = FindFirstFreeSlot(as);

		// check if we didn't find one.
		if(in == 0)
		{
			ExpandHeap();
			return AllocateChunk(as);
		}
		else
		{
			// we must have found it.
			// check that it's actually valid.
			assert(!(in % sizeof(Chunk_type)));

			Chunk_type* c = (Chunk_type*) in;

			if(GetSize(c) > as)
			{
				SplitChunk(c, as);
			}

			c->Size &= (uint64_t) (~0x1);
			return (void*) (c->Offset + KernelHeapAddress);
		}
	}






	bool TryMergeLeft(Chunk_type* c)
	{
		// make sure that this is not the first chunk.
		if(addr(c) == KernelHeapMetadata + MetadataSize)
			return false;

		uint64_t p = KernelHeapMetadata + MetadataSize;
		bool found = false;

		for(uint64_t m = 0; m < NumberOfChunksInHeap && p < KernelHeapMetadata + (SizeOfMetadataInPages * 0x1000); m++)
		{
			Chunk_type* ct = (Chunk_type*) p;
			if(ct->Offset + GetSize(ct) == c->Offset && IsFree(ct))
			{
				found = true;
				break;
			}

			p += sizeof(Chunk_type);
		}

		if(found)
		{
			Chunk_type* c1 = (Chunk_type*) p;

			c->Offset = c1->Offset;
			c->Size += c1->Size;
			c->Size |= 0x1;

			c1->Offset = 0;
			c1->Size = 0;

			// merge with that instead.
			// c1->Size += GetSize(c);
			// c1->Size |= 0x1;

			// c->Offset = 0;
			// c->Size = 0;
		}

		return found;
	}



	bool TryMergeRight(Chunk_type* c)
	{
		// make sure that this is not the last chunk.
		if(addr(c) + sizeof(Chunk_type) >= KernelHeapMetadata + (SizeOfMetadataInPages * 0x1000))
			return false;

		uint64_t p = KernelHeapMetadata + MetadataSize;
		bool found = false;

		for(uint64_t m = 0; m < NumberOfChunksInHeap && p < KernelHeapMetadata + (SizeOfMetadataInPages * 0x1000); m++)
		{
			Chunk_type* ct = (Chunk_type*) p;
			if(c->Offset + GetSize(c) == ct->Offset && IsFree(ct))
			{
				found = true;
				break;
			}

			p += sizeof(Chunk_type);
		}

		if(found)
		{
			Chunk_type* c1 = (Chunk_type*) p;

			// merge with that instead.
			c->Size += GetSize(c1);
			c->Size |= 0x1;

			c1->Offset = 0;
			c1->Size = 0;
		}
		return false;
	}


	void FreeChunk(void* Pointer)
	{
		// AutoMutex lock = AutoMutex(Mutexes::KernelHeap);

		// find the corresponding chunk in the mdata.
		uint64_t p = KernelHeapMetadata + MetadataSize;
		Chunk_type* tc = 0;

		for(uint64_t k = 0; k < NumberOfChunksInHeap; k++)
		{
			Chunk_type* c = (Chunk_type*) p;
			if((uint64_t) Pointer - KernelHeapAddress == (uint64_t) c->Offset)
			{
				if(k < FirstFreeIndex)
					FirstFreeIndex = k;

				tc = c;
				break;
			}

			else
				p += sizeof(Chunk_type);
		}

		if(!tc)
		{
			if(Warn)
				Log(1, "Tried to free nonexistent chunk at address %x, return address: %x, ignoring...", (uint64_t) Pointer, __builtin_return_address(0));

			return;
		}

		// else, set it to free.

		tc->Size |= 0x1;
		Memory::Set((void*)((uint64_t)(tc->Offset + KernelHeapAddress)), 0x00, GetSize(tc));

		// try to merge left or right.
		TryMergeRight(tc);
		TryMergeLeft(tc);
	}









	void ContractHeap()
	{
		Log(1, "Cannot shrink heap, not implemented.");
	}


	uint64_t QuerySize(void* Address)
	{
		// we need to loop through the list of chunks to find an entry that matches.

		for(uint64_t n = 0; n < NumberOfChunksInHeap; n++)
		{
			if((uint64_t) Address - KernelHeapAddress == (uint64_t) GetChunkAtIndex(n)->Offset)
			{
				// we've got it.
				return GetSize(GetChunkAtIndex(n));
			}
		}
		return 0;
	}







	void Print()
	{
		uint64_t p = KernelHeapMetadata + MetadataSize;
		for(uint64_t n = 0; n < NumberOfChunksInHeap; n++)
		{
			Chunk_type* c = (Chunk_type*) p;
			Log("Offset: %x, Size: %x, IsFree: %b", (uint64_t) c->Offset, GetSize(c), IsFree(c));

			p += sizeof(Chunk_type);
		}
	}
}
}
}
}


