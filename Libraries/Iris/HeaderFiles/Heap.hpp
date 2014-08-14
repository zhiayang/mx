// Heap.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


// Heap things.
#include <stdint.h>

namespace Library {
namespace Heap
{
	struct __attribute__((packed)) Chunk_type
	{
		uint32_t Offset;
		uint32_t Size;
	};


	// #define DefaultHeapMetadata		0xFFFFFFFF10000000
	// #define DefaultHeapAddress		0xFFFFFFFF20000000

	#define DefaultHeapMetadata		0x100000000
	#define DefaultHeapAddress		0x200000000

	void Initialise();
	uint64_t FindFirstFreeSlot(uint32_t Size);
	void* AllocateChunk(uint64_t s);
	void FreeChunk(void* Pointer);
	void ExpandHeap(uint64_t pages);
	void TryMergeLeft(Chunk_type* c);
	void TryMergeRight(Chunk_type* c);
	void SplitChunk(Chunk_type* Header, uint32_t Size);
	void ContractHeap();
	uint64_t GetHeapSize();
	uint64_t GetMetadataSize();
	uint64_t QuerySize(void* Address);
	void DBG_PrintChunks();
}
}



