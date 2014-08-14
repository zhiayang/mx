// MiscFileIO.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <stdint.h>
namespace Library
{
	namespace SystemCall
	{
		void* ListObjects(uint64_t fd, void* output, uint64_t* items)
		{
			uint64_t ret = 0;
			asm volatile("mov $0x1E, %%r10; mov %[f], %%rdi; mov %[o], %%rsi; mov %[i], %%rdx; int $0xF8; mov %%rax, %[r]" : [r]"=r"(ret) : [f]"r"(fd), [o]"r"((uint64_t) output), [i]"r"((uint64_t) items) : "%r10", "%rdi", "%rsi", "%rdx");

			return (void*) ret;
		}

		uint64_t GetFileSize(uint64_t fd)
		{
			uint64_t ret = 0;
			asm volatile("mov $0x1F, %%r10; mov %[f], %%rdi; int $0xF8; mov %%rax, %[r]" : [r]"=r"(ret) : [f]"r"(fd) : "%r10", "%rdi");
			return ret;
		}

		bool CheckFileExistence(const char* path)
		{
			uint64_t ret = 0;
			asm volatile("mov $0x20, %%r10; mov %[p], %%rdi; int $0xF8; mov %%rax, %[r]" : [r]"=r"(ret) : [p]"r"(path) : "%r10", "%rdi");
			return (bool) ret;
		}

		bool CheckFolderExistence(const char* path)
		{
			uint64_t ret = 0;
			asm volatile("mov $0x21, %%r10; mov %[p], %%rdi; int $0xF8; mov %%rax, %[r]" : [r]"=r"(ret) : [p]"r"(path) : "%r10", "%rdi");
			return (bool) ret;
		}

	}
}
