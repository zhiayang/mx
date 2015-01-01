// Scheduler.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#include <Kernel.hpp>
#include <HardwareAbstraction/Devices/IOPort.hpp>
#include <String.hpp>
#include <stdlib.h>

using namespace Kernel;
using namespace Library;


namespace Kernel {
namespace HardwareAbstraction {
namespace Multitasking
{
	static bool IsFirst = true;
	static rde::list<Thread*>* PendingSleepList;
	static RunQueue* mainRunQueue;
	static Thread* CurrentThread = 0;
	static uint64_t CurrentCR3 = 0;
	static uint64_t ScheduleCount = 0;

	rde::list<Thread*>* SleepList;
	rde::list<Process*>* ProcessList;

	bool SchedulerEnabled = true;
	bool isfork = false;

	void Initialise()
	{
		CurrentCR3 = GetKernelCR3();
		SleepList = new rde::list<Thread*>();
		ProcessList = new rde::list<Process*>();
		PendingSleepList = new rde::list<Thread*>();

		mainRunQueue = new RunQueue();
		mainRunQueue->queue = new rde::list<Thread*>*[NUM_PRIO];

		for(int i = 0; i < NUM_PRIO; i++)
			mainRunQueue->queue[i] = new rde::list<Thread*>();

		*((int64_t*) 0x2610) = 0;
	}

	Process* GetCurrentProcess()		{ return CurrentThread ? CurrentThread->Parent : Kernel::KernelProcess; }
	Thread* GetCurrentThread()			{ return CurrentThread; }
	uint64_t GetCurrentThreadID()		{ return CurrentThread->ThreadID; }
	uint64_t GetCurrentProcessID()		{ return CurrentThread->Parent->ProcessID; }
	RunQueue* getRunQueue()				{ return mainRunQueue; }

	// if, after N number of switches, processes in the low/norm queue don't get to run, run them all to completion.
	// todo: fix scheduler. starvation still happens (unable to prevent cross-starvation)
	static int StarvationThresholds[NUM_PRIO] = { 64, 16, 1, 1 };

	Thread* GetNextThread()
	{
		auto theQueue = getRunQueue();
		Thread* r = CurrentThread;
		theQueue->lock();
		for(int i = 0; i < NUM_PRIO; i++)
		{
			if(!theQueue->queue[i]->empty() && (ScheduleCount % StarvationThresholds[i]) == 0)
			{
				r = theQueue->queue[i]->pop_front();
				theQueue->queue[i]->push_back(r);
				break;
			}
		}

		if(r == nullptr)
		{
			Log("ERROR");
			// error recovery: switch to the kernel thread
			CurrentThread = KernelProcess->Threads.front();
			r = CurrentThread;
			assert(r);
		}

		theQueue->unlock();

		ScheduleCount++;
		return r;
	}

	extern "C" uint64_t SwitchProcess(uint64_t context)
	{
		if(BOpt_Likely(!IsFirst))
		{
			if(PendingSleepList->size() > 0)
			{
				for(uint64_t i = 0, s = PendingSleepList->size(); i < s; i++)
				{
					SleepList->push_back(PendingSleepList->pop_front());
					SleepList->back()->StackPointer = context;
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

		if(isfork)
		{
			while(CurrentThread->Parent->Name[0] == 'a')
			{
				CurrentThread = GetNextThread();
				// Log(3, "A");
			}
		}

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


		if((uint64_t) CurrentThread->Parent->VAS->PML4 != CurrentCR3)
		{
			// Only change the value in cr3 if we need to, to avoid trashing the TLB.
			*((uint64_t*) 0x2600) = (uint64_t) CurrentThread->Parent->VAS->PML4;

			CurrentCR3 = (uint64_t) CurrentThread->Parent->VAS->PML4;
			MemoryManager::Virtual::SwitchPML4T((MemoryManager::Virtual::PageMapStructure*) CurrentCR3);
		}
		else
		{
			*((uint64_t*) 0x2600) = 0;
		}

		return CurrentThread->StackPointer;
	}

	extern "C" void VerifySchedule()
	{
		if(CurrentThread->Parent->Name[0] == 'b')
		{
			Utilities::StackDump((uint64_t*) CurrentThread->StackPointer, 20);
			// HALT("");
		}
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
		PendingSleepList->push_back(p);

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
			return (uint64_t) CurrentThread->Parent->VAS->PML4;
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




