// Scheduler.cpp
// Copyright (c) 2013 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#include <Kernel.hpp>
#include <HardwareAbstraction/Devices/IOPort.hpp>
#include <String.hpp>
#include <stdlib.h>
#include <stl/vector.h>

using namespace Kernel;
using namespace Library;


namespace Kernel {
namespace HardwareAbstraction {
namespace Multitasking
{
	static bool IsFirst = true;
	static stl::vector<Thread*> PendingSleepList;

	static RunQueue mainRunQueue;
	static Thread* CurrentThread = 0;

	static uint64_t CurrentCR3 = 0;
	static uint64_t ScheduleCount = 0;

	rde::vector<Thread*> SleepList;
	rde::vector<Process*> ProcessList;

	bool SchedulerEnabled = true;

	void Initialise()
	{
		CurrentCR3 = GetKernelCR3();
		// SleepList = new stl::vector<Thread*>();
		// ProcessList = new stl::vector<Process*>();
		// PendingSleepList = new rde::vector<Thread*>();

		mainRunQueue = RunQueue();
		mainRunQueue.queue = new rde::vector<Thread*>[NUM_PRIO];

		// for(int i = 0; i < NUM_PRIO; i++)
			// mainRunQueue.queue[i] = stl::vector<Thread*>();

		*((int64_t*) 0x2610) = 0;
	}



	// static void* getsp()
	// {
	// 	void* sp = 0;
	// 	asm volatile("movq %%rsp, %0" : "=r"(sp) : /* No input */ : /* no clobbered */);

	// 	return sp;
	// }






	Process* GetCurrentProcess()	{ return CurrentThread ? CurrentThread->Parent : Kernel::KernelProcess; }
	Thread* GetCurrentThread()		{ return CurrentThread; }
	pid_t GetCurrentThreadID()		{ return CurrentThread->ThreadID; }
	pid_t GetCurrentProcessID()		{ return CurrentThread->Parent->ProcessID; }
	RunQueue& GetRunQueue()			{ return mainRunQueue; }

	// if, after N number of switches, processes in the low/norm queue don't get to run, run them all to completion.
	// todo: fix scheduler. starvation still happens (unable to prevent cross-starvation)
	static size_t StarvationThresholds[NUM_PRIO] = { 64, 16, 1, 1 };

	Thread* GetNextThread()
	{
		auto& theQueue = GetRunQueue();
		Thread* r = CurrentThread;
		theQueue.lock();

		for(int i = 0; i < NUM_PRIO; i++)
		{
			if(!theQueue.queue[i].empty() && (ScheduleCount % StarvationThresholds[i]) == 0)
			{
				r = theQueue.queue[i].front();
				theQueue.queue[i].erase(theQueue.queue[i].begin());
				theQueue.queue[i].push_back(r);
				break;
			}
		}

		if(r == nullptr)
		{
			Log(3, "FATAL: Thread was null!");
			// error recovery: switch to the kernel thread
			CurrentThread = KernelProcess->Threads.front();
			r = CurrentThread;
			assert(r);
		}

		theQueue.unlock();

		ScheduleCount++;
		return r;
	}

	extern "C" uint64_t SwitchProcess(uint64_t context)
	{
		if(BOpt_Likely(!IsFirst))
		{
			if(PendingSleepList.size() > 0)
			{
				for(uint64_t i = 0, s = PendingSleepList.size(); i < s; i++)
				{
					auto front = PendingSleepList.front();
					PendingSleepList.erase_unordered(PendingSleepList.begin());
					SleepList.push_back(front);

					SleepList.back()->StackPointer = context;
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




		// 0x2610 stores the thread's current errno.
		// we therefore need to save it before switching threads.
		CurrentThread->currenterrno = *((int64_t*) 0x2610);
		CurrentThread = GetNextThread();



		if(CurrentThread->Parent->Flags & 0x1)
		{
			// this tells switch.s (on return) that we need to return to user-mode.
			*((uint64_t*) 0x2608) = 0xFADE;
		}


		// set tss
		*((uint64_t*) 0x2504) = CurrentThread->TopOfStack;

		// update fs
		uint64_t tlsptr = (uintptr_t) &CurrentThread->tlsptr;
		SetTLS(tlsptr);


		if((uint64_t) CurrentThread->Parent->VAS.PML4 != CurrentCR3)
		{
			using namespace MemoryManager;

			// Only change the value in cr3 if we need to, to avoid trashing the TLB.
			*((uint64_t*) 0x2600) = (uint64_t) CurrentThread->Parent->VAS.PML4;
			CurrentCR3 = (uint64_t) CurrentThread->Parent->VAS.PML4;
			Virtual::SwitchPML4T((Virtual::PageMapStructure*) CurrentCR3);
		}
		else
		{
			*((uint64_t*) 0x2600) = 0;
		}



		return CurrentThread->StackPointer;
	}

	extern "C" void VerifySchedule()
	{
	}

	extern "C" void YieldCPU()
	{
		asm volatile("int $0xF7");
	}

	void Sleep(int64_t t)
	{
		if(CurrentThread->Sleep != 0)
		{
			Log("SLEEP (%d, %d, %x)", GetCurrentThread()->ThreadID, GetCurrentThread()->State, __builtin_return_address(0));
		}

		Thread* p = FetchAndRemoveThread(CurrentThread);

		p->Sleep = (uint32_t) __abs(t);
		PendingSleepList.push_back(p);

		// if time is negative, we called from userspace, so don't nest interrupts.
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
			return (uint64_t) CurrentThread->Parent->VAS.PML4;
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




