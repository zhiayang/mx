// OperatorNew.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <stddef.h>
#include <string.h>
#include <HardwareAbstraction/MemoryManager/KernelHeap.hpp>

static const char* __file__ = "?";
static const char* __line__ = "0";

using namespace Kernel::HardwareAbstraction::MemoryManager;
void operator delete(void* p) noexcept
{
	KernelHeap::FreeChunk(p);
}

void operator delete[](void* p) noexcept
{
	KernelHeap::FreeChunk(p);
}


const char* __set_file(const char* f)
{
	__file__ = f;
	return __file__;
}

const char* __set_line(const char* l)
{
	__line__ = l;
	return __line__;
}

#define DEBUG_NEW 0
void* operator new(size_t size)
{
	#if DEBUG_NEW
	{
		char final[256] = { 0 };
		strcat(final, __file__);
		strcat(final, ":");
		strcat(final, __line__);

		return (void*) KernelHeap::AllocateChunk(size, final);
	}
	#else
	{
		return (void*) KernelHeap::AllocateFromHeap(size);
	}
	#endif
}

void* operator new[](size_t size)
{
	#if DEBUG_NEW
	{
		char final[256] = { 0 };
		strcat(final, __file__);
		strcat(final, ":");
		strcat(final, __line__);

		return (void*) KernelHeap::AllocateChunk(size, final);
	}
	#else
	{
		return (void*) KernelHeap::AllocateFromHeap(size);
	}
	#endif
}









