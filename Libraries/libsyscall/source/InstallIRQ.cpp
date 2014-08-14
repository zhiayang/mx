// InstallIRQ.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <stdint.h>

namespace Library
{
	namespace SystemCall
	{
		void InstallIRQHandlerWithRegs(uint64_t irq, uint64_t handleraddr)
		{
			asm volatile("mov $0x9, %%r10; mov %[c], %%rdi; mov %[h], %%rsi; int $0xF8" :: [c]"r"(irq), [h]"r"(handleraddr) : "%r10");
		}

		void InstallIRQHandler(uint64_t irq, uint64_t handleraddr)
		{
			asm volatile("mov $0xA, %%r10; mov %[c], %%rdi; mov %[h], %%rsi; int $0xF8" :: [c]"r"(irq), [h]"r"(handleraddr) : "%r10");
		}
	}
}
