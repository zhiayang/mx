// Heap.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


// Heap things.

#include "../HeaderFiles/Memory.hpp"
#include "../HeaderFiles/SystemCall.hpp"
#include "../HeaderFiles/Heap.hpp"

using namespace Library;

#define MaximumHeapSizeInPages			0xFFF
#define Alignment					16
#define MetadataSize					8


static uint32_t NumberOfChunksInHeap;
static uint16_t SizeOfHeapInPages;
static uint16_t SizeOfMetadataInPages;


void operator delete(void* p)
{
	Library::Heap::FreeChunk(p);
}

void operator delete[](void* p)
{
	Library::Heap::FreeChunk(p);
}

void* operator new(unsigned long size)
{
	return Library::Heap::AllocateChunk(size);
}

void* operator new[](unsigned long size)
{
	return Library::Heap::AllocateChunk(size);
}

void* operator new(unsigned long, void* addr)
{
	return addr;
}


namespace Library {
namespace Heap
{
	uint32_t GetSize(Chunk_type* c)
	{
		uint32_t f = c->Size & (uint32_t)(~0x1);
		return f;
	}

	bool IsFree(Chunk_type* c)
	{
		return c->Size & 0x1;
	}


	void ExpandMetadata()
	{
		uint64_t d = DefaultHeapMetadata + (SizeOfMetadataInPages * 0x1000);


		SystemCall::MapVirtualAddress(d, SystemCall::AllocatePage(), 0x07);
		(SizeOfMetadataInPages)++;
	}

	uint64_t CreateNewChunk(uint32_t offset, uint32_t size)
	{
		uint64_t s = DefaultHeapMetadata + MetadataSize;
		bool found = false;


		for(uint32_t m = 0; m < NumberOfChunksInHeap; m++)
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
				c1->Size = size & (uint32_t)(~0x1);
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
		Chunk_type* c1 = (Chunk_type*)(DefaultHeapMetadata + p + sizeof(Chunk_type));
		NumberOfChunksInHeap++;

		c1->Offset = offset;
		c1->Size = size | 0x1;

		return (uint64_t) c1;
	}

	Chunk_type* GetChunkAtIndex(uint32_t i)
	{
		uint64_t p = DefaultHeapMetadata + MetadataSize + (i * sizeof(Chunk_type));
		return (Chunk_type*) p;
	}

	uint32_t RoundSize(uint32_t s)
	{
		uint32_t remainder = s % Alignment;

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
		uint64_t h = SystemCall::AllocatePage();
		uint64_t d = SystemCall::AllocatePage();

		// also map it.
		SystemCall::MapVirtualAddress(DefaultHeapMetadata, h, 0x07);
		SystemCall::MapVirtualAddress(DefaultHeapAddress, d, 0x07);


		// structure:
		/*
			Offset 0, Size 4
			NumberOfChunksInHeap

			Offset 4, Size 2
			SizeOfHeapInPages

			Offset 6, Size 2
			SizeOfMetadataInPages
		*/


		NumberOfChunksInHeap = 1;
		SizeOfHeapInPages = 1;
		SizeOfMetadataInPages = 1;




		Chunk_type* fc = (Chunk_type*)(DefaultHeapMetadata + MetadataSize);
		fc->Offset = 0;

		// bit zero stores free/not: 1 is free, 0 is not. that means we can only do 2-byte granularity.
		fc->Size = 0x1000 | 0x1;
	}

	uint64_t FindFirstFreeSlot(uint32_t Size)
	{
		// free chunks are more often than not found at the back,
		// so fuck convention and scan from the back.

		if(true)
		{
			uint64_t p = DefaultHeapMetadata + MetadataSize;

			for(uint32_t k = 0; k < NumberOfChunksInHeap; k++)
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
		else
		{
			uint64_t p = DefaultHeapMetadata + (NumberOfChunksInHeap * sizeof(Chunk_type));

			for(uint32_t k = 0; k < NumberOfChunksInHeap; k++)
			{
				Chunk_type* c = (Chunk_type*) p;
				if(GetSize(c) > Size && IsFree(c))
					return p;

				else
					p -= sizeof(Chunk_type);
			}

			// because the minimum offset (from the meta address) is 8, zero will do.
			return 0;
		}
	}





	void* AllocateChunk(uint64_t s)
	{
		// round to alignment.
		uint32_t Size = (uint32_t) s;

		uint32_t as = RoundSize(Size);

		// find first matching chunk.
		uint64_t in = FindFirstFreeSlot(as);

		// check if we didn't find one.
		if(in == 0)
		{
			ExpandHeap(1);
			return AllocateChunk(as);
		}
		else
		{
			Chunk_type* c = (Chunk_type*) in;

			if(GetSize(c) > as)
			{
				SplitChunk(c, as);
			}

			c->Size &= (uint32_t)(~0x1);
			return (void*)(c->Offset + DefaultHeapAddress);
		}
	}

	void FreeChunk(void* Pointer)
	{
		// find the corresponding chunk in the mdata.
		uint64_t p = DefaultHeapMetadata + MetadataSize;
		Chunk_type* tc = 0;


		for(uint32_t k = 0; k < NumberOfChunksInHeap; k++)
		{
			Chunk_type* c = (Chunk_type*) p;
			if((uint64_t) Pointer == (uint64_t) c->Offset + DefaultHeapAddress)
				tc = c;

			else
				p += sizeof(Chunk_type);
		}

		if(!tc)
		{
			return;
		}



		// else, set it to free.
		tc->Size |= 0x1;
		Memory::Set((void*)((uint64_t)((uint64_t) tc->Offset + DefaultHeapAddress)), 0x00, GetSize(tc));

		// try to merge left or right.
		// TryMergeLeft(tc);
		// TryMergeRight(tc);
	}








	void ExpandHeap(uint64_t pages)
	{
		if(SizeOfHeapInPages == MaximumHeapSizeInPages)
			return;

		uint64_t p = DefaultHeapMetadata + MetadataSize;
		bool found = false;
		for(uint32_t m = 0; m < NumberOfChunksInHeap; m++)
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
			c1->Size += 0x1000 * pages;
			c1->Size |= 0x1;
		}
		else
		{
			// nothing we can do.
			// create a new chunk.
			CreateNewChunk((SizeOfHeapInPages * 0x1000), (uint32_t)(0x1000 * pages));
		}

		// create and map the actual space.
		for(uint64_t i = 0; i < pages; i++)
		{
			SystemCall::MapVirtualAddress(DefaultHeapAddress + (SizeOfHeapInPages * 0x1000) + (i * 0x1000), SystemCall::AllocatePage(), 0x07);
		}

		SizeOfHeapInPages += pages;
	}



	void TryMergeLeft(Chunk_type* c)
	{
		// make sure that this is not the first chunk.
		if((uint64_t) c == DefaultHeapMetadata + MetadataSize)
			return;

		uint64_t p = DefaultHeapMetadata + MetadataSize;
		bool found = false;
		for(uint32_t m = 0; m < NumberOfChunksInHeap; m++)
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

			// merge with that instead.
			c1->Size += GetSize(c);
			c1->Size |= 0x1;

			c->Offset = 0;
			c->Size = 0;

			NumberOfChunksInHeap--;
		}
	}



	void TryMergeRight(Chunk_type* c)
	{
		// make sure that this is not the last chunk.
		if((uint64_t) GetChunkAtIndex((uint32_t)(NumberOfChunksInHeap) - 1) == (uint64_t) c)
			return;

		uint64_t p = DefaultHeapMetadata + MetadataSize;
		bool found = false;
		for(uint32_t m = 0; m < NumberOfChunksInHeap; m++)
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

			NumberOfChunksInHeap--;
		}
	}


	void SplitChunk(Chunk_type* Header, uint32_t Size)
	{
		// round up to nearest two.
		uint32_t ns = ((Size / 2) + 1) * 2;

		// create a new chunk.
		// because of the inherently unordered nature of the list (ie. we can't insert arbitrarily into the middle of the
		// list, for it is a simple array.

		// therefore we cannot make assumptions (aka assert() s) about the Offset and Size fields.
		// because this function will definitely disorder the list, because CreateNewChunk() can only put
		// new chunks at the end of the list.

		uint32_t nsonc = GetSize(Header) - ns;
		CreateNewChunk(Header->Offset + ns, nsonc);

		Header->Size = ns | 0x1;
	}


	void ContractHeap()
	{
	}


	uint64_t GetHeapSize()
	{
		return SizeOfHeapInPages;
	}

	uint64_t GetMetadataSize()
	{
		return SizeOfMetadataInPages;
	}



	uint64_t QuerySize(void* Address)
	{
		// we need to loop through the list of chunks to find an entry that matches.

		for(uint32_t n = 0; n < NumberOfChunksInHeap; n++)
		{
			if((uint64_t) Address >= (uint64_t)((uint64_t) GetChunkAtIndex(n)->Offset + DefaultHeapAddress) && (uint64_t) Address < (uint64_t)((uint64_t) GetChunkAtIndex(n)->Offset + DefaultHeapAddress + GetSize(GetChunkAtIndex(n))))
			{
				// we've got it.
				return GetSize(GetChunkAtIndex(n));
			}
			else
			{
				// StandardIO::PrintFormatted("%x != %x\n", (uint64_t) Address - DefaultHeapAddress, (uint64_t) GetChunkAtIndex(n)->Offset);
			}
		}
		return 0;
	}

}
}





