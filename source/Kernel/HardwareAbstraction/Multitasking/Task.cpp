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

	static void SetupStackThread_Kern(Thread* thread, uint64_t u, uint64_t f, void* p1, void* p2, void* p3, void* p4, void* p5, void* p6)
	{
		uint64_t* stack = (uint64_t*) thread->StackPointer;

		// need to insert a return statement here that will call the killthread function.
		uint64_t* usp = (uint64_t*) (u + DefaultRing3StackSize - 8);
		*usp = (uint64_t) Multitasking::ExitThread;

		// kernel thread
		*--stack = 0x10;				// SS (-8)
		*--stack = u + DefaultRing3StackSize - 8;	// User stack pointer (-16)
		*--stack = 0x202;				// RFLAGS (-24)
		*--stack = 0x08;				// CS (-32)
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

	static void SetupStackThread_Proc(Thread* thread, uint64_t u, uint64_t f, void* p1, void* p2, void* p3, void* p4, void* p5, void* p6)
	{
		UNUSED(p1);
		UNUSED(p2);
		UNUSED(p3);
		UNUSED(p4);
		UNUSED(p5);
		UNUSED(p6);

		uint64_t* stack = (uint64_t*) thread->StackPointer;
		// put argv* and envp* on stack.

		uint64_t* usp = (uint64_t*) (u + DefaultRing3StackSize - 8);
		*--usp = 0;								//		(envp)
		*--usp = 0;

		// user thread (always)
		*--stack = 0x23;				// SS
		*--stack = u + DefaultRing3StackSize - 8;	// User stack pointer
		*--stack = 0x202;				// RFLAGS
		*--stack = 0x1B;				// CS
		*--stack = (uint64_t) f;				// RIP (-40)

		*--stack = 0;					// R15 (-48)
		*--stack = 0;					// R14 (-56)
		*--stack = 0;					// R13 (-64)
		*--stack = 0;					// R12 (-72)
		*--stack = 0;					// R11 (-80)
		*--stack = 0;					// R10 (-88)
		*--stack = 0;					// R9 (-96)
		*--stack = 0;					// R8 (-104)

		*--stack = 0;					// RDX (-112)	(envc)
		*--stack = 0;					// RCX (-120)	(envp)
		*--stack = 0;					// RBX (-128)
		*--stack = 0;					// RAX (-136)

		*--stack = 0;					// RBP (-144)
		*--stack = 0;					// RSI (-152)	(argv)
		*--stack = 0;					// RDI (-160)	(argc)

		thread->StackPointer = (uint64_t) stack;
	}

	static void SetupStackThread(Thread* thread, uint64_t u, uint64_t f, void* p1, void* p2, void* p3, void* p4, void* p5, void* p6)
	{
		if(thread->Parent == Kernel::KernelProcess)
			SetupStackThread_Kern(thread, u, f, p1, p2, p3, p4, p5, p6);

		else
			SetupStackThread_Proc(thread, u, f, p1, p2, p3, p4, p5, p6);
	}

	Thread* CreateThread(Process* Parent, void (*Function)(), uint8_t Priority, void* p1, void* p2, void* p3, void* p4, void* p5, void* p6)
	{
		Thread* thread = new Thread();

		// Allocate kernel stack
		// uint64_t k = Physical::AllocatePage(DefaultRing3StackSize / 0x1000);

		uint64_t k = Virtual::AllocatePage(DefaultRing3StackSize / 0x1000);
		// Virtual::MapRegion(k, k, DefaultRing3StackSize / 0x1000, 0x3);

		if(Parent->CR3 != GetKernelCR3())
		{
			Virtual::AllocateVirtual(DefaultRing3StackSize / 0x1000, k);
			Virtual::MapRegion(k, k, DefaultRing3StackSize / 0x1000, 0x3);

			// Virtual::MapRegion(k, k, DefaultRing3StackSize / 0x1000, 0x3, (Virtual::PageMapStructure*) Parent->CR3);
		}

		// allocate user stack.
		// uint64_t u = Physical::AllocatePage(DefaultRing3StackSize / 0x1000);
		// Virtual::MapRegion(u, u, DefaultRing3StackSize / 0x1000, 0x7, (Virtual::PageMapStructure*) Parent->CR3);

		uint64_t pu = Physical::AllocatePage(DefaultRing3StackSize / 0x1000);
		uint64_t u = Virtual::AllocateVirtual(DefaultRing3StackSize / 0x1000, 0, Parent->VAS);

		Virtual::MapRegion(u, pu, DefaultRing3StackSize / 0x1000, 0x7, (Virtual::PageMapStructure*) Parent->CR3);



		thread->TopOfStack				= k + DefaultRing3StackSize;
		thread->StackPointer				= thread->TopOfStack;
		thread->Thread				= Function;
		thread->State					= 1;
		thread->ThreadID				= NumThreads;
		thread->Parent				= Parent;
		thread->CurrentSharedMemoryOffset		= 0;
		thread->SimpleMessageQueue		= new Library::LinkedList<IPC::SimpleMessage>();
		thread->Priority				= Priority;
		thread->ExecutionTime			= 0;
		thread->InstructionPointer			= 0;


		NumThreads++;
		Parent->Threads->InsertFront(thread);


		SetupStackThread(thread, u, (uint64_t) Function, p1, p2, p3, p4, p5, p6);



		// unmap
		if(Parent->CR3 != (uint64_t) Virtual::GetCurrentPML4T())
		{
			Virtual::UnMapRegion(k, DefaultRing3StackSize / 0x1000);
		}

		if(FirstProc)
		{
			// set tss
			asm volatile("movq %[stacktop], 0x504" :: [stacktop]"r"(thread->TopOfStack));
		}

		Log("Created Thread: Parent: %s, TID: %d", Parent->Name, thread->ThreadID);
		return thread;
	}

	Thread* CreateKernelThread(void (*Function)(), uint8_t Priority, void* p1, void* p2, void* p3, void* p4, void* p5, void* p6)
	{
		return CreateThread(Kernel::KernelProcess, Function, Priority, p1, p2, p3, p4, p5, p6);
	}

	Process* CreateProcess(const char name[64], uint8_t Flags, void (*Function)(), uint8_t Priority)
	{
		using namespace Kernel::HardwareAbstraction::MemoryManager::Virtual;
		using Library::LinkedList;

		Process* process = new Process();
		process->Threads = new LinkedList<Thread>();

		PageMapStructure* PML4 = FirstProc ? (PageMapStructure*)(GetKernelCR3()) : (PageMapStructure*) Virtual::CreateVAS();

		process->Flags					= Flags;
		process->ProcessID				= NumProcesses;
		process->CR3					= (uint64_t) PML4;
		process->SimpleMessageQueue		= new Library::LinkedList<IPC::SimpleMessage>();
		process->CurrentSharedMemoryOffset		= 0;
		process->CurrentFDIndex			= 4;
		process->AllocatedPageList			= new Library::Vector<uint64_t>();
		process->VAS					= new Virtual::VirtualAddressSpace(PML4);
		process->SignalHandlers			= (sighandler_t*) KernelHeap::AllocateChunk(sizeof(sighandler_t) * __SIGCOUNT);
		process->iocontext				= new Filesystems::IOContext();
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

		strcpy(process->Name, name);


		NumProcesses++;
		(void) CreateThread(process, Function, Priority);

		if(FirstProc)
			FirstProc = false;

		else
		{
			// setup the descriptors.
			// manually.
			// stdio:
			// Process* parent = process->Parent;
			// process->FileDescriptors[0].Pointer = parent->FileDescriptors[0].Pointer;
			// process->FileDescriptors[1].Pointer = parent->FileDescriptors[1].Pointer;
			// process->FileDescriptors[2].Pointer = parent->FileDescriptors[2].Pointer;
			// process->FileDescriptors[3].Pointer = parent->FileDescriptors[3].Pointer;
		}

		Log("Creating new process in VAS (%s): CR3(phys): %x", name, (uint64_t) PML4);
		return process;
	}

}
}
}
















