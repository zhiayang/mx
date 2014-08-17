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


#define PARANOIA					0x1
#define MapFlags					0x7
#define fail()						assert(0)
#define printf						Log



namespace Kernel {
namespace HardwareAbstraction {
namespace MemoryManager {
namespace KernelHeap
{
	// book keeping
	static uint64_t ChunksInHeap;
	static uint64_t LastFree;

	static uint64_t MetadataAddr;
	static uint64_t HeapAddress;

	static uint64_t SizeOfHeap;
	static uint64_t SizeOfMeta;

	const uint64_t Alignment = 64;

	static Mutex* mtx;

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
		return (Chunk*) MetadataAddr;
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

	static uint64_t GetPageFixed(uint64_t addr)
	{
		// simple.
		uint64_t p = Physical::AllocatePage();
		Virtual::MapAddress(addr, p, 0x3);
		return addr;
	}













	Chunk* index(uint64_t i)
	{
		Chunk* arr = (Chunk*) MetadataAddr;
		if(i < ChunksInHeap)
			return &arr[i];

		else
			return nullptr;
	}

	void pushback(uint64_t at)
	{
		// memcpy them behind.
		// but first, check if we have enough space.
		if((ChunksInHeap + 1) * sizeof(Chunk) > SizeOfMeta * 0x1000)
		{
			// we need to expand.
			uint64_t fixed = GetPageFixed(MetadataAddr + (SizeOfMeta * 0x1000));
			assert(fixed == MetadataAddr + (SizeOfMeta * 0x1000));
			SizeOfMeta++;
		}

		auto behind = (ChunksInHeap - 1) - at;

		// memcpy.
		memmove((void*) addr(index(at + 1)), (void*) index(at), behind * sizeof(Chunk));
		memset((void*) addr(index(at)), 0, sizeof(Chunk));
	}

	void pullfront(uint64_t at)
	{
		// essentially deletes a chunk.
		// 'at' contains the index of the chunk to delete.
		void* c = index(at);
		if(at == ChunksInHeap - 1)
			return;

		// calculate how many chunks to pull
		auto ahead = (ChunksInHeap - 1) - at;

		memset(c, 0, sizeof(Chunk));
		memmove(c, index(at + 1), ahead * sizeof(Chunk));
		memset((void*) ((uint64_t) c - 1 + ahead * sizeof(Chunk)), 0, sizeof(Chunk));
	}


	uint64_t _round(uint64_t s)
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
		MetadataAddr = GetPageFixed(KernelHeapMetadata);
		HeapAddress = GetPageFixed(KernelHeapAddress);

		SizeOfHeap = 1;
		SizeOfMeta = 1;
		ChunksInHeap = 1;
		LastFree = 0;

		Chunk* c = index(0);
		c->offset = 0;
		c->size = 0x1000;
		setfree(c);

		mtx = new Mutex();
	}

	void CreateChunk(uint64_t offset, uint64_t size)
	{
		uint64_t o = bsearch(offset, [](uint64_t i) -> uint64_t { return index(i)->offset; });
		if(o < ChunksInHeap)
		{
			if(index(o)->offset == offset)
			{
				assert("tried to create duplicate chunk" && false);
			}
		}

		ChunksInHeap++;

		if(o <= ChunksInHeap)
		{
			// check if there are more chunks behind us.
			if(o < ChunksInHeap - 1)
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


	void ExpandHeap()
	{
		// expand the heap.
		GetPageFixed(HeapAddress + (SizeOfHeap * 0x1000));

		// always offset sorted;
		Chunk* last = index(ChunksInHeap - 1);
		if(last->offset + size(last) != (SizeOfHeap * 0x1000))
		{
			printf("failure: %llx + %llx != %llx\n", last->offset, size(last), SizeOfHeap * 0x1000);
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



	void* AllocateChunk(uint64_t sz)
	{
		// auto m = AutoMutex(mtx);
		LOCK(mtx);
		sz = _round(sz);
		assert((sz % Alignment) == 0);
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
			UNLOCK(mtx);
			return AllocateChunk(sz);
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

		assert(sz % Alignment == 0);
		assert(((HeapAddress + o) % Alignment) == 0);

		UNLOCK(mtx);
		return (void*) (HeapAddress + o);
	}

	void FreeChunk(void* ptr)
	{
		// auto m = AutoMutex(mtx);
		LOCK(mtx);
		uint64_t p = (uint64_t) ptr;
		assert(p >= HeapAddress);

		p -= HeapAddress;
		assert(p % Alignment == 0);

		// this is where the offset-sorted list comes in handy.
		uint64_t o = bsearch(p, [](uint64_t i) -> uint64_t { return index(i)->offset; });
		Chunk* self = index(o);

		if(self->offset != p)
		{
			Print();
			Log(3, "%x pages in heap: %x - got %x, expected %x, index %d, %d CIH", SizeOfHeap, __builtin_return_address(0), p, self->offset, o, ChunksInHeap);
			fail();
		}

		setfree(self);

		// do merge here.
		// check right first, because checking left may modify our own state.
		if(o + 1 < ChunksInHeap)
		{
			// check right neighbour
			Chunk* right = index(o + 1);
			if(self->offset + size(self) != right->offset)
			{
				printf("failure: %x + %x != %x, %x chunks in heap", self->offset, size(self), right->offset, ChunksInHeap);
				printf("left(%x):\toffset %x, size %x - %x", o - 1, index(o - 1)->offset, size(index(o - 1)), isfree(index(o - 1)));
				printf("self(%x):\toffset %x, size %x - %x", o, self->offset, size(self), isfree(index(o)));
				printf("right(%x):\toffset %x, size %x - %x", o + 1, right->offset, size(right), isfree(index(o + 1)));
				printf("right2(%x):\toffset %x, size %x - %x", o + 2, index(o + 2)->offset, size(index(o + 2)), isfree(index(o + 2)));
				fail();
			}

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
				printf("failure: %llx + %llx != %llx\n", left->offset, size(left), self->offset);
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
		if(o < LastFree)
		{
			assert(isfree(index(o)));
			LastFree = o;
		}

		UNLOCK(mtx);
		// Log("free %x", p);
	}

	uint64_t QuerySize(void* ptr)
	{
		auto m = AutoMutex(mtx);

		uint64_t p = (uint64_t) ptr;
		assert(p >= HeapAddress);

		p -= HeapAddress;

		// this is where the offset-sorted list comes in handy.
		uint64_t o = bsearch(p, [](uint64_t i) -> uint64_t { return index(i)->offset; });
		Chunk* self = index(o);


		if(self->offset != p)
		{
			printf("failure: got offset %llx, expected %llx", p, self->offset);
			fail();
		}

		return size(self);
	}

	void Print()
	{
		Log(3, "%x chunks in heap", ChunksInHeap);
		for(uint64_t i = ChunksInHeap - 1; i > 0; i--)
			Log(1, "chunk(%x) => offset %x, size %x - %x", i, index(i)->offset, size(index(i)), isfree(index(i)) ? 1 : 0);

		Log(1, "Done");
	}
}
}
}
}





