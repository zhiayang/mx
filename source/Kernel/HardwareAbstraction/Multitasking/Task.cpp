// Task.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <HardwareAbstraction/Multitasking.hpp>
#include <HardwareAbstraction/MemoryManager.hpp>
#include <String.hpp>

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
	bool isfork = false;

	static void SetupThreadRegs(Thread* thread, uint64_t* stack, Thread_attr* attr)
	{
		*--stack = attr->regs.r15;													// R15 (-48)
		*--stack = attr->regs.r14;													// R14 (-56)
		*--stack = attr->regs.r13;													// R13 (-64)
		*--stack = attr->regs.r12;													// R12 (-72)
		*--stack = attr->regs.r11;													// R11 (-80)
		*--stack = attr->regs.r10;													// R10 (-88)
		*--stack = (uint64_t) (attr->a6 ? attr->a6 : (void*) attr->regs.r9);		// R9 (-96)
		*--stack = (uint64_t) (attr->a5 ? attr->a5 : (void*) attr->regs.r8);		// R8 (-104)

		*--stack = (uint64_t) (attr->a3 ? attr->a3 : (void*) attr->regs.rdx);		// RDX (-112)
		*--stack = (uint64_t) (attr->a4 ? attr->a4 : (void*) attr->regs.rcx);		// RCX (-120)
		*--stack = attr->regs.rbx;													// RBX (-128)
		*--stack = attr->regs.rax;													// RAX (-136)

		*--stack = attr->regs.rbp;													// RBP (-144)
		*--stack = (uint64_t) (attr->a2 ? attr->a2 : (void*) attr->regs.rsi);		// RSI (-152)	(argv)
		*--stack = (uint64_t) (attr->a1 ? attr->a1 : (void*) attr->regs.rdi);		// RDI (-160)	(argc)

		thread->StackPointer = (uint64_t) stack;
	}

	static void SetupStackThread_Kern(Thread* thread, uint64_t u, uint64_t f, Thread_attr* attr)
	{
		uint64_t* stack = (uint64_t*) thread->StackPointer;

		// need to insert a return statement here that will call the killthread function.
		uint64_t* usp = (uint64_t*) (u + DefaultRing3StackSize - 8);
		*usp = (uint64_t) Multitasking::ExitThread;

		// kernel thread
		*--stack = 0x10;															// SS (-8)
		*--stack = u + DefaultRing3StackSize - 8;									// User stack pointer (-16)
		*--stack = 0x202;															// RFLAGS (-24)
		*--stack = 0x08;															// CS (-32)
		*--stack = (uint64_t) f;													// RIP (-40)

		SetupThreadRegs(thread, stack, attr);
	}

	static void SetupStackThread_Proc(Thread* thread, uint64_t u, uint64_t physu, uint64_t stacksize, uint64_t f, Thread_attr* attr)
	{
		uint64_t* stack = (uint64_t*) thread->StackPointer;

		if(stacksize == 0)
			stacksize = DefaultRing3StackSize;

		// need to insert a return statement here that will call the killthread function.
		// it's kinda dangerous, because we're jumping directly to kernel code
		uint64_t* usp = (uint64_t*) (u + stacksize - 8);

		{
			Virtual::MapAddress(TemporaryVirtualMapping, (physu + stacksize - 8) & I_AlignMask, 0x07);

			uint64_t uspv = (uint64_t) usp;
			uint64_t* tmpusp = (uint64_t*) (TemporaryVirtualMapping + (uspv - (uspv & I_AlignMask)));

			*tmpusp = (uint64_t) Multitasking::ExitThread_Userspace;
			Virtual::UnmapAddress(TemporaryVirtualMapping);
		}

		{
			// user thread (always)
			*--stack = 0x23;															// SS
			*--stack = u + stacksize - 8;												// User stack pointer
			*--stack = 0x202;															// RFLAGS
			*--stack = 0x1B;															// CS
			*--stack = (uint64_t) f;													// RIP (-40)

			SetupThreadRegs(thread, stack, attr);
		}
	}

	static void SetupStackThread(Thread* thread, uint64_t u, uint64_t physu, uint64_t us, uint64_t f, Thread_attr* attr)
	{
		if(thread->Parent == Kernel::KernelProcess)
			SetupStackThread_Kern(thread, u, f, attr);

		else
			SetupStackThread_Proc(thread, u, physu, us, f, attr);
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
		DisableScheduler();
		Thread* thread = new Thread();
		Thread_attr* attr = 0;

		// allocate kernel stack.
		uint64_t k = 0;
		if((uint64_t) Parent->VAS->PML4 != GetKernelCR3())
		{
			auto pk = Physical::AllocatePage(DefaultRing3StackSize / 0x1000);
			k = Virtual::AllocateVirtual(DefaultRing3StackSize / 0x1000, 0, Parent->VAS, pk);

			Virtual::MapRegion(k, pk, DefaultRing3StackSize / 0x1000, 0x07, Parent->VAS->PML4);

			if(Parent->VAS->PML4 != Virtual::GetCurrentPML4T())
				Virtual::MapRegion(k, pk, DefaultRing3StackSize / 0x1000, 0x03);
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
		uint64_t physu = 0;
		if(attr->stackptr == 0 || attr->stacksize == 0)
		{
			physu = Physical::AllocatePage(DefaultRing3StackSize / 0x1000);
			u = Virtual::AllocateVirtual(DefaultRing3StackSize / 0x1000, 0, Parent->VAS, physu);
			us = DefaultRing3StackSize;

			Virtual::MapRegion(u, physu, DefaultRing3StackSize / 0x1000, 0x07, Parent->VAS->PML4);
		}
		else
		{
			u = attr->stackptr;
			us = attr->stacksize;
			HALT("Unsupported");
		}

		thread->StackSize			= DefaultRing3StackSize;
		thread->TopOfStack			= k + us;
		thread->StackPointer		= thread->TopOfStack;
		thread->Thread				= Function;
		thread->State				= STATE_NORMAL;
		thread->ThreadID			= NumThreads;
		thread->Parent				= Parent;
		thread->Priority			= attr->priority;
		thread->ExecutionTime		= 0;
		thread->tlsptr				= new uint8_t[Parent->tlssize];
		thread->CrashState			= new ThreadRegisterState_type;
		thread->flags				= Parent->Flags;
		thread->currenterrno		= 0;

		Parent->Threads.push_back(thread);

		SetupStackThread(thread, u, physu, us, (uint64_t) Function, attr);
		NumThreads++;

		// unmap
		if(Parent->VAS->PML4 != Virtual::GetCurrentPML4T())
			Virtual::UnmapRegion(k, DefaultRing3StackSize / 0x1000);

		if(FirstProc)
		{
			// set tss
			asm volatile("movq %[stacktop], 0x2504" :: [stacktop]"r"(thread->TopOfStack));
		}

		Log("Created Thread: Parent: %s, TID: %d", Parent->Name, thread->ThreadID);
		if(oattr == nullptr)
			delete attr;

		EnableScheduler();
		return thread;
	}

	Thread* CloneThread(Thread* orig)
	{
		Thread* ret = new Thread();
		ret->ThreadID		= NumThreads, NumThreads++;
		ret->StackPointer	= orig->StackPointer;
		ret->TopOfStack		= orig->TopOfStack;
		ret->StackSize		= orig->StackSize;
		ret->State			= orig->State;
		ret->Sleep			= orig->Sleep;
		ret->Priority		= orig->Priority;
		ret->flags			= orig->flags;
		ret->ExecutionTime	= orig->ExecutionTime;
		ret->Parent			= orig->Parent;
		ret->currenterrno	= orig->currenterrno;
		ret->returnval		= orig->returnval;
		ret->Thread			= orig->Thread;
		ret->CrashState		= new ThreadRegisterState_type;
		ret->tlsptr			= new uint8_t[orig->Parent->tlssize];

		return ret;
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
		DisableScheduler();
		Process* process = new Process();

		PageMapStructure* PML4 = FirstProc ? (PageMapStructure*)(GetKernelCR3()) : (PageMapStructure*) Virtual::CreateVAS();

		// everybody needs some tls
		if(tlssize == 0)
			tlssize = 8;

		process->Flags					= Flags;
		process->ProcessID				= NumProcesses;
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

		String::Copy(process->Name, name);


		NumProcesses++;
		(void) CreateThread(process, Function, Priority, a1, a2, a3, a4, a5, a6);

		if(FirstProc)
			FirstProc = false;

		else
		{
			// setup the descriptors.
			// manually.
			using namespace Filesystems::VFS;
			OpenFile(process->iocontext, "/dev/stdin", 0);
			OpenFile(process->iocontext, "/dev/stdout", 0);
			OpenFile(process->iocontext, "/dev/stderr", 0);
			OpenFile(process->iocontext, "/dev/stdlog", 0);
		}

		Log("Creating new process in VAS (%s): CR3(phys): %x, PID %d", name, (uint64_t) PML4, process->ProcessID);
		EnableScheduler();
		return process;
	}


	// copy all the mappings to the current space.
	// in fact, just copy the entire VAS map set + bookkeeping.
	// the only thread shall be the current thread.
	// child proc gets ret = 0, parent proc gets pid of child.
	// -1 on error.
	Process* ForkProcess(const char name[64], Thread_attr* attr)
	{
		DisableScheduler();
		using namespace Kernel::HardwareAbstraction::MemoryManager::Virtual;

		Process* proc = new Process();
		Thread* curthr = GetCurrentThread();
		PageMapStructure* PML4 = (PageMapStructure*) Virtual::CreateVAS();

		proc->Parent				= Multitasking::GetCurrentProcess();
		proc->Flags					= proc->Parent->Flags;
		proc->ProcessID				= NumProcesses;
		proc->VAS					= new Virtual::VirtualAddressSpace(PML4);
		proc->SignalHandlers		= (sighandler_t*) KernelHeap::AllocateChunk(sizeof(sighandler_t) * __SIGCOUNT);
		proc->iocontext				= new Filesystems::IOContext();
		proc->tlssize				= proc->Parent->tlssize;

		for(int i = 0; i < __SIGCOUNT; i++)
			proc->SignalHandlers[i] = __signal_ignore;

		Virtual::CopyVAS(proc->Parent->VAS, proc->VAS);
		String::Copy(proc->Name, name);

		NumProcesses++;
		isfork = true;

		// copy the thread.
		Thread* newt = CloneThread(curthr);
		newt->Parent = proc;
		proc->Threads.push_back(newt);


		// hacky? maybe.
		// works? not really
		newt->StackPointer = newt->TopOfStack - 160;
		// Utilities::StackDump((uint64_t*) newt->StackPointer, 20);
		// *((uint64_t*) (newt->StackPointer + 24)) = 0;

		// setup the descriptors.
		// manually.
		// sorry.
		using namespace Filesystems::VFS;
		OpenFile(proc->iocontext, "/dev/stdin", 0);
		OpenFile(proc->iocontext, "/dev/stdout", 0);
		OpenFile(proc->iocontext, "/dev/stderr", 0);
		OpenFile(proc->iocontext, "/dev/stdlog", 0);

		Log("Forking process from PID %d, new PID %d, CR3 %x", proc->Parent->ProcessID, proc->ProcessID, proc->VAS->PML4);
		EnableScheduler();
		return proc;
	}

	extern "C" int64_t Syscall_ForkProcess()
	{
		Process* proc = ForkProcess("knife", 0);
		// Multitasking::AddToQueue(proc);

		return proc->ProcessID;
	}
}
}
}











