// Process.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <HardwareAbstraction/LoadBinary.hpp>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-macros"
#define PROT_EXEC		(1 << 0)
#define PROT_WRITE		(1 << 1)
#define PROT_READ		(1 << 2)
#define PROT_USER		(PROT_EXEC | PROT_WRITE | PROT_READ)

#define MAP_SHARED		(1 << 0)
#define MAP_PRIVATE		(1 << 1)

#define MAP_ANONYMOUS	(1 << 2)
#define MAP_FIXED		(1 << 3)
#pragma clang diagnostic pop

namespace Kernel {
namespace HardwareAbstraction {
namespace SystemCalls
{
	extern "C" void Syscall_ExitProc(uint64_t retval)
	{
		UNUSED(retval);

		// TODO.
		// needs to kill the entire process chain.
		Multitasking::Process* proc = Multitasking::GetCurrentThread()->Parent;
		assert(proc->Threads.front());

		Multitasking::Kill(proc);
	}

	extern "C" uint64_t Syscall_MMapAnon(uint64_t addr, uint64_t size, uint64_t prot, uint64_t flags)
	{
		using namespace MemoryManager;
		// flags is one of MAP_*
		// prot is one of PROT_*

		uint64_t finalflag = 0x1;
		if(prot & PROT_WRITE)
			finalflag |= 0x2;

		if(prot & PROT_WRITE)
			;

		if(prot & PROT_EXEC)
			;

		if(flags & MAP_FIXED && addr == 0)
			return 0;

		finalflag |= 0x4;

		// fetch a physical page.
		size = (size + 0xFFF) / 0x1000;

		auto ret = Virtual::AllocatePage(size, addr, finalflag);
		return ret;
	}

	extern "C" void Syscall_Sleep(int64_t milliseconds)
	{
		Multitasking::Sleep(milliseconds);
	}

	extern "C" void Syscall_Yield()
	{
		YieldCPU();
	}

	extern "C" void Syscall_Block()
	{
		Multitasking::Block(1);
	}

	extern "C" uint64_t Syscall_CreateThread(uint64_t ptr, pthread_attr_t* attr)
	{
		void (*t)() = (void(*)())(ptr);
		Multitasking::Process* p = Multitasking::GetCurrentProcess();
		Multitasking::Thread* thr = 0;
		Multitasking::AddToQueue(thr = Multitasking::CreateThread(p, t, attr));
		return thr->ThreadID;
	}

	extern "C" pid_t Syscall_SpawnProcess(const char* ExecutableFilename, const char* ProcessName)
	{
		auto proc = LoadBinary::Load(ExecutableFilename, ProcessName);
		if(!proc)
			return 0;

		assert(proc);
		Multitasking::AddToQueue(proc);

		return proc->ProcessID;
	}

	extern "C" uint64_t Syscall_GetPID()
	{
		return Multitasking::GetCurrentProcess()->ProcessID;
	}

	extern "C" uint64_t Syscall_GetParentPID()
	{
		return Multitasking::GetCurrentProcess()->Parent->ProcessID;
	}
}
}
}




