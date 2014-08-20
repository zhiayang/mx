// Scheduler.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#include <Kernel.hpp>
#include <StandardIO.hpp>
#include <List.hpp>
#include <string.h>

using namespace Kernel;
using namespace Library;


namespace Kernel {
namespace HardwareAbstraction {
namespace Multitasking
{
	extern "C" void ExitThread()
	{
		Log("Thread %d exited.", GetCurrentThread()->ThreadID);
		Kill(GetCurrentThread());
		while(true);
	}



	Library::LinkedList<Thread>* GetThreadList(Thread* t)
	{
		return t->Priority == 0 ? ThreadList_LowPrio : (t->Priority == 1 ? ThreadList_NormPrio : ThreadList_HighPrio);
	}

	Thread* FetchAndRemoveThread(Thread* thread)
	{
		int64_t id = GetThreadList(thread)->IndexOf(thread);

		if(id < 0)
		{
			HALT("Thread corrupted");
		}

		return GetThreadList(thread)->RemoveAt((uint64_t) id);
	}

	uint64_t GetNumberOfThreads()
	{
		return ThreadList_HighPrio->Size() + ThreadList_NormPrio->Size() + ThreadList_LowPrio->Size() + SleepList->Size();
	}

	// waking for IPC will place the process in the front of the run queue of its current priority.
	void WakeForMessage(Process* process)
	{
		// not supported.
		(void) process;
	}

	void WakeForMessage(Thread* thread)
	{
		if(thread->State != STATE_BLOCKING)
		{
			GetThreadList(thread)->InsertFront(FetchAndRemoveThread(thread));
		}
		else
		{
			// thread is blocking, shoo it out of the blocking queue.
			int64_t id = SleepList->IndexOf(thread);

			if(id < 0)
				HALT("Thread corrupted");

			Thread* t = SleepList->RemoveAt((uint64_t) id);
			GetThreadList(t)->InsertFront(t);
			t->State = STATE_NORMAL;
		}

		YieldCPU();
	}



	void Block(uint8_t purpose)
	{
		// assert(GetCurrentThread()->State != STATE_BLOCKING);
		if(GetCurrentThread()->State == STATE_BLOCKING)
		{
			Log("BLOCK (%d, %d, %x)", GetCurrentThread()->ThreadID, GetCurrentThread()->State, __builtin_return_address(0));
			HALT("");
		}

		Thread* thread = FetchAndRemoveThread(GetCurrentThread());

		thread->State = STATE_BLOCKING;
		SleepList->InsertBack(thread);

		if(purpose == 0)
			YieldCPU();
	}








	void AddToQueue(Process* Proc)
	{
		ProcessList->InsertFront(Proc);
		for(uint8_t d = 0; d < Proc->Threads->Size(); d++)
		{
			AddToQueue(Proc->Threads->Get(d));
		}
	}

	void AddToQueue(Thread* t)
	{
		switch(t->Priority)
		{
			case 0:
				ThreadList_LowPrio->InsertFront(t);
				break;

			case 1:
				ThreadList_NormPrio->InsertFront(t);
				break;

			case 2:
				ThreadList_HighPrio->InsertFront(t);
				break;
		}
	}


	void TerminateCurrentThread(ThreadRegisterState_type* r)
	{
		Thread* p = GetCurrentThread();
		if(!p)
			return;

		Kill(p);
		Log("Killed thread %d, name: %s", p->ThreadID, p->Parent->Name);

		(void) r;

		// this is where we would say do a core dump.
		YieldCPU();

		while(true);
	}




	// direct
	void Suspend(Thread* p)
	{
		if(p && p->State == STATE_NORMAL)
		{
			Log("Suspended thread %d, name: %s", p->ThreadID, p->Parent->Name);

			GetThreadList(p)->RemoveAt((uint64_t) GetThreadList(p)->IndexOf(p));
			SleepList->InsertFront(p);

			p->State = STATE_SUSPEND;
		}
	}

	void Resume(Thread* p)
	{
		if(p && p->State == STATE_SUSPEND)
		{
			Log("Resumed thread %d, name: %s", p->ThreadID, p->Parent->Name);

			p->Sleep = 0;
			p->State = STATE_NORMAL;
		}
	}

	void Kill(Thread* p)
	{
		if(p && (p->State == STATE_NORMAL || p->State == STATE_SUSPEND))
		{
			p->State = STATE_AWAITDEATH;

			// remove the thread from its parent process's list.
			Process* par = p->Parent;

			int64_t id = par->Threads->IndexOf(p);

			if(id < 0)
				HALT("Thread corrupted");

			par->Threads->RemoveAt((uint64_t) id);
			GetThreadList(p)->RemoveAt((uint64_t) GetThreadList(p)->IndexOf(p));
			SleepList->InsertFront(p);
		}
	}


	// by name
	void Suspend(const char* p)
	{
		Suspend(GetProcessByName(p));
	}

	void Resume(const char* p)
	{
		Resume(GetProcessByName(p));
	}

	void Kill(const char* p)
	{
		Kill(GetProcessByName(p));
	}


	// by process
	void Suspend(Process* p)
	{
		for(uint64_t k = 0; k < p->Threads->Size(); k++)
		{
			Suspend(p->Threads->Get(k));
		}
	}

	void Resume(Process* p)
	{
		for(uint64_t k = 0; k < p->Threads->Size(); k++)
		{
			Resume(p->Threads->Get(k));
		}
	}

	void Kill(Process* p)
	{
		Log("Killing %d threads of process %s", p->Threads->Size(), p->Name);
		for(uint64_t k = 0; k < p->Threads->Size(); k++)
		{
			Kill(p->Threads->Get(k));
		}
	}


	bool CurrentProcessInRing3()
	{
		if(GetCurrentThread() && GetCurrentThread()->Parent)
			return GetCurrentThread()->Parent->Flags & 0x1;

		return false;
	}

	Library::LinkedList<Thread>* SearchByName(const char* n)
	{
		LinkedList<Thread>* list = new LinkedList<Thread>();
		for(uint64_t s = 0; s < ThreadList_HighPrio->Size(); s++)
		{
			if(strcmp(n, ThreadList_HighPrio->Get(s)->Parent->Name) == 0)
				list->InsertFront(ThreadList_HighPrio->Get(s));
		}
		for(uint64_t s = 0; s < ThreadList_HighPrio->Size(); s++)
		{
			if(strcmp(n, ThreadList_NormPrio->Get(s)->Parent->Name) == 0)
				list->InsertFront(ThreadList_NormPrio->Get(s));
		}
		for(uint64_t s = 0; s < ThreadList_LowPrio->Size(); s++)
		{
			if(strcmp(n, ThreadList_LowPrio->Get(s)->Parent->Name) == 0)
				list->InsertFront(ThreadList_LowPrio->Get(s));
		}




		for(uint64_t s = 0; s < SleepList->Size(); s++)
		{
			if(strcmp(n, SleepList->Get(s)->Parent->Name) == 0)
				list->InsertFront(SleepList->Get(s));
		}

		return list;
	}

	Thread* GetProcessByName(const char* n)
	{
		return SearchByName(n)->Size() == 0 ? 0 : SearchByName(n)->Front();
	}

	Process* GetProcess(uint64_t pid)
	{
		for(uint64_t i = 0; i < ProcessList->Size(); i++)
		{
			if(ProcessList->Get(i)->ProcessID == pid)
				return ProcessList->Get(i);
		}

		return 0;
	}

	Thread* GetThread(uint64_t tid)
	{
		for(uint64_t i = 0; i < ThreadList_HighPrio->Size(); i++)
		{
			if(ThreadList_HighPrio->Get(i)->ThreadID == tid)
				return ThreadList_HighPrio->Get(i);
		}

		for(uint64_t i = 0; i < ThreadList_NormPrio->Size(); i++)
		{
			if(ThreadList_NormPrio->Get(i)->ThreadID == tid)
				return ThreadList_NormPrio->Get(i);
		}

		for(uint64_t i = 0; i < ThreadList_LowPrio->Size(); i++)
		{
			if(ThreadList_LowPrio->Get(i)->ThreadID == tid)
				return ThreadList_LowPrio->Get(i);
		}

		for(uint64_t i = 0; i < SleepList->Size(); i++)
		{
			if(SleepList->Get(i)->ThreadID == tid)
				return SleepList->Get(i);
		}

		return 0;
	}
}
}
}




