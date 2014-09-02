// Scheduler.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#include <Kernel.hpp>
#include <HardwareAbstraction/Devices/IOPort.hpp>
#include <List.hpp>
#include <String.hpp>
#include <stdlib.h>

using namespace Kernel;
using namespace Library;

namespace Kernel {
namespace HardwareAbstraction {
namespace Multitasking
{
	static bool IsFirst = true;
	LinkedList<Thread>* SleepList;
	static LinkedList<Thread>* PendingSleepList;

	Mutex* listlock;
	LinkedList<Thread>* ThreadList_LowPrio;
	LinkedList<Thread>* ThreadList_NormPrio;
	LinkedList<Thread>* ThreadList_HighPrio;

	LinkedList<Process>* ProcessList;

	static Thread* CurrentThread = 0;
	static uint64_t CurrentCR3;
	static uint64_t ScheduleCount = 0;

	bool SchedulerEnabled = true;

	void Initialise()
	{
		CurrentCR3 = GetKernelCR3();
		SleepList = new LinkedList<Thread>();
		ProcessList = new LinkedList<Process>();
		PendingSleepList = new LinkedList<Thread>();

		ThreadList_LowPrio = new LinkedList<Thread>();
		ThreadList_NormPrio = new LinkedList<Thread>();
		ThreadList_HighPrio = new LinkedList<Thread>();

		listlock = new Mutex();
	}

	Process* GetCurrentProcess()			{ return CurrentThread ? CurrentThread->Parent : Kernel::KernelProcess;	}
	Thread* GetCurrentThread()			{ return CurrentThread;							}
	uint64_t GetCurrentThreadID()			{ return CurrentThread->ThreadID;					}
	uint64_t GetCurrentProcessID()		{ return CurrentThread->Parent->ProcessID;				}

	// if, after N number of switches, processes in the low/norm queue don't get to run, run them all to completion.
	#define LowStarveThreshold	1024
	#define NormStarveThreshold	128

	Thread* GetNextThread()
	{
		ScheduleCount++;
		Thread* r = nullptr;

		if(ThreadList_LowPrio->Size() > 0 && (ScheduleCount % LowStarveThreshold == 0))
		{
			r = ThreadList_LowPrio->RemoveFront();
			ThreadList_LowPrio->InsertBack(r);
		}
		else if(ThreadList_NormPrio->Size() > 0 && (ScheduleCount % NormStarveThreshold == 0))
		{
			r = ThreadList_NormPrio->RemoveFront();
			ThreadList_NormPrio->InsertBack(r);
		}
		else if(ThreadList_HighPrio->Size() > 0)
		{
			r = ThreadList_HighPrio->RemoveFront();
			ThreadList_HighPrio->InsertBack(r);
		}
		else
		{
			r = CurrentThread;
		}

		if(r == nullptr)
			HALT("wtfs");

		return r;
	}

	extern "C" uint64_t SwitchProcess(uint64_t context)
	{
		if(BOpt_Likely(!IsFirst))
		{
			if(PendingSleepList->Size() > 0)
			{
				for(uint64_t i = 0, s = PendingSleepList->Size(); i < s; i++)
				{
					SleepList->InsertBack(PendingSleepList->RemoveFront());
					SleepList->Back()->StackPointer = context;
				}
			}
			else
			{
				CurrentThread->StackPointer = context;
			}
		}
		else
			IsFirst = false;



		if(BOpt_Likely(!IsFirst))
		{
			uint64_t StackBottom = CurrentThread->TopOfStack - DefaultRing3StackSize;

			// check if we're about to overflow.
			if(context - StackBottom <= 0x100)
			{
				Log(1, "Warning: Thread(%d) of Process(%s, %d) has only 0x100 bytes of stack space left", CurrentThread->ThreadID, CurrentThread->Parent->Name, CurrentThread->Parent->ProcessID);
			}
			else if(context - StackBottom <= 0x10)
			{
				Log(1, "Warning: Thread(%d) of Process(%s, %d) has only 16 bytes of stack space left, taking action...", CurrentThread->ThreadID, CurrentThread->Parent->Name, CurrentThread->Parent->ProcessID);
				UHALT();
			}
		}

		// find the current rip and store it.
		/* context:

			rdi		<-- context
			rsi
			...
			r8
			r9
			...
			r14
			r15		(+120)
			rip		<-- this.
		*/
		CurrentThread->InstructionPointer = *((uint64_t*) (context + 120));
		CurrentThread = GetNextThread();


		if(CurrentThread->Parent->Flags & 0x1)
		{
			// this tells switch.s (on return) that we need to return to user-mode.
			*((uint64_t*) 0x2608) = 0xFADE;
		}

		if(CurrentThread->Parent->CR3 != CurrentCR3)
		{
			// Only change the value in cr3 if we need to, to avoid trashing the TLB.
			*((uint64_t*) 0x2600) = CurrentThread->Parent->CR3;

			CurrentCR3 = CurrentThread->Parent->CR3;
			MemoryManager::Virtual::SwitchPML4T((MemoryManager::Virtual::PageMapStructure*) CurrentCR3);
		}
		else
			*((uint64_t*) 0x2600) = 0;


		// set tss
		*((uint64_t*) 0x2504) = CurrentThread->TopOfStack;

		// update fs
		// let asm do it, 0x2610 contains the address.
		// *((uint64_t*) 0x2610) = ((uint64_t) CurrentThread->tlsptr) + CurrentThread->Parent->tlssize;
		uint64_t tlsptr = CurrentThread->tlsptrptr;
		if(CurrentThread->Parent->Flags & 0x1)
			Log("%x - eax: %x, edx: %x", tlsptr, tlsptr & 0xFFFFFFFF, tlsptr >> 32);

		uint64_t thing = 0;
		uint32_t low = tlsptr & 0xFFFFFFFF;
		uint32_t high = tlsptr >> 32;
		uint64_t eax = 0;
		uint64_t edx = 0;
		asm volatile(
				"mov $0x2B, %%bx		\n\t"
				"mov %%bx, %%fs		\n\t"

				"movl $0xC0000100, %%ecx	\n\t"
				"movl %[lo], %%eax		\n\t"
				"movl %[hi], %%edx		\n\t"
				"wrmsr				\n\t"

				: "=a"(eax), "=d"(edx) : [lo]"g"(low), [hi]"g"(high) : "memory", "rax", "rbx", "rcx", "rdx");

		if(CurrentThread->Parent->Flags & 0x1)
		{
			asm volatile("lea %%fs:0x0, %%rax; mov %%rax, %[put]" : [put]"=g"(thing) :: "rax");
			Log("%x - eax: %x, edx: %x", thing, eax, edx);
		}

		return CurrentThread->StackPointer;
	}





	extern "C" void YieldCPU()
	{
		asm volatile("int $0xF7");
	}

	void Sleep(int64_t time)
	{
		if(CurrentThread->Sleep != 0)
		{
			Log("SLEEP (%d, %d, %x)", GetCurrentThread()->ThreadID, GetCurrentThread()->State, __builtin_return_address(0));
		}

		Thread* p = FetchAndRemoveThread(CurrentThread);

		p->Sleep = (uint32_t) __abs(time);
		PendingSleepList->InsertBack(p);

		// if time is negative, we called from userspace, so don't nest interrupts.
		if(time > 0)
			YieldCPU();
	}

	uint64_t GetCurrentCR3()
	{
		if(IsFirst)
		{
			return Kernel::GetKernelCR3();
		}
		else
		{
			return CurrentThread->Parent->CR3;
		}
	}

	void DisableScheduler()
	{
		SchedulerEnabled = false;
	}

	void EnableScheduler()
	{
		SchedulerEnabled = true;
	}

}
}
}




