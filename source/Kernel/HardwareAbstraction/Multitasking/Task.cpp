// Task.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <HardwareAbstraction/Multitasking.hpp>
#include <HardwareAbstraction/MemoryManager.hpp>
#include <List.hpp>
#include <string.h>
#include "../IPC/Dispatchers/CentralDispatch.hpp"

using namespace Kernel;
using namespace Kernel::HardwareAbstraction::MemoryManager;
using namespace Library;

static bool FirstProc = true;

namespace Kernel {
namespace HardwareAbstraction {
namespace Multitasking
{
	#define __signal_ignore	0
	uint64_t NumThreads = 0;
	uint64_t NumProcesses = 0;

	static void SetupStackThread_Kern(Thread* thread, uint64_t u, uint64_t f, void* p1, void* p2, void* p3, void* p4, void* p5, void* p6)
	{
		uint64_t* stack = (uint64_t*) thread->StackPointer;

		// need to insert a return statement here that will call the killthread function.
		uint64_t* usp = (uint64_t*) (u + DefaultRing3StackSize - 8);
		*usp = (uint64_t) Multitasking::ExitThread;

		// kernel thread
		*--stack = 0x10;					// SS (-8)
		*--stack = u + DefaultRing3StackSize - 8;	// User stack pointer (-16)
		*--stack = 0x202;				// RFLAGS (-24)
		*--stack = 0x08;					// CS (-32)
		*--stack = (uint64_t) f;				// RIP (-40)

		*--stack = 0;					// R15 (-48)
		*--stack = 0;					// R14 (-56)
		*--stack = 0;					// R13 (-64)
		*--stack = 0;					// R12 (-72)
		*--stack = 0;					// R11 (-80)
		*--stack = 0;					// R10 (-88)
		*--stack = (uint64_t) p6;			// R9 (-96)
		*--stack = (uint64_t) p5;			// R8 (-104)

		*--stack = (uint64_t) p3;			// RDX (-112)
		*--stack = (uint64_t) p4;			// RCX (-120)
		*--stack = 0;					// RBX (-128)
		*--stack = 0;					// RAX (-136)

		*--stack = 0;					// RBP (-144)
		*--stack = (uint64_t) p2;			// RSI (-152)
		*--stack = (uint64_t) p1;			// RDI (-160)

		thread->StackPointer = (uint64_t) stack;
	}

	static void SetupStackThread_Proc(Thread* thread, uint64_t u, uint64_t stacksize, uint64_t f, void* p1, void* p2, void* p3, void* p4, void* p5, void* p6)
	{
		uint64_t* stack = (uint64_t*) thread->StackPointer;
		// put argv* and envp* on stack.

		if(stacksize == 0)
			stacksize = DefaultRing3StackSize;

		// need to insert a return statement here that will call the killthread function.
		// it's kinda dangerous, because we're jumping directly to kernel code
		uint64_t* usp = (uint64_t*) (u + stacksize - 8);
		*usp = (uint64_t) Multitasking::ExitThread_Userspace;

		// user thread (always)
		*--stack = 0x23;					// SS
		*--stack = u + (stacksize) - 8;			// User stack pointer
		*--stack = 0x202;				// RFLAGS
		*--stack = 0x1B;					// CS
		*--stack = (uint64_t) f;				// RIP (-40)

		*--stack = 0;					// R15 (-48)
		*--stack = 0;					// R14 (-56)
		*--stack = 0;					// R13 (-64)
		*--stack = 0;					// R12 (-72)
		*--stack = 0;					// R11 (-80)
		*--stack = 0;					// R10 (-88)
		*--stack = (uint64_t) p5;			// R9 (-96)
		*--stack = (uint64_t) p6;			// R8 (-104)

		*--stack = (uint64_t) p3;			// RDX (-112)
		*--stack = (uint64_t) p4;			// RCX (-120)
		*--stack = 0;					// RBX (-128)
		*--stack = 0;					// RAX (-136)

		*--stack = 0;					// RBP (-144)
		*--stack = (uint64_t) p2;			// RSI (-152)	(argv)
		*--stack = (uint64_t) p1;			// RDI (-160)	(argc)

		thread->StackPointer = (uint64_t) stack;
	}

	static void SetupStackThread(Thread* thread, uint64_t u, uint64_t us, uint64_t f, void* p1, void* p2, void* p3, void* p4, void* p5, void* p6)
	{
		if(thread->Parent == Kernel::KernelProcess)
			SetupStackThread_Kern(thread, u, f, p1, p2, p3, p4, p5, p6);

		else
			SetupStackThread_Proc(thread, u, us, f, p1, p2, p3, p4, p5, p6);
	}

	Thread* CreateThread(Process* Parent, void (*Function)(), uint8_t Priority, void* p1, void* p2, void* p3, void* p4, void* p5, void* p6)
	{
		Thread_attr a;
		a.a1 = p1;
		a.a2 = p2;
		a.a3 = p3;
		a.a4 = p4;
		a.a5 = p5;
		a.a6 = p6;

		a.priority = Priority;
		Memory::Set(&a.regs, 0, sizeof(a.regs));
		a.stackptr = 0;
		a.stacksize = 0;

		return CreateThread(Parent, Function, &a);
	}

	Thread* CreateThread(Process* Parent, void (*Function)(), Thread_attr* oattr)
	{
		Thread* thread = new Thread();
		Thread_attr* attr = 0;

		uint64_t k = 0;
		if(Parent->CR3 != GetKernelCR3())
		{
			k = Virtual::AllocateVirtual(DefaultRing3StackSize / 0x1000, 0, Parent->VAS);
			auto pk = Physical::AllocatePage(DefaultRing3StackSize / 0x1000);
			Virtual::MapRegion(k, pk, DefaultRing3StackSize / 0x1000, 0x3, Parent->VAS->PML4);
			Virtual::MapRegion(k, pk, DefaultRing3StackSize / 0x1000, 0x3);
		}
		else
		{
			k = Virtual::AllocatePage(DefaultRing3StackSize / 0x1000);
		}

		// check.
		if(oattr == nullptr)
		{
			attr = new Thread_attr;
			Memory::Set(attr, 0, sizeof(Thread_attr));
		}
		else
			attr = oattr;

		// allocate user stack.
		uint64_t u = 0;
		uint64_t us = 0;
		if(attr->stackptr == 0 || attr->stacksize == 0)
		{
			uint64_t pu = Physical::AllocatePage(DefaultRing3StackSize / 0x1000);
			u = Virtual::AllocateVirtual(DefaultRing3StackSize / 0x1000, 0, Parent->VAS);
			us = DefaultRing3StackSize;

			Virtual::MapRegion(u, pu, DefaultRing3StackSize / 0x1000, 0x7, (Virtual::PageMapStructure*) Parent->CR3);
		}
		else
		{
			u = attr->stackptr;
			us = attr->stacksize;
		}

		thread->TopOfStack			= k + us;
		thread->StackPointer			= thread->TopOfStack;
		thread->Thread				= Function;
		thread->State				= STATE_NORMAL;
		thread->ThreadID			= NumThreads;
		thread->Parent				= Parent;
		thread->messagequeue			= new rde::list<uintptr_t>();
		thread->Priority				= attr->priority;
		thread->ExecutionTime			= 0;
		thread->InstructionPointer		= 0;
		thread->tlsptr				= new uint8_t[Parent->tlssize];
		thread->CrashState			= new ThreadRegisterState_type;
		thread->flags				= Parent->Flags;

		Parent->Threads->InsertFront(thread);

		SetupStackThread(thread, u, us, (uint64_t) Function, attr->a1, attr->a2, attr->a3, attr->a4, attr->a5, attr->a6);
		NumThreads++;

		// unmap
		if(Parent->CR3 != (uint64_t) Virtual::GetCurrentPML4T())
			Virtual::UnMapRegion(k, DefaultRing3StackSize / 0x1000);

		if(FirstProc)
		{
			// set tss
			asm volatile("movq %[stacktop], 0x2504" :: [stacktop]"r"(thread->TopOfStack));
		}

		Log("Created Thread: Parent: %s, TID: %d", Parent->Name, thread->ThreadID);
		if(oattr == nullptr)
			delete attr;

		return thread;
	}



	Thread* CreateKernelThread(void (*Function)(), uint8_t Priority, void* p1, void* p2, void* p3, void* p4, void* p5, void* p6)
	{
		return CreateThread(Kernel::KernelProcess, Function, Priority, p1, p2, p3, p4, p5, p6);
	}

	Process* CreateProcess(const char name[64], uint8_t Flags, void (*Function)())
	{
		return CreateProcess(name, Flags, 0, Function, 1, 0, 0, 0, 0, 0, 0);
	}

	Process* CreateProcess(const char name[64], uint8_t Flags, void (*Function)(), uint8_t Priority, void* a1, void* a2, void* a3, void* a4, void* a5, void* a6)
	{
		return CreateProcess(name, Flags, 0, Function, Priority, a1, a2, a3, a4, a5, a6);
	}

	Process* CreateProcess(const char name[64], uint8_t Flags, uint64_t tlssize, void (*Function)(), uint8_t Priority, void* a1, void* a2, void* a3, void* a4, void* a5, void* a6)
	{
		using namespace Kernel::HardwareAbstraction::MemoryManager::Virtual;
		using Library::LinkedList;

		Process* process = new Process();
		process->Threads = new LinkedList<Thread>();

		PageMapStructure* PML4 = FirstProc ? (PageMapStructure*)(GetKernelCR3()) : (PageMapStructure*) Virtual::CreateVAS();

		process->Flags					= Flags;
		process->ProcessID				= NumProcesses;
		process->CR3					= (uint64_t) PML4;
		process->AllocatedPageList			= new Library::Vector<uint64_t>();
		process->VAS					= new Virtual::VirtualAddressSpace(PML4);
		process->SignalHandlers			= (sighandler_t*) KernelHeap::AllocateChunk(sizeof(sighandler_t) * __SIGCOUNT);
		process->iocontext				= new Filesystems::IOContext();
		process->tlssize				= tlssize;
		for(int i = 0; i < __SIGCOUNT; i++)
			process->SignalHandlers[i] = __signal_ignore;

		Virtual::SetupVAS(process->VAS);

		if(!FirstProc)
		{
			process->Parent = Multitasking::GetCurrentProcess();
		}
		else
		{
			process->Parent = 0;
			Kernel::KernelProcess = process;
		}

		strncpy(process->Name, name, 64);


		NumProcesses++;

		(void) CreateThread(process, Function, Priority, a1, a2, a3, a4, a5, a6);

		if(FirstProc)
			FirstProc = false;

		else
		{
			// setup the descriptors.
			// manually.
			using namespace Filesystems::VFS;
			assert(OpenFile(process->iocontext, "/dev/stdin", 0)->fd == 0);
			assert(OpenFile(process->iocontext, "/dev/stdout", 0)->fd == 1);
		}

		Log("Creating new process in VAS (%s): CR3(phys): %x, PID %d", name, (uint64_t) PML4, process->ProcessID);
		return process;
	}
}
}
}
















