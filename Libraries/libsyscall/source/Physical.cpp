// Physical.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <stdint.h>

namespace Library
{
	namespace SystemCall
	{
		uint64_t AllocatePage()
		{
			uint64_t ret = 0;
			asm volatile("mov $0x13, %%r10; int $0xF8; mov %%rax, %[r]" : [r]"=r"(ret) :: "%r10");
			return ret;
		}

		void FreePage(uint64_t p)
		{
			asm volatile("mov $0x14, %%r10; mov %[vt], %%rdi; int $0xF8" :: [vt]"r"(p) : "%r10");
		}
	}
}
