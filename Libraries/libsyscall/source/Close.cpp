// Close.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <stdint.h>

namespace Library
{
	namespace SystemCall
	{
		void CloseAny(uint64_t fd)
		{
			asm volatile("mov $0x3C, %%r10; mov %[f], %%rdi; int $0xF8" :: [f]"r"(fd) : "%r10", "%rdi");
		}

		void CloseIPCSocket(uint64_t sd)
		{
			CloseAny(sd);
		}

		void CloseFolder(uint64_t fd)
		{
			CloseAny(fd);
		}

		void CloseFile(uint64_t fd)
		{
			CloseAny(fd);
		}
	}
}
