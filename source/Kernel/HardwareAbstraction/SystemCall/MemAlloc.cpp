// MemAlloc.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#include <Kernel.hpp>

namespace Kernel {
namespace HardwareAbstraction {
namespace SystemCalls
{
	extern "C" uint64_t Syscall_AllocatePage()
	{
		return MemoryManager::Physical::AllocatePage();
	}

	extern "C" void Syscall_FreePage(uint64_t p)
	{
		MemoryManager::Physical::FreePage(p);
	}

	extern "C" void Syscall_MapVirtualAddress(uint64_t v, uint64_t p, uint64_t f)
	{
		MemoryManager::Virtual::MapAddress(v, p, f);
	}

	extern "C" void Syscall_UnmapVirtualAddress(uint64_t v)
	{
		MemoryManager::Virtual::UnMapAddress(v);
	}
}
}
}
