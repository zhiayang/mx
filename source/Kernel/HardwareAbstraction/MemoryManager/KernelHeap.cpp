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


static uint64_t GetPageFixed(uint64_t addr)
{
	// simple.
	uint64_t p = Physical::AllocatePage();
	Virtual::MapAddress(addr, p, 0x3);
	return addr;
}

// We have more than one heap implementation in this file.
// from top-bottom, they are:
// In place heap chunk (header + footer system)
// new-heap, featured in orion-x4/failures/new-heap
// old old heap, original orion-x4 heap.
//
// This is the standard interface required:
// void Initialise()
// void* AllocateChunk(size_t)
// void FreeChunk(void*)
// size_t QuerySize(void*)
// void Print()
// everything else is implementation defined.

// namespace Kernel {
// namespace HardwareAbstraction {
// namespace MemoryManager {
// namespace KernelHeap
// {
// 	// bookkeeping
// 	static uint64_t SizeOfHeap;
// 	static uint64_t HeapAddress;
// 	static uint64_t HeapEnd;

// 	#define FREE_HEADER_MAGIC		0xF1EEC0DE
// 	#define USED_HEADER_MAGIC	0xDEADC0DE
// 	#define FOOTER_MAGIC		0xC0DEBABE
// 	#define ALIGNMENT			64

// 	#define fail()				assert(0)

// 	struct _header
// 	{
// 		uint64_t size;
// 		uint32_t magic;
// 		uint32_t owningtid;

// 	} __attribute__ ((packed));

// 	struct _footer
// 	{
// 		_header* hdr;
// 		uint32_t magic;
// 		uint32_t pad;

// 	} __attribute__ ((packed));

// 	bool _verify(_header* hdr)
// 	{
// 		if(hdr->magic == FREE_HEADER_MAGIC || hdr->magic == USED_HEADER_MAGIC)
// 			return true;

// 		else
// 		{
// 			StandardIO::PrintFormatted("false: [%x - %x - %x, %x]\n", hdr, hdr->magic, __builtin_return_address(0), __builtin_return_address(1));
// 			return false;
// 		}
// 	}
// 	bool _verify(_footer* ftr)
// 	{
// 		if(ftr->magic == FOOTER_MAGIC)
// 			return true;

// 		else
// 		{
// 			StandardIO::PrintFormatted("[%x (%x) - %x, %x, %x]", ftr, ftr->hdr, ftr->magic, __builtin_return_address(0), __builtin_return_address(1));
// 			return false;
// 		}
// 	}


// 	_header* verify(_header* hdr)		{ assert(_verify(hdr)); return hdr; }
// 	_footer* verify(_footer* ftr)		{ assert(_verify(ftr)); return ftr; }

// 	_header* header(uint64_t addr)	{ return verify((_header*) addr); }
// 	_footer* footer(uint64_t addr)		{ return verify((_footer*) addr); }

// 	_header* mkheader(uint64_t at, size_t size)
// 	{
// 		_header* h = (_header*) at;
// 		h->magic = FREE_HEADER_MAGIC;
// 		h->owningtid = 0;
// 		h->size = size;

// 		return verify(h);
// 	}

// 	_footer* mkfooter(uint64_t at, _header* hdr)
// 	{
// 		_footer* ftr = (_footer*) at;
// 		ftr->hdr = verify(hdr);
// 		ftr->magic = FOOTER_MAGIC;
// 		ftr->pad = 0;

// 		return verify(ftr);
// 	}

// 	_footer* footer(_header* hdr)
// 	{
// 		verify(hdr);
// 		return footer((uint64_t) hdr + sizeof(_header) + hdr->size);
// 	}

// 	_header* left(_header* hdr)
// 	{
// 		verify(hdr);
// 		return verify(footer((uint64_t) hdr - sizeof(_footer))->hdr);
// 	}

// 	_header* right(_header* hdr)
// 	{
// 		verify(hdr);
// 		verify(footer((uint64_t) hdr + hdr->size));
// 		return verify(header((uint64_t) hdr + hdr->size + sizeof(_footer)));
// 	}

// 	bool isfree(_header* hdr)
// 	{
// 		verify(hdr);
// 		if(hdr->magic == FREE_HEADER_MAGIC)
// 			return true;

// 		else
// 			return false;
// 	}

// 	void setfree(_header* hdr)
// 	{
// 		verify(hdr)->magic = FREE_HEADER_MAGIC;
// 	}

// 	void setfree(_footer* ftr)
// 	{
// 		setfree(verify(ftr->hdr));
// 	}

// 	void setused(_header* hdr)
// 	{
// 		verify(hdr)->magic = USED_HEADER_MAGIC;
// 	}

// 	void setused(_footer* ftr)
// 	{
// 		setfree(verify(ftr->hdr));
// 	}

// 	size_t calcoverhead(size_t s)
// 	{
// 		return s > sizeof(_header) + sizeof(_footer) ? s - sizeof(_header) - sizeof(_footer) : 0;
// 	}

// 	uint64_t _round(uint64_t s)
// 	{
// 		uint64_t remainder = s % ALIGNMENT;

// 		if(remainder == 0)
// 			return s;

// 		return s + ALIGNMENT - remainder;
// 	}


// 	void Initialise()
// 	{
// 		SizeOfHeap = 1;
// 		HeapAddress = GetPageFixed(KernelHeapAddress);
// 		HeapEnd = HeapAddress + 0x1000;

// 		auto hdr = mkheader(HeapAddress, calcoverhead(0x1000));
// 		mkfooter(HeapAddress + sizeof(_header) + hdr->size, hdr);
// 	}

// 	void ExpandHeap()
// 	{
// 		// get the last chunk.
// 		_footer* ftr = footer(HeapEnd - sizeof(_footer));
// 		GetPageFixed(HeapEnd);
// 		if(isfree(ftr->hdr))
// 		{
// 			// add to its header's size, then move the footer
// 			verify(ftr->hdr)->size += 0x1000;
// 			auto f = mkfooter(HeapEnd + 0x1000 - sizeof(_footer), ftr->hdr);
// 			memset(ftr, 0, sizeof(_footer));

// 			verify(f);
// 		}
// 		else
// 		{
// 			auto hdr = mkheader(HeapEnd, calcoverhead(0x1000));
// 			mkfooter(HeapEnd + 0x1000 - sizeof(_footer), hdr);
// 			setfree(hdr);
// 		}

// 		SizeOfHeap++;
// 		HeapEnd += 0x1000;
// 	}

// 	void* AllocateChunk(size_t sz)
// 	{
// 		sz = _round(sz);
// 		uint64_t ptr = HeapAddress;
// 		_header* hdr = nullptr;
// 		while(ptr < HeapEnd)
// 		{
// 			_header* h = header(ptr);
// 			if(isfree(h))
// 			{
// 				hdr = h;
// 				break;
// 			}

// 			ptr += sizeof(_header) + h->size + sizeof(_footer);
// 		}

// 		if(hdr == nullptr)
// 		{
// 			ExpandHeap();
// 			return AllocateChunk(sz);
// 		}

// 		// set it to used.
// 		verify(hdr);
// 		setused(hdr);

// 		auto oldsz = hdr->size;
// 		if(oldsz - sz > sizeof(_header) + sizeof(_footer) + ALIGNMENT)
// 		{
// 			// we have enough space
// 			// make a new header right after the previous footer
// 			auto nh = mkheader((uint64_t) hdr + sizeof(_header) + sz, oldsz - sz - sizeof(_header) - sizeof(_footer));
// 			mkfooter((uint64_t) hdr + sizeof(_header) + verify(hdr)->size, nh);
// 			setfree(nh);
// 		}

// 		Log("allocated %x bytes at %x", sz, (uint64_t) hdr + sizeof(_header));
// 		return (void*) ((uint64_t) hdr + sizeof(_header));
// 	}

// 	void FreeChunk(void* ptr)
// 	{
// 	}

// 	size_t QuerySize(void* ptr)
// 	{
// 		return 0;
// 	}

// 	void Print()
// 	{
// 	}
// }
// }
// }
// }








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
			HALT("");
			// we need to expand.
			uint64_t fixed = GetPageFixed(MetadataAddr + (SizeOfMeta * 0x1000));
			assert(fixed == MetadataAddr + (SizeOfMeta * 0x1000));
			SizeOfMeta++;
		}

		auto behind = (ChunksInHeap - 1) - at;

		// memcpy.
		memcpy((void*) index(at + 1), (void*) index(at), behind * sizeof(Chunk));
		memset((void*) index(at), 0, sizeof(Chunk));
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

		size = _round(size);

		if(o <= ChunksInHeap)
		{
			// check if there are more chunks behind us.
			if(o + 1 < ChunksInHeap)
				pushback(o);

			// now that's solved, make the chunk.
			ChunksInHeap++;
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
		assert(o % Alignment == 0);

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
















// #define MaximumHeapSizeInPages			0xFFFFFFFF
// #define Alignment					32
// #define MetadataSize					32
// #define Warn						0
// #define MapFlags					0x7

// static uint64_t NumberOfChunksInHeap;
// static uint64_t SizeOfHeapInPages;
// static uint64_t SizeOfMetadataInPages;
// static uint64_t FirstFreeIndex;

// namespace Kernel {
// namespace HardwareAbstraction {
// namespace MemoryManager {
// namespace KernelHeap
// {
// 	bool DidInitialise = false;

// 	struct Chunk_type
// 	{
// 		uint64_t Offset;
// 		uint64_t Size;
// 		// uint64_t uuid;
// 		// uint64_t magic;

// 	} __attribute__ ((packed));




// 	uint64_t GetSize(Chunk_type* c)
// 	{
// 		uint64_t f = c->Size & (uint64_t)(~0x1);
// 		return f;
// 	}

// 	uint64_t addr(Chunk_type* c)
// 	{
// 		return (uint64_t) c;
// 	}

// 	bool IsFree(Chunk_type* c)
// 	{
// 		return c->Size & 0x1;
// 	}

// 	void ExpandMetadata()
// 	{
// 		uint64_t d = KernelHeapMetadata + (SizeOfMetadataInPages * 0x1000);
// 		uint64_t p = Physical::AllocatePage();

// 		Virtual::MapAddress(d, p, MapFlags);
// 		SizeOfMetadataInPages++;
// 	}

// 	uint64_t CreateNewChunk(uint64_t offset, uint64_t size)
// 	{
// 		uint64_t s = KernelHeapMetadata + MetadataSize;
// 		bool found = false;

// 		for(uint64_t m = 0; m < NumberOfChunksInHeap; m++)
// 		{
// 			Chunk_type* ct = (Chunk_type*) s;
// 			if((ct->Offset + GetSize(ct) == offset && IsFree(ct)) || (ct->Offset == 0 && ct->Size == 0))
// 			{
// 				found = true;
// 				break;
// 			}

// 			s += sizeof(Chunk_type);
// 		}

// 		if(found)
// 		{
// 			Chunk_type* c1 = (Chunk_type*) s;

// 			if(c1->Offset == 0 && c1->Size == 0)
// 			{
// 				// this is one of those derelict chunks, repurpose it.
// 				c1->Offset = offset;
// 				c1->Size = size & (uint64_t)(~0x1);
// 				c1->Size |= 0x1;
// 			}
// 			else
// 			{
// 				c1->Size += size;
// 				c1->Size |= 0x1;
// 			}

// 			return (uint64_t) c1;
// 		}




// 		uint64_t p = MetadataSize + ((NumberOfChunksInHeap - 1) * sizeof(Chunk_type));

// 		if(p + sizeof(Chunk_type) == SizeOfMetadataInPages * 0x1000)
// 		{
// 			// if this is the last one before the page
// 			// create a new one.
// 			ExpandMetadata();
// 		}

// 		// create another one.
// 		Chunk_type* c1 = (Chunk_type*)(KernelHeapMetadata + p + sizeof(Chunk_type));
// 		NumberOfChunksInHeap++;

// 		c1->Offset = offset;
// 		c1->Size = size | 0x1;

// 		return (uint64_t) c1;
// 	}

// 	Chunk_type* GetChunkAtIndex(uint64_t i)
// 	{
// 		uint64_t p = KernelHeapMetadata + MetadataSize + (i * sizeof(Chunk_type));
// 		return (Chunk_type*) p;
// 	}

// 	uint64_t RoundSize(uint64_t s)
// 	{
// 		uint64_t remainder = s % Alignment;

// 		if(remainder == 0)
// 			return s;

// 		return s + Alignment - remainder;
// 	}













// 	void Initialise()
// 	{
// 		// the first page will contain bits of metadata.
// 		// including: number of chunks.
// 		// size of heap in pages.

// 		// let's start.
// 		// allocate one page for the heap.
// 		// will be mapped for now.
// 		uint64_t h = Physical::AllocatePage();
// 		uint64_t d = Physical::AllocatePage();

// 		// also map it.
// 		Virtual::MapAddress(KernelHeapMetadata, h, MapFlags);
// 		Virtual::MapAddress(KernelHeapAddress, d, MapFlags);



// 		NumberOfChunksInHeap = 1;
// 		SizeOfHeapInPages = 1;
// 		SizeOfMetadataInPages = 1;
// 		FirstFreeIndex = 0;


// 		Chunk_type* fc = (Chunk_type*)(KernelHeapMetadata + MetadataSize);
// 		fc->Offset = 0;

// 		// bit zero stores free/not: 1 is free, 0 is not. that means we can only do 2-byte granularity.
// 		fc->Size = 0x1000 | 0x1;

// 		DidInitialise = true;
// 	}

// 	uint64_t FindFirstFreeSlot(uint64_t Size)
// 	{
// 		// free chunks are more often than not found at the back,
// 		// so fuck convention and scan from the back.

// 		#if 1
// 		{
// 			uint64_t p = KernelHeapMetadata + MetadataSize + (FirstFreeIndex * sizeof(Chunk_type));

// 			for(uint64_t k = 0; k < NumberOfChunksInHeap - FirstFreeIndex; k++)
// 			{
// 				Chunk_type* c = (Chunk_type*) p;
// 				if(GetSize(c) > Size && IsFree(c))
// 					return p;

// 				else
// 					p += sizeof(Chunk_type);
// 			}

// 			// because the minimum offset (from the meta address) is 8, zero will do.
// 			return 0;
// 		}

// 		#else
// 		{
// 			uint64_t p = KernelHeapMetadata + NumberOfChunksInHeap * sizeof(Chunk_type);

// 			for(uint64_t k = 0; k < NumberOfChunksInHeap; k++)
// 			{
// 				Chunk_type* c = (Chunk_type*) p;
// 				if(GetSize(c) > Size && IsFree(c))
// 					return p;

// 				else
// 					p -= sizeof(Chunk_type);
// 			}

// 			// because the minimum offset (from the meta address) is 8, zero will do.
// 			return 0;
// 		}
// 		#endif
// 	}


// 	void SplitChunk(Chunk_type* Header, uint64_t Size)
// 	{
// 		// round up to nearest two.
// 		uint64_t ns = ((Size / 2) + 1) * 2;

// 		assert(GetSize(Header) >= ns);

// 		// create a new chunk.
// 		// because of the inherently unordered nature of the list (ie. we can't insert arbitrarily into the middle of the
// 		// list, for it is a simple array), we cannot make assumptions (aka assert() s) about the Offset and Size fields.
// 		// because this function will definitely disorder the list, because CreateNewChunk() can only put
// 		// new chunks at the end of the list.

// 		uint64_t nsonc = GetSize(Header) - ns;
// 		CreateNewChunk(Header->Offset + ns, nsonc);

// 		Header->Size = ns | 0x1;
// 	}


// 	void ExpandHeap()
// 	{
// 		if(SizeOfHeapInPages == MaximumHeapSizeInPages)
// 		{
// 			asm volatile("cli");
// 			HALT("Heap out of memory! (Tried to expand beyond max amount of pages)");
// 		}


// 		uint64_t p = KernelHeapMetadata + MetadataSize;
// 		bool found = false;
// 		for(uint64_t m = 0; m < NumberOfChunksInHeap; m++)
// 		{
// 			Chunk_type* ct = (Chunk_type*) p;
// 			if(ct->Offset + GetSize(ct) == SizeOfHeapInPages * 0x1000 && IsFree(ct))
// 			{
// 				found = true;
// 				break;
// 			}

// 			p += sizeof(Chunk_type);
// 		}


// 		// check if the chunk before the new one is free.
// 		if(found)
// 		{
// 			Chunk_type* c1 = (Chunk_type*) p;

// 			// merge with that instead.
// 			c1->Size += 0x1000;
// 			c1->Size |= 0x1;
// 		}
// 		else
// 		{
// 			// nothing we can do.
// 			// create a new chunk.
// 			CreateNewChunk(SizeOfHeapInPages * 0x1000, 0x1000);
// 		}



// 		// create and map the actual space.
// 		uint64_t x = Physical::AllocatePage();

// 		Virtual::MapAddress(KernelHeapAddress + SizeOfHeapInPages * 0x1000, x, MapFlags);
// 		SizeOfHeapInPages++;
// 	}

// 	void* AllocateChunk(uint64_t s)
// 	{
// 		// round to alignment.
// 		uint64_t Size = (uint64_t) s;

// 		// AutoMutex lock = AutoMutex(Mutexes::KernelHeap);

// 		// round to alignment.
// 		uint64_t as = RoundSize(Size);

// 		// find first matching chunk.
// 		uint64_t in = FindFirstFreeSlot(as);

// 		// check if we didn't find one.
// 		if(in == 0)
// 		{
// 			ExpandHeap();
// 			return AllocateChunk(as);
// 		}
// 		else
// 		{
// 			// we must have found it.
// 			// check that it's actually valid.
// 			assert(!(in % sizeof(Chunk_type)));

// 			Chunk_type* c = (Chunk_type*) in;

// 			if(GetSize(c) > as)
// 			{
// 				SplitChunk(c, as);
// 			}

// 			c->Size &= (uint64_t) (~0x1);
// 			return (void*) (c->Offset + KernelHeapAddress);
// 		}
// 	}






// 	bool TryMergeLeft(Chunk_type* c)
// 	{
// 		// make sure that this is not the first chunk.
// 		if(addr(c) == KernelHeapMetadata + MetadataSize)
// 			return false;

// 		uint64_t p = KernelHeapMetadata + MetadataSize;
// 		bool found = false;

// 		for(uint64_t m = 0; m < NumberOfChunksInHeap && p < KernelHeapMetadata + (SizeOfMetadataInPages * 0x1000); m++)
// 		{
// 			Chunk_type* ct = (Chunk_type*) p;
// 			if(ct->Offset + GetSize(ct) == c->Offset && IsFree(ct))
// 			{
// 				found = true;
// 				break;
// 			}

// 			p += sizeof(Chunk_type);
// 		}

// 		if(found)
// 		{
// 			Chunk_type* c1 = (Chunk_type*) p;

// 			c->Offset = c1->Offset;
// 			c->Size += c1->Size;
// 			c->Size |= 0x1;

// 			c1->Offset = 0;
// 			c1->Size = 0;

// 			// merge with that instead.
// 			// c1->Size += GetSize(c);
// 			// c1->Size |= 0x1;

// 			// c->Offset = 0;
// 			// c->Size = 0;
// 		}

// 		return found;
// 	}



// 	bool TryMergeRight(Chunk_type* c)
// 	{
// 		// make sure that this is not the last chunk.
// 		if(addr(c) + sizeof(Chunk_type) >= KernelHeapMetadata + (SizeOfMetadataInPages * 0x1000))
// 			return false;

// 		uint64_t p = KernelHeapMetadata + MetadataSize;
// 		bool found = false;

// 		for(uint64_t m = 0; m < NumberOfChunksInHeap && p < KernelHeapMetadata + (SizeOfMetadataInPages * 0x1000); m++)
// 		{
// 			Chunk_type* ct = (Chunk_type*) p;
// 			if(c->Offset + GetSize(c) == ct->Offset && IsFree(ct))
// 			{
// 				found = true;
// 				break;
// 			}

// 			p += sizeof(Chunk_type);
// 		}

// 		if(found)
// 		{
// 			Chunk_type* c1 = (Chunk_type*) p;

// 			// merge with that instead.
// 			c->Size += GetSize(c1);
// 			c->Size |= 0x1;

// 			c1->Offset = 0;
// 			c1->Size = 0;
// 		}
// 		return false;
// 	}


// 	void FreeChunk(void* Pointer)
// 	{
// 		// AutoMutex lock = AutoMutex(Mutexes::KernelHeap);

// 		// find the corresponding chunk in the mdata.
// 		uint64_t p = KernelHeapMetadata + MetadataSize;
// 		Chunk_type* tc = 0;

// 		for(uint64_t k = 0; k < NumberOfChunksInHeap; k++)
// 		{
// 			Chunk_type* c = (Chunk_type*) p;
// 			if((uint64_t) Pointer - KernelHeapAddress == (uint64_t) c->Offset)
// 			{
// 				if(k < FirstFreeIndex)
// 					FirstFreeIndex = k;

// 				tc = c;
// 				break;
// 			}

// 			else
// 				p += sizeof(Chunk_type);
// 		}

// 		if(!tc)
// 		{
// 			if(Warn)
// 				Log(1, "Tried to free nonexistent chunk at address %x, return address: %x, ignoring...", (uint64_t) Pointer, __builtin_return_address(0));

// 			return;
// 		}

// 		// else, set it to free.

// 		tc->Size |= 0x1;
// 		Memory::Set((void*)((uint64_t)(tc->Offset + KernelHeapAddress)), 0x00, GetSize(tc));

// 		// try to merge left or right.
// 		TryMergeRight(tc);
// 		TryMergeLeft(tc);
// 	}









// 	void ContractHeap()
// 	{
// 		Log(1, "Cannot shrink heap, not implemented.");
// 	}


// 	uint64_t QuerySize(void* Address)
// 	{
// 		// we need to loop through the list of chunks to find an entry that matches.

// 		for(uint64_t n = 0; n < NumberOfChunksInHeap; n++)
// 		{
// 			if((uint64_t) Address - KernelHeapAddress == (uint64_t) GetChunkAtIndex(n)->Offset)
// 			{
// 				// we've got it.
// 				return GetSize(GetChunkAtIndex(n));
// 			}
// 		}
// 		return 0;
// 	}







// 	void Print()
// 	{
// 		uint64_t p = KernelHeapMetadata + MetadataSize;
// 		for(uint64_t n = 0; n < NumberOfChunksInHeap; n++)
// 		{
// 			Chunk_type* c = (Chunk_type*) p;
// 			Log("Offset: %x, Size: %x, IsFree: %b", (uint64_t) c->Offset, GetSize(c), IsFree(c));

// 			p += sizeof(Chunk_type);
// 		}
// 	}
// }
// }
// }
// }




