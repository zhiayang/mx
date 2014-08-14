// Virtual.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <stdint.h>

namespace Library
{
	namespace SystemCall
	{
		void MapVirtualAddress(uint64_t v, uint64_t p, uint64_t f)
		{
			asm volatile("mov $0x11, %%r10; mov %[vt], %%rdi; mov %[pt], %%rsi; mov %[ft], %%rdx; int $0xF8" :: [vt]"r"(v), [pt]"r"(p), [ft]"r"(f) : "%r10");
		}

		void UnmapVirtualAddress(uint64_t v)
		{
			asm volatile("mov $0x12, %%r10; mov %[vt], %%rdi; int $0xF8" :: [vt]"r"(v) : "%r10");
		}
	}
}
