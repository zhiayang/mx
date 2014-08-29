// _heap.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "../../include/stdint.h"
#include "../../include/string.h"
#include "../../include/stdlib.h"
#include "../../include/assert.h"
#include "../../include/stdio.h"
#include "../../include/sys/mman.h"

namespace Heap
{
	// plugs:
	#define GetPage()		((uint64_t) mmap(0, 0x1000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, 0, 0))
	#define GetPageFixed(x)	((uint64_t) mmap((void*) (x), 0x1000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_FIXED | MAP_ANON, 0, 0))
	#define fail()			abort()

	#if 0
	static uint64_t NumberOfChunksInHeap;
	static uint64_t SizeOfHeapInPages;
	static uint64_t SizeOfMetadataInPages;
	static uint64_t FirstFreeIndex;
	static uint64_t MetadataAddr;
	static uint64_t HeapAddr;
	static uint64_t Alignment = 64;

	struct Chunk_type
	{
		uint64_t Offset;
		uint64_t Size;

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
		// uint64_t d = KernelHeapMetadata + (SizeOfMetadataInPages * 0x1000);
		// uint64_t p = Physical::AllocatePage();

		// Virtual::MapAddress(d, p, MapFlags);
		GetPageFixed(MetadataAddr + (SizeOfMetadataInPages * 0x1000));
		SizeOfMetadataInPages++;
	}

	uint64_t CreateNewChunk(uint64_t offset, uint64_t size)
	{
		uint64_t s = MetadataAddr;
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




		uint64_t p = ((NumberOfChunksInHeap - 1) * sizeof(Chunk_type));

		if(p + sizeof(Chunk_type) == SizeOfMetadataInPages * 0x1000)
		{
			// if this is the last one before the page
			// create a new one.
			ExpandMetadata();
		}

		// create another one.
		Chunk_type* c1 = (Chunk_type*)(MetadataAddr + p + sizeof(Chunk_type));
		NumberOfChunksInHeap++;

		c1->Offset = offset;
		c1->Size = size | 0x1;

		return (uint64_t) c1;
	}

	Chunk_type* GetChunkAtIndex(uint64_t i)
	{
		uint64_t p = MetadataAddr + (i * sizeof(Chunk_type));
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
		// uint64_t h = Physical::AllocatePage();
		// uint64_t d = Physical::AllocatePage();

		// also map it.
		// Virtual::MapAddress(KernelHeapMetadata, h, MapFlags);
		// Virtual::MapAddress(KernelHeapAddress, d, MapFlags);

		MetadataAddr = GetPage();
		HeapAddr = GetPage();



		NumberOfChunksInHeap = 1;
		SizeOfHeapInPages = 1;
		SizeOfMetadataInPages = 1;
		FirstFreeIndex = 0;


		Chunk_type* fc = (Chunk_type*)(MetadataAddr);
		fc->Offset = 0;

		// bit zero stores free/not: 1 is free, 0 is not. that means we can only do 2-byte granularity.
		fc->Size = 0x1000 | 0x1;
	}

	uint64_t FindFirstFreeSlot(uint64_t Size)
	{
		// free chunks are more often than not found at the back,
		// so fuck convention and scan from the back.

		uint64_t p = MetadataAddr + (FirstFreeIndex * sizeof(Chunk_type));

		for(uint64_t k = 0; k < NumberOfChunksInHeap - FirstFreeIndex; k++)
		{
			Chunk_type* c = (Chunk_type*) p;
			if(GetSize(c) > Size && IsFree(c))
				return p;

			else
				p += sizeof(Chunk_type);
		}

		// because the minimum offset (from the meta address) is 8, zero will do.
		assert(false);
		return 0;
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
		// if(SizeOfHeapInPages == MaximumHeapSizeInPages)
		// {
		// 	asm volatile("cli");
		// 	HALT("Heap out of memory! (Tried to expand beyond max amount of pages)");
		// }


		uint64_t p = MetadataAddr;
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
		// uint64_t x = Physical::AllocatePage();

		// Virtual::MapAddress(KernelHeapAddress + SizeOfHeapInPages * 0x1000, x, MapFlags);
		GetPageFixed(HeapAddr + SizeOfHeapInPages * 0x1000);
		SizeOfHeapInPages++;
	}

	void* Allocate(uint64_t s)
	{
		// round to alignment.
		uint64_t Size = (uint64_t) s;

		// AutoMutex lock = AutoMutex(Mutexes::KernelHeap);

		// round to alignment.
		uint64_t as = RoundSize(Size);

		// find first matching chunk.
		uint64_t in = FindFirstFreeSlot(as);

		// check if we didn't find one.
		if(in == 0)
		{
			ExpandHeap();
			return Allocate(as);
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
			return (void*) (c->Offset + HeapAddr);
		}
	}






	bool TryMergeLeft(Chunk_type* c)
	{
		// make sure that this is not the first chunk.
		if(addr(c) == MetadataAddr)
			return false;

		uint64_t p = MetadataAddr;
		bool found = false;

		for(uint64_t m = 0; m < NumberOfChunksInHeap && p < MetadataAddr + (SizeOfMetadataInPages * 0x1000); m++)
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
		if(addr(c) + sizeof(Chunk_type) >= MetadataAddr + (SizeOfMetadataInPages * 0x1000))
			return false;

		uint64_t p = MetadataAddr;
		bool found = false;

		for(uint64_t m = 0; m < NumberOfChunksInHeap && p < MetadataAddr + (SizeOfMetadataInPages * 0x1000); m++)
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


	void Free(void* Pointer)
	{
		// AutoMutex lock = AutoMutex(Mutexes::KernelHeap);

		// find the corresponding chunk in the mdata.
		uint64_t p = MetadataAddr;
		Chunk_type* tc = 0;

		for(uint64_t k = 0; k < NumberOfChunksInHeap; k++)
		{
			Chunk_type* c = (Chunk_type*) p;
			if((uint64_t) Pointer - HeapAddr == (uint64_t) c->Offset)
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
			// if(Warn)
			// 	Log(1, "Tried to free nonexistent chunk at address %x, return address: %x, ignoring...", (uint64_t) Pointer, __builtin_return_address(0));
			printf("err");
			return;
		}

		// else, set it to free.

		tc->Size |= 0x1;
		memset((void*)((uint64_t)(tc->Offset + HeapAddr)), 0x00, GetSize(tc));

		// try to merge left or right.
		TryMergeRight(tc);
		TryMergeLeft(tc);
	}





	void* Reallocate(void* ptr, uint64_t newsize)
	{
		if(newsize == 0)
		{
			Free(ptr);
			return NULL;
		}


		// find the corresponding chunk in the mdata.
		uint64_t p = MetadataAddr;
		Chunk_type* tc = 0;

		for(uint64_t k = 0; k < NumberOfChunksInHeap; k++)
		{
			Chunk_type* c = (Chunk_type*) p;
			if((uint64_t) ptr - HeapAddr == (uint64_t) c->Offset)
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
			// if(Warn)
			// 	Log(1, "Tried to free nonexistent chunk at address %x, return address: %x, ignoring...", (uint64_t) Pointer, __builtin_return_address(0));

			return NULL;
		}

		// else, set it to free.
		auto s = tc->Size & ~0x1;
		void* newc = Allocate(newsize);
		memcpy(newc, ptr, newsize > s ? s : newsize);

		Free(ptr);
		return newc;
	}




	void ContractHeap()
	{
		// Log(1, "Cannot shrink heap, not implemented.");
	}


	uint64_t QuerySize(void* Address)
	{
		// we need to loop through the list of chunks to find an entry that matches.

		for(uint64_t n = 0; n < NumberOfChunksInHeap; n++)
		{
			if((uint64_t) Address - HeapAddr == (uint64_t) GetChunkAtIndex(n)->Offset)
			{
				// we've got it.
				return GetSize(GetChunkAtIndex(n));
			}
		}
		return 0;
	}







	void Print()
	{
		uint64_t p = MetadataAddr;
		for(uint64_t n = 0; n < NumberOfChunksInHeap; n++)
		{
			Chunk_type* c = (Chunk_type*) p;
			printf("Offset: %x, Size: %x, IsFree: %b\n", (uint64_t) c->Offset, GetSize(c), IsFree(c));

			p += sizeof(Chunk_type);
		}
	}


	#else
	// book keeping
	static uint64_t ChunksInHeap;
	static uint64_t LastFree;

	static uint64_t MetadataAddr;
	static uint64_t HeapAddress;

	static uint64_t SizeOfHeap;
	static uint64_t SizeOfMeta;

	const uint64_t MetaOffset = 32;
	const uint64_t Alignment = 32;

	struct Chunk
	{
		uint64_t offset;
		uint64_t size;
	};

	uint64_t addr(Chunk* c)
	{
		return (uint64_t) c;
	}

	Chunk* chunk(uint64_t addr)
	{
		return (Chunk*) addr;
	}

	Chunk* array()
	{
		return (Chunk*) (MetadataAddr + MetaOffset);
	}

	bool isfree(Chunk* c)
	{
		return c->size & 0x1;
	}

	void setfree(Chunk* c)
	{
		c->size |= 0x1;
	}

	void setused(Chunk* c)
	{
		c->size &= ~0x1;
	}

	uint64_t size(Chunk* c)
	{
		return c->size & ~0x1;
	}

	uint64_t midpoint(uint64_t a, uint64_t b)
	{
		return a + ((b - a) / 2);
	}

	static void CheckAndExpandMeta()
	{
		if((ChunksInHeap + 2) * sizeof(Chunk) > SizeOfMeta * 0x1000)
		{
			// we need to expand.
			uint64_t fixed = GetPageFixed(MetadataAddr + (SizeOfMeta * 0x1000));
			assert(fixed == MetadataAddr + (SizeOfMeta * 0x1000));
			SizeOfMeta++;
		}
	}


	Chunk* index(uint64_t i)
	{
		Chunk* arr = (Chunk*) (MetadataAddr + MetaOffset);
		if(i < ChunksInHeap)
			return &arr[i];

		else
			return nullptr;
	}

	void pushback(uint64_t at)
	{
		// memcpy them behind.
		// but first, check if we have enough space.
		CheckAndExpandMeta();
		auto behind = ChunksInHeap - at;

		// memcpy.
		memmove((void*) addr(index(at + 1)), (void*) index(at), behind * sizeof(Chunk));
		memset((void*) index(at), 0, sizeof(Chunk));
	}

	void pullfront(uint64_t at)
	{
		// essentially deletes a chunk.
		// 'at' contains the index of the chunk to delete.
		void* c = index(at);
		CheckAndExpandMeta();

		if(at == ChunksInHeap - 1)
			return;

		// calculate how many chunks to pull
		auto ahead = (ChunksInHeap - 1) - at;

		memset(c, 0, sizeof(Chunk));
		memmove(c, index(at + 1), ahead * sizeof(Chunk));
	}





	uint64_t round(uint64_t s)
	{
		uint64_t remainder = s % Alignment;

		if(remainder == 0)
			return s;

		return s + Alignment - remainder;
	}






	uint64_t bsearch(uint64_t key, uint64_t (*getkey)(uint64_t ind))
	{
		uint64_t imin = 0;
		uint64_t imax = ChunksInHeap - 1;
		uint64_t remaining = ChunksInHeap;

		while(imax >= imin && remaining > 0)
		{
			// calculate the midpoint for roughly equal partition
			auto imid = midpoint(imin, imax);

			// determine which subarray to search
			if(getkey(imid) < key)
			{
				if(imid < ChunksInHeap)
					imin = imid + 1;

				else
					break;
			}

			else if(getkey(imid) > key)
			{
				if(imid > 0)
					imax = imid - 1;

				else
					break;
			}

			else if(getkey(imid) == key)
				return imid;

			remaining--;
		}

		return imin;
	}








	// implementation:
	void Initialise()
	{
		MetadataAddr = GetPage();
		HeapAddress = GetPage();

		SizeOfHeap = 1;
		SizeOfMeta = 1;
		ChunksInHeap = 1;
		LastFree = 0;

		Chunk* c = index(0);
		c->offset = 0;
		c->size = 0x1000;
		setfree(c);
	}

	void CreateChunk(uint64_t offset, uint64_t size)
	{
		uint64_t o = bsearch(offset, [](uint64_t i) -> uint64_t { return index(i)->offset; });
		ChunksInHeap++;

		// create chunk in offset-sorted metalist
		{
			if(o <= ChunksInHeap)
			{
				// check if there are more chunks behind us.
				// if(o < ChunksInHeap - 1)
					pushback(o);

				// now that's solved, make the chunk.
				Chunk* c = index(o);
				c->offset = offset;
				c->size = size;
				setfree(c);
			}
			else
				fail();
		}
	}


	void ExpandHeap()
	{
		// expand the heap.
		GetPageFixed(HeapAddress + (SizeOfHeap * 0x1000));

		// always offset sorted;
		Chunk* last = index(ChunksInHeap - 1);
		if(last->offset + size(last) != (SizeOfHeap * 0x1000))
		{
			fprintf(stderr, "failure: %lx + %lx != %lx\n", last->offset, size(last), SizeOfHeap * 0x1000);
			fail();
		}


		// either create a new chunk, or expand the last one.
		if(isfree(last))
		{
			last->size += 0x1000;
			setfree(last);
		}
		else
		{
			CreateChunk(SizeOfHeap * 0x1000, 0x1000);
		}
		SizeOfHeap++;
	}



	void* Allocate(size_t sz)
	{
		sz = round(sz);
		// loop through each chunk, hoping to find something big enough.

		Chunk* c = 0;
		for(uint64_t i = LastFree; i < ChunksInHeap; i++)
		{
			if(size(index(i)) >= sz && isfree(index(i)))
			{
				c = index(i);
				break;
			}
		}
		if(c == 0)
		{
			ExpandHeap();
			return Allocate(sz);
		}

		uint64_t o = c->offset;
		uint64_t oldsize = size(c);

		c->size = sz;
		setused(c);

		auto newsize = oldsize - sz;
		if(newsize >= Alignment)
		{
			CreateChunk(c->offset + sz, newsize - (newsize % Alignment));
		}
		else
		{
			c->size = oldsize;
			setused(c);
		}

		assert(sz % Alignment == 0);
		assert(o % Alignment == 0);
		return (void*) (HeapAddress + o);
	}

	void Free(void* ptr)
	{
		uint64_t p = (uint64_t) ptr;
		assert(p >= HeapAddress);
		p -= HeapAddress;

		// this is where the offset-sorted list comes in handy.
		uint64_t o = bsearch(p, [](uint64_t i) -> uint64_t { return index(i)->offset; });
		Chunk* self = index(o);


		if(self->offset != p)
		{
			fprintf(stderr, "failure: got offset %lx, expected %lx", p, self->offset);
			fail();
		}

		setfree(self);

		// do merge here.
		// check right first, because checking left may modify our own state.

		if(o < ChunksInHeap - 1)
		{
			// check right neighbour
			Chunk* right = index(o + 1);
			assert(self->offset + size(self) == right->offset);
			if(isfree(right))
			{
				self->size += size(right);
				setfree(self);

				pullfront(o + 1);
				ChunksInHeap--;
			}
		}
		if(o > 0)
		{
			// check left neighbour
			Chunk* left = index(o - 1);
			if(left->offset + size(left) != self->offset)
			{
				fprintf(stderr, "failure: %lx + %lx != %lx\n", left->offset, size(left), self->offset);
				fail();
			}

			if(isfree(left))
			{
				// do merge
				left->size += size(self);
				setfree(left);

				// delete us.
				pullfront(o);
				ChunksInHeap--;
			}
		}
	}

	void* Reallocate(void* ptr, size_t newsize)
	{
		// get a new chunk.
		if(newsize == 0)
		{
			Free(ptr);
			return NULL;
		}
		else
		{
			uint64_t p = (uint64_t) ptr;
			assert(p >= HeapAddress);
			p -= HeapAddress;

			// this is where the offset-sorted list comes in handy.
			uint64_t o = bsearch(p, [](uint64_t i) -> uint64_t { return index(i)->offset; });
			Chunk* self = index(o);

			if(self->offset != p)
			{
				fprintf(stderr, "failure: got offset %lx, expected %lx", p, self->offset);
				fail();
			}

			// 'self' is now valid.
			// get the offset.
			auto s = size(self);
			void* newc = Allocate(newsize);
			memcpy(newc, ptr, newsize > s ? s : newsize);

			Free(ptr);
			return newc;
		}
	}

	void Print()
	{
		// for(size_t i = 0; i < ChunksInHeap; i++)
		for(long i = ChunksInHeap - 1; i >= 0; i--)
		{
			Chunk* c = index(i);
			printf("[%d]: (%#x, %#x) - %d\n", i, c->offset, size(c), isfree(c));
		}
	}
	#endif

}




















