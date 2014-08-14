// Write.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <stdint.h>
namespace Library
{
	namespace SystemCall
	{
		uint64_t WriteToAny(uint64_t sd, uint8_t* buffer, uint64_t length)
		{
			uint64_t ret = 0;
			asm volatile("mov $0x3A, %%r10; mov %[f], %%rdi; mov %[b], %%rsi; mov %[l], %%rdx; int $0xF8; mov %%rax, %[r]" : [r]"=r"(ret) : [f]"r"(sd), [b]"r"((uint64_t) buffer), [l]"r"(length) : "%r10", "%rdi", "%rsi", "%rdx");

			return ret;
		}

		uint8_t* WriteFile(uint64_t fd, uint8_t* buffer, uint64_t length)
		{
			WriteToAny(fd, buffer, length);
			return buffer;
		}

		uint8_t* WriteIPCSocket(uint64_t sd, uint8_t* buffer, uint64_t length)
		{
			WriteToAny(sd, buffer, length);
			return buffer;
		}
	}
}
