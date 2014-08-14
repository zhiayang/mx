// Process.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <stdint.h>

namespace Library
{
	namespace SystemCall
	{
		void Yield()
		{
			asm volatile("int $0xF7");
		}

		void Block()
		{
			asm volatile("mov $0x29, %%r10; int $0xF8" ::: "%r10");
			Yield();
		}

		void Sleep(uint64_t ms)
		{
			int64_t t = -ms;
			asm volatile("mov $0x6, %%r10; mov %[c], %%rdi; int $0xF8" :: [c]"r"(t) : "%r10");
			Yield();
		}

		void CreateThread(void (*t)())
		{
			asm volatile("mov $0xC, %%r10; mov %[c], %%rdi; int $0xF8" :: [c]"r"(t) : "%r10");
		}

		void SpawnProcess(const char* path, const char* name)
		{
			asm volatile("mov $0x2B, %%r10; mov %[ex], %%rdi; mov %[pn], %%rsi; int $0xF8" :: [ex]"r"(path), [pn]"r"(name): "%r10", "%rdi", "%rsi");
		}

		uint64_t MMap_Anonymous(uint64_t addr, uint64_t size, uint64_t prot, uint64_t flags)
		{
			uint64_t ret = 0;
			asm volatile("mov $0x37, %%r10; mov %[ad], %%rdi; mov %[sz], %%rsi; mov %[pt], %%rdx; mov %[fl], %%rcx; int $0xF8; mov %%rax, %[r]"
				: [r]"=r"(ret) : [ad]"r"(addr), [sz]"r"((uint64_t) size), [pt]"r"(prot), [fl]"r"(flags) : "%r10", "%rdi", "%rsi", "%rdx", "%rcx");

			return ret;
		}

		uint64_t MMap_File(uint64_t fd, uint64_t length, uint64_t offset, uint64_t prot, uint64_t flags)
		{
			(void) fd;
			(void) length;
			(void) offset;
			(void) prot;
			(void) flags;

			return 0;
		}

		uint64_t GetPID()
		{
			uint64_t ret = 0;
			asm volatile("mov $0x3E, %%r10; int $0xF8; mov %%rax, %[re]" : [re]"=r"(ret) :: "%r10");
			return ret;
		}

		uint64_t GetParentPID()
		{
			uint64_t ret = 0;
			asm volatile("mov $0x3F, %%r10; int $0xF8; mov %%rax, %[re]" : [re]"=r"(ret) :: "%r10");
			return ret;
		}
	}
}
