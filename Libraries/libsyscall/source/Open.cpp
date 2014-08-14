// Open.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <stdint.h>

namespace Library
{
	namespace SystemCall
	{
		uint64_t OpenFile(const char* path, uint8_t mode)
		{
			uint64_t ret = 0;
			asm volatile("mov $0x18, %%r10; mov %[p], %%rdi; mov %[m], %%rsi; int $0xF8; mov %%rax, %[r]" : [r]"=r"(ret) : [p]"r"(path), [m]"r"((uint64_t) mode) : "%r10", "%rdi", "%rsi");
			return ret;
		}

		uint64_t OpenFolder(const char* path)
		{
			uint64_t ret = 0;
			asm volatile("mov $0x1C, %%r10; mov %[p], %%rdi; int $0xF8; mov %%rax, %[r]" : [r]"=r"(ret) : [p]"r"(path) : "%r10", "%rdi");
			return ret;
		}

		void OpenIPCSocket(uint64_t pid, uint64_t size)
		{
			asm volatile("mov $0x33, %%r10; mov %[c], %%rdi; mov %[h], %%rsi; int $0xF8" :: [c]"r"(pid), [h]"r"(size) : "%r10", "%rdi", "%rsi");
		}

		uint64_t OpenAny(const char* path, uint64_t flags)
		{
			uint64_t ret = 0;
			asm volatile("mov $0x3B, %%r10; mov %[p], %%rdi; mov %[m], %%rsi; int $0xF8; mov %%rax, %[r]" : [r]"=r"(ret) : [p]"r"(path), [m]"r"(flags) : "%r10", "%rdi", "%rsi");
			return ret;
		}
	}
}
