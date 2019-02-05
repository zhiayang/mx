// Task.cpp
// Copyright (c) 2013 - 2016, zhiayang@gmail.com
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

	static uint64_t* SetupThreadRegs(Thread* thread, uint64_t* stack, Thread_attr* attr)
	{
		(void) thread;

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

		return stack;
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

		thread->StackPointer = (uint64_t) SetupThreadRegs(thread, stack, attr);
	}

	static void SetupStackThread_Proc(Thread* thread, uint64_t u, uint64_t physu, uint64_t physks, uint64_t stacksize, uint64_t f, Thread_attr* attr)
	{
		uint64_t* stack = (uint64_t*) thread->StackPointer;

		if(stacksize == 0)
			stacksize = DefaultRing3StackSize;

		// need to insert a return statement here that will call the killthread function.
		// it's kinda dangerous, because we're jumping directly to kernel code
		uint64_t* usp = (uint64_t*) (u + stacksize - 8);

		// probably not in the same address space anyway
		uint64_t oldstack = (uint64_t) stack;
		physks += (stacksize - 0x1000);

		{
			Virtual::MapAddress(TemporaryVirtualMapping, (physu + stacksize - 8) & I_AlignMask, 0x07);

			uint64_t uspv = (uint64_t) usp;
			uint64_t* tmpusp = (uint64_t*) (TemporaryVirtualMapping + (uspv - (uspv & I_AlignMask)));

			*tmpusp = (uint64_t) Multitasking::ExitThread_Userspace;
			Virtual::UnmapAddress(TemporaryVirtualMapping);


			Virtual::MapAddress(TemporaryVirtualMapping + oldstack - 0x1000, physks, 0x07);
			stack = (uint64_t*) (TemporaryVirtualMapping + (uint64_t) oldstack);
		}

		{
			// user thread (always)
			*--stack = 0x23;															// SS
			*--stack = u + stacksize - 8;												// User stack pointer
			*--stack = 0x202;															// RFLAGS
			*--stack = 0x1B;															// CS
			*--stack = (uint64_t) f;													// RIP (-40)

			thread->StackPointer = (uint64_t) SetupThreadRegs(thread, stack, attr) - TemporaryVirtualMapping;
			Virtual::UnmapAddress(TemporaryVirtualMapping + oldstack - 0x1000);
		}
	}

	static void SetupStackThread(Thread* thread, uint64_t u, uint64_t physu, uint64_t physks, uint64_t stacksz, uint64_t f, Thread_attr* attr)
	{
		if(thread->Parent == Kernel::KernelProcess)
		{
			SetupStackThread_Kern(thread, u, f, attr);
		}
		else
		{
			SetupStackThread_Proc(thread, u, physu, physks, stacksz, f, attr);
		}
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
		uint64_t physks = 0;
		if((uint64_t) Parent->VAS.PML4 != GetKernelCR3())
		{
			auto pk = Physical::AllocatePage(DefaultRing3StackSize / 0x1000);
			k = Virtual::AllocateVirtual(DefaultRing3StackSize / 0x1000, 0, &Parent->VAS, pk);

			Virtual::MapRegion(k, pk, DefaultRing3StackSize / 0x1000, 0x07, Parent->VAS.PML4);
			physks = pk;

		}
		else
		{
			k = Virtual::AllocatePage(DefaultRing3StackSize / 0x1000);
			physks = Virtual::GetVirtualPhysical(k);

			Log("kernel stack for tid %d is at %x, %x bytes", NumThreads, physks, DefaultRing3StackSize);
		}


		// check.
		if(oattr == nullptr)
		{
			attr = new Thread_attr;
			Memory::Set(attr, 0, sizeof(Thread_attr));
		}
		else
		{
			attr = oattr;
		}

		// allocate user stack.
		uint64_t u = 0;
		uint64_t ustacksz = 0;
		uint64_t physu = 0;
		if(attr->stackptr == 0 || attr->stacksize == 0)
		{
			physu = Physical::AllocatePage(DefaultRing3StackSize / 0x1000);
			u = Virtual::AllocateVirtual(DefaultRing3StackSize / 0x1000, 0, &Parent->VAS, physu);
			ustacksz = DefaultRing3StackSize;

			Virtual::MapRegion(u, physu, DefaultRing3StackSize / 0x1000, 0x07, Parent->VAS.PML4);
		}
		else
		{
			u = attr->stackptr;
			ustacksz = attr->stacksize;
			HALT("Unsupported");
		}

		thread->StackSize			= DefaultRing3StackSize;
		thread->TopOfStack			= k + ustacksz;
		thread->StackPointer		= thread->TopOfStack;
		thread->funcpointer			= Function;
		thread->State				= STATE_NORMAL;
		thread->ThreadID			= (pid_t) NumThreads, NumThreads++;
		thread->Parent				= Parent;
		thread->Priority			= attr->priority;
		thread->ExecutionTime		= 0;
		thread->tlsptr				= new uint8_t[Parent->tlssize];
		thread->CrashState			= new ThreadRegisterState_type;
		thread->flags				= Parent->Flags;
		thread->currenterrno		= 0;

		SetupStackThread(thread, u, physu, physks, ustacksz, (uint64_t) Function, attr);
		Parent->Threads.push_back(thread);

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

		PageMapStructure* PML4 = FirstProc ? (PageMapStructure*) (GetKernelCR3()) : (PageMapStructure*) Virtual::CreateVAS();
		Process* process = new Process(PML4);


		// everybody needs some tls
		if(tlssize == 0)
			tlssize = 8;

		process->Flags					= Flags;
		process->ProcessID				= (pid_t) NumProcesses;
		process->VAS					= Virtual::VirtualAddressSpace(PML4);
		process->SignalHandlers			= new sighandler_t[__SIGCOUNT];
		process->tlssize				= tlssize;
		for(int i = 0; i < __SIGCOUNT; i++)
			process->SignalHandlers[i] = __signal_ignore;

		Virtual::SetupVAS(&process->VAS);

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
		auto k = CreateThread(process, Function, Priority, a1, a2, a3, a4, a5, a6);
		(void) k;

		if(FirstProc)
			FirstProc = false;

		else
		{
			// setup the descriptors.
			// manually.
			using namespace Filesystems::VFS;
			OpenFile(&process->iocontext, "/dev/stdin", 0);
			OpenFile(&process->iocontext, "/dev/stdout", 0);
			OpenFile(&process->iocontext, "/dev/stderr", 0);
		}

		Log("Creating new process in VAS (%s): CR3(phys): %x, PID %d", name, (uint64_t) PML4, process->ProcessID);
		EnableScheduler();
		return process;
	}


	Thread* CloneThread(Thread* orig)
	{
		Thread* ret			= new Thread();
		ret->ThreadID		= (pid_t) NumThreads, NumThreads++;
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
		ret->funcpointer	= orig->funcpointer;
		ret->CrashState		= new ThreadRegisterState_type;
		ret->tlsptr			= new uint8_t[orig->Parent->tlssize];

		Memory::Copy(ret->tlsptr, orig->tlsptr, orig->Parent->tlssize);

		return ret;
	}



	// copy all the mappings to the current space.
	// in fact, just copy the entire VAS map set + bookkeeping.
	// the only thread shall be the current thread.
	// child proc gets ret = 0, parent proc gets pid of child.
	// -1 on error.
	Process* ForkProcess(const char name[64], Thread_attr* attr)
	{
		(void) attr;

		DisableScheduler();
		using namespace Kernel::HardwareAbstraction::MemoryManager::Virtual;

		Thread* curthr = GetCurrentThread();
		PageMapStructure* PML4 = (PageMapStructure*) Virtual::CreateVAS();
		Process* proc = new Process(PML4);

		proc->Parent				= Multitasking::GetCurrentProcess();
		proc->Flags					= proc->Parent->Flags;
		proc->ProcessID				= (pid_t) NumProcesses;
		proc->VAS					= Virtual::VirtualAddressSpace(PML4);
		proc->SignalHandlers		= new sighandler_t[__SIGCOUNT];
		proc->tlssize				= proc->Parent->tlssize;

		for(int i = 0; i < __SIGCOUNT; i++)
			proc->SignalHandlers[i] = __signal_ignore;

		// setup first
		Virtual::SetupVAS(&proc->VAS);
		proc->VAS.regions.clear();


		Virtual::CopyVAS(&proc->Parent->VAS, &proc->VAS);
		String::Copy(proc->Name, name);

		NumProcesses++;

		// copy the thread.
		Thread* newt = CloneThread(curthr);
		newt->Parent = proc;
		// assert(newt->StackPointer == newt->TopOfStack - 160);

		proc->Threads.push_back(newt);

		// hacky? maybe.
		// works? not really

		// setup the descriptors.
		// manually.
		// sorry.
		using namespace Filesystems::VFS;
		OpenFile(&proc->iocontext, "/dev/stdin", 0);
		OpenFile(&proc->iocontext, "/dev/stdout", 0);
		OpenFile(&proc->iocontext, "/dev/stderr", 0);

		Log("Forking process from PID %d, new PID %d, CR3 %x", proc->Parent->ProcessID, proc->ProcessID, proc->VAS.PML4);

		EnableScheduler();
		return proc;
	}



	// todo: zero reentrancy-safe.
	static uint64_t saved_rip = 0;
	static uint64_t saved_ss = 0;
	static uint64_t saved_cs = 0;
	static uint64_t saved_rfl = 0;
	static uint64_t saved_usp = 0;

	static ThreadRegisterState_type saved_registers;
	extern "C" void __syscall_internal_save_registers(uint64_t rsp)
	{
		uint64_t* stack = (uint64_t*) rsp;

		saved_registers.rdi	= *stack++;
		saved_registers.rsi	= *stack++;
		saved_registers.rbp	= *stack++;

		saved_registers.rax	= *stack++;
		saved_registers.rbx	= *stack++;
		saved_registers.rcx	= *stack++;
		saved_registers.rdx	= *stack++;

		saved_registers.r8	= *stack++;
		saved_registers.r9	= *stack++;
		saved_registers.r10	= *stack++;
		saved_registers.r11	= *stack++;
		saved_registers.r12	= *stack++;
		saved_registers.r13	= *stack++;
		saved_registers.r14	= *stack++;
		saved_registers.r15	= *stack++;

		// skip rbp pushed.
		stack++;

		saved_rip			= *stack++;
		saved_cs			= *stack++;
		saved_rfl			= *stack++;
		saved_usp			= *stack++;
		saved_ss			= *stack++;

		// Log("%p / %p / %p / %p / %p", saved_rip, saved_cs, saved_rfl, saved_usp, saved_ss);
	}

	extern "C" int64_t Syscall_ForkProcess()
	{
		Process* cur = GetCurrentProcess();
		Process* proc = ForkProcess(cur->Name, 0);

		// we access the saved registers here.
		// proc has the things set up, but we need to retroactively screw with the registers on stack.
		{
			// get a temporary mapping.
			uint64_t begin = proc->Threads.front()->StackPointer & ~((uint64_t) 0xFFF);
			uint64_t sz = 1;

			if(proc->Threads.front()->StackPointer + 160 > begin + 0x1000)
				sz = 2;

			// map... hopefully.
			uint64_t phys = Virtual::GetVirtualPhysical(begin, &proc->VAS);
			uint64_t virt = Virtual::AllocateVirtual(sz);

			Virtual::MapRegion(virt, phys, sz, 0x07);

			uint64_t stackptr = virt + (proc->Threads.front()->StackPointer - begin);

			uint64_t* stack = (uint64_t*) (stackptr + 160);

			*--stack = saved_ss;
			*--stack = saved_usp;
			*--stack = saved_rfl;
			*--stack = saved_cs;
			*--stack = saved_rip;

			*--stack = saved_registers.r15;		// R15 (-48)
			*--stack = saved_registers.r14;		// R14 (-56)
			*--stack = saved_registers.r13;		// R13 (-64)
			*--stack = saved_registers.r12;		// R12 (-72)
			*--stack = saved_registers.r11;		// R11 (-80)
			*--stack = saved_registers.r10;		// R10 (-88)
			*--stack = saved_registers.r9;		// R9 (-96)
			*--stack = saved_registers.r8;		// R8 (-104)

			*--stack = saved_registers.rdx;		// RDX (-112)
			*--stack = saved_registers.rcx;		// RCX (-120)
			*--stack = saved_registers.rbx;		// RBX (-128)
			*--stack = 0;						// RAX (-136)

			*--stack = saved_registers.rbp;		// RBP (-144)
			*--stack = saved_registers.rsi;		// RSI (-152)
			*--stack = saved_registers.rdi;		// RDI (-160)

			Virtual::UnmapRegion(virt, sz);
			Virtual::FreeVirtual(virt, sz);
		}
		// done.


		// since the child will never actually execute this part,
		// always return the child's PID, to the parent.

		assert(proc->ProcessID != cur->ProcessID);
		Multitasking::AddToQueue(proc);

		return (int64_t) proc->ProcessID;
	}
}
}
}











