// Scheduler.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#include <Kernel.hpp>
#include <StandardIO.hpp>
#include <List.hpp>
#include <AVLTree.hpp>
#include <String.hpp>


using namespace Kernel;
using namespace Library;

namespace Kernel {
namespace HardwareAbstraction {
namespace Multitasking
{
	// if, after N number of switches, processes in the low/norm queue don't get to run, run them all to completion.
	#define LowStarveThreshold	256
	#define NormStarveThreshold	64


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

	uint64_t NumThreads = 0;
	uint64_t NumProcesses = 0;
	static uint64_t ScheduleCount = 0;
	bool SchedulerEnabled = true;

	static uint64_t LowStarvationLevel = 0;
	static uint64_t NormStarvationLevel = 0;

	static uint64_t LowStarvedRemaining = 0;
	static uint64_t NormStarvedRemaining = 0;

	void Initialise()
	{
		CurrentCR3 = GetKernelCR3();
		SleepList = new LinkedList<Thread>();
		ProcessList = new LinkedList<Process>();
		PendingSleepList = new LinkedList<Thread>();
		// ThreadQueue = new AVLTree<uint16_t, Thread*>();

		ThreadList_LowPrio = new LinkedList<Thread>();
		ThreadList_NormPrio = new LinkedList<Thread>();
		ThreadList_HighPrio = new LinkedList<Thread>();

		listlock = new Mutex();
	}

	Process* GetCurrentProcess()			{ return CurrentThread ? CurrentThread->Parent : Kernel::KernelProcess;	}
	Thread* GetCurrentThread()			{ return CurrentThread;							}
	uint64_t GetCurrentThreadID()			{ return CurrentThread->ThreadID;					}
	uint64_t GetCurrentProcessID()		{ return CurrentThread->Parent->ProcessID;				}


	Thread* GetNextThread()
	{
		ScheduleCount++;
		Thread* r;

		if(ThreadList_HighPrio->Size() > 0 && NormStarvationLevel < NormStarveThreshold && NormStarvedRemaining == 0
			&& LowStarvationLevel < LowStarveThreshold && LowStarvedRemaining == 0)
		{
			r = ThreadList_HighPrio->RemoveFront();
			ThreadList_HighPrio->InsertBack(r);

			NormStarvationLevel++;
			LowStarvationLevel++;
		}
		else if(ThreadList_NormPrio->Size() > 0 && LowStarvationLevel < LowStarveThreshold && LowStarvedRemaining == 0)
		{
			r = ThreadList_NormPrio->RemoveFront();
			ThreadList_NormPrio->InsertBack(r);

			LowStarvationLevel++;

			// check if we're starved.
			if(NormStarvationLevel >= NormStarveThreshold)
			{
				// we are.
				NormStarvedRemaining = ThreadList_NormPrio->Size();
				NormStarvationLevel = 0;
			}
			if(NormStarvedRemaining > 0)
			{
				NormStarvedRemaining--;
			}
		}
		else if(ThreadList_LowPrio->Size() > 0)
		{
			r = ThreadList_LowPrio->RemoveFront();
			ThreadList_LowPrio->InsertBack(r);

			if(LowStarvationLevel >= LowStarveThreshold)
			{
				// we are.
				LowStarvedRemaining = ThreadList_LowPrio->Size();
				LowStarvationLevel = 0;
			}
			if(LowStarvedRemaining > 0)
			{
				LowStarvedRemaining--;
			}
		}
		else
		{
			r = CurrentThread;
		}

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
			asm volatile("movq $0x000000000000FADE, 0x608" ::: "memory");
		}

		if(CurrentThread->Parent->CR3 != CurrentCR3)
		{
			// Only change the value in cr3 if we need to, to avoid trashing the TLB.
			asm volatile("movq %[page], 0x600" :: [page]"r"(CurrentThread->Parent->CR3): "memory");
			CurrentCR3 = CurrentThread->Parent->CR3;
			MemoryManager::Virtual::SwitchPML4T((MemoryManager::Virtual::PageMapStructure*) CurrentThread->Parent->CR3);
		}
		else
		{
			asm volatile("movq $0x0, 0x600" ::: "memory");
		}


		// set tss
		asm volatile("movq %[stacktop], 0x504" :: [stacktop]"r"(CurrentThread->TopOfStack));


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

		p->Sleep = (uint32_t) math::abs(time);
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




