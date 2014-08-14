// IPC.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <stdint.h>
#include "../syscall.h"

namespace Library
{
	namespace SystemCall
	{
		void SendMemoryMessageToProcess(uint64_t TargetPID, MessageTypes Type, void* Data, uint64_t DataSize, void (*Callback)())
		{
			asm volatile("mov $0x22, %%r10; mov %[tpid], %%rdi; mov %[tp], %%rsi; mov %[dat], %%rdx; mov %[sz], %%rcx; mov %[c], %%r8; int $0xF8" :: [tpid]"r"(TargetPID), [tp]"r"((uint64_t) Type), [dat]"r"(Data), [sz]"r"(DataSize), [c]"r"((uint64_t) Callback) : "%r10", "%rdi", "%rsi", "%rdx", "%rcx", "%r8");
		}

		void SendSimpleMessageToProcess(uint64_t TargetPID, MessageTypes typ, uint64_t D1, uint64_t D2, uint64_t D3, void (*cb)())
		{
			asm volatile("mov $0x24, %%r10; mov %[tpid], %%rdi; mov %[tp], %%rsi; mov %[d1], %%rdx; mov %[d2], %%rcx; mov %[d3], %%r8; mov %[c], %%r9; int $0xF8" :: [tpid]"r"(TargetPID), [tp]"r"((uint64_t) typ), [d1]"r"(D1), [d2]"r"(D2), [d3]"r"(D3), [c]"r"((uint64_t) cb) : "%r10", "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9");
		}

		SimpleMessage* GetSimpleMessage()
		{
			uint64_t ret = 0;
			asm volatile("mov $0x27, %%r10; int $0xF8; mov %%rax, %[r]" : [r]"=r"(ret) :: "%r10");
			return (SimpleMessage*) ret;
		}

		sighandler_t InstallSignalHandler(uint64_t signum, sighandler_t handler)
		{
			uint64_t ret = 0;
			asm volatile("mov $0x3D, %%r10; mov %[p], %%rdi; mov %[m], %%rsi; int $0xF8; mov %%rax, %[r]" : [r]"=r"(ret)
				: [p]"r"(signum), [m]"r"(handler) : "%r10", "%rdi", "%rsi");

			return (sighandler_t) ret;
		}
	}
}
