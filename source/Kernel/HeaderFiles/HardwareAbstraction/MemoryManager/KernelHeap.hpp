// KernelHeap.hpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
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

		void* AllocateChunk(uint64_t Size);
		void FreeChunk(void* Pointer);

		uint64_t QuerySize(void* Address);

		void Print();
	}
}
}
}
