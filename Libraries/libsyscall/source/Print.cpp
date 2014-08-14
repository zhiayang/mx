// Print.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <stdint.h>

namespace Library
{
	namespace SystemCall
	{
		void PrintChar(uint8_t ch)
		{
			asm volatile("mov $0x1, %%r10; mov %[c], %%rdi; int $0xF8" :: [c]"g"((uint64_t) ch) : "%r10");
		}

		void PrintString(const char* str)
		{
			asm volatile("mov $0x2, %%r10; mov %[c], %%rdi; int $0xF8" :: [c]"g"((uint64_t) str) : "%r10");
		}
	}
}
