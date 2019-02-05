// KernelHeap.hpp
// Copyright (c) 2013 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#pragma once
#include <stdint.h>

namespace Kernel {
namespace HardwareAbstraction {
namespace MemoryManager
{
	namespace KernelHeap
	{
		struct Chunk_type;
		extern bool DidInitialise;

		void Initialise();

		#ifndef STRINGIFY
			#define STRINGIFY0(x) #x
			#define STRINGIFY(x) STRINGIFY0(x)
		#endif

		void* AllocateChunk(uint64_t size);
		void FreeChunk(void* Pointer);
		void* ReallocateChunk(void* ptr, uint64_t size);

		void Print();

		uint64_t GetFirstHeapMetadataPhysPage();
		uint64_t GetFirstHeapPhysPage();
	}
}
}
}
