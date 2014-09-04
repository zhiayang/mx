// Scheduler.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#include <Kernel.hpp>
#include <StandardIO.hpp>
#include <string.h>
#include <errno.h>

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

	extern "C" void ExitThread_Userspace()
	{
		void* retval;
		asm volatile("mov %%rax, %[ret]" : [ret]"=g"(retval));
		GetCurrentThread()->returnval = retval;

		asm volatile("mov $4012, %%r10; int $0xF8" ::: "r10");
	}

	extern "C" void* Syscall_JoinThread(pthread_t tid)
	{
		// todo: better blocking
		volatile Thread* t = GetThread(tid);
		if(t == 0)
			errno = EINVAL;

		void* retval = 0;
		while(t->State != STATE_DEAD)
			asm volatile("pause");

		retval = t->returnval;

		delete t;
		return (void*) retval;
	}

	extern "C" void Syscall_DetachThread(pthread_t tid)
	{
		volatile Thread* volatile t = GetThread(tid);
		if(t == 0)
			errno = EINVAL;

		t->flags |= FLAG_DETACHED;
	}

	extern "C" pthread_t Syscall_GetTID()
	{
		return (pthread_t) GetCurrentThreadID();
	}














	void SetTLS(uint64_t tlsptr)
	{
		uint32_t low = tlsptr & 0xFFFFFFFF;
		uint32_t high = tlsptr >> 32;
		asm volatile(
			"mov $0x2B, %%bx		\n\t"
			"mov %%bx, %%fs		\n\t"

			"movl $0xC0000100, %%ecx	\n\t"
			"movl %[lo], %%eax		\n\t"
			"movl %[hi], %%edx		\n\t"
			"wrmsr				\n\t"

			:: [lo]"g"(low), [hi]"g"(high) : "memory", "rax", "rbx", "rcx", "rdx");
	}

	rde::list<Thread*>* GetThreadList(Thread* t)
	{
		return t->Priority == 0 ? ThreadList_LowPrio : (t->Priority == 1 ? ThreadList_NormPrio : ThreadList_HighPrio);
	}

	Thread* FetchAndRemoveThread(Thread* thread)
	{
		// int64_t id = GetThreadList(thread)->(thread);
		// GetThreadList(thread)->
		rde::list<Thread*>* thrlist = GetThreadList(thread);
		auto id = rde::find(thrlist->begin(), thrlist->end(), thread);

		if(id == thrlist->end())
		{
			Log(3, "%x", *id);
			HALT("Thread corrupted");
			return nullptr;
		}

		Thread* ret = *id;
		thrlist->erase(id);

		return ret;
	}

	uint64_t GetNumberOfThreads()
	{
		return ThreadList_HighPrio->size() + ThreadList_NormPrio->size() + ThreadList_LowPrio->size() + SleepList->size();
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
			// GetThreadList(thread)->InsertFront(FetchAndRemoveThread(thread));
			rde::list<Thread*>* thrlist = GetThreadList(thread);
			thrlist->push_front(FetchAndRemoveThread(thread));
		}
		else
		{
			// thread is blocking, shoo it out of the blocking queue.
			auto id = rde::find(SleepList->begin(), SleepList->end(), thread);

			if(id == SleepList->end())
				HALT("Thread corrupted");

			Thread* t = *id;
			SleepList->erase(id);

			t->State = STATE_NORMAL;
			GetThreadList(t)->push_front(t);
		}

		YieldCPU();
	}



	void Block(uint8_t purpose)
	{
		if(GetCurrentThread()->State == STATE_BLOCKING)
		{
			Log("BLOCK (%d, %d, %x)", GetCurrentThread()->ThreadID, GetCurrentThread()->State, __builtin_return_address(0));
			HALT("");
		}

		Thread* thread = FetchAndRemoveThread(GetCurrentThread());

		thread->State = STATE_BLOCKING;
		SleepList->push_back(thread);

		if(purpose == 0)
			YieldCPU();
	}








	void AddToQueue(Process* proc)
	{
		ProcessList->push_back(proc);

		for(auto t : *proc->Threads)
			AddToQueue(t);
	}

	void AddToQueue(Thread* t)
	{
		switch(t->Priority)
		{
			case 0:
				ThreadList_LowPrio->push_front(t);
				break;

			case 1:
				ThreadList_NormPrio->push_front(t);
				break;

			case 2:
				ThreadList_HighPrio->push_front(t);
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

			auto thrlist = GetThreadList(p);
			thrlist->erase(rde::find(thrlist->begin(), thrlist->end(), p));
			SleepList->push_front(p);

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
			par->Threads->erase(rde::find(par->Threads->begin(), par->Threads->end(), p));

			auto thrlist = GetThreadList(p);
			thrlist->erase(rde::find(thrlist->begin(), thrlist->end(), p));
			SleepList->push_back(p);
		}
		else
		{
			Log(3, "state: %d", p->State);
			HALT("");
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
		for(auto t : *p->Threads)
			Suspend(t);
	}

	void Resume(Process* p)
	{
		for(auto t : *p->Threads)
			Resume(t);
	}

	void Kill(Process* p)
	{
		Log("Killing %d thread%s of process %s", p->Threads->size(), p->Threads->size() == 1 ? "" : "s", p->Name);
		for(size_t i = 0, s = p->Threads->size(); i < s; i++)
			Kill(p->Threads->front());
	}


	bool CurrentProcessInRing3()
	{
		if(GetCurrentThread() && GetCurrentThread()->Parent)
			return GetCurrentThread()->Parent->Flags & 0x1;

		return false;
	}

	rde::list<Thread*>* SearchByName(const char* n)
	{
		(void) n;
		rde::list<Thread*>* list = new rde::list<Thread*>();
		// for(uint64_t s = 0; s < ThreadList_HighPrio->Size(); s++)
		// {
		// 	if(strcmp(n, ThreadList_HighPrio->Get(s)->Parent->Name) == 0)
		// 		list->push_front(ThreadList_HighPrio->Get(s));
		// }
		// for(uint64_t s = 0; s < ThreadList_HighPrio->Size(); s++)
		// {
		// 	if(strcmp(n, ThreadList_NormPrio->Get(s)->Parent->Name) == 0)
		// 		list->push_front(ThreadList_NormPrio->Get(s));
		// }
		// for(uint64_t s = 0; s < ThreadList_LowPrio->Size(); s++)
		// {
		// 	if(strcmp(n, ThreadList_LowPrio->Get(s)->Parent->Name) == 0)
		// 		list->push_front(ThreadList_LowPrio->Get(s));
		// }




		// for(uint64_t s = 0; s < SleepList->Size(); s++)
		// {
		// 	if(strcmp(n, SleepList->Get(s)->Parent->Name) == 0)
		// 		list->InsertFront(SleepList->Get(s));
		// }

		return list;
	}

	Thread* GetProcessByName(const char* n)
	{
		return SearchByName(n)->size() == 0 ? 0 : SearchByName(n)->front();
	}

	Process* GetProcess(uint64_t pid)
	{
		for(auto p : *ProcessList)
		{
			if(p->ProcessID == pid)
				return p;
		}

		return 0;
	}

	Thread* GetThread(uint64_t tid)
	{
		for(Thread* t : *ThreadList_LowPrio)
		{
			if(t->ThreadID == tid)
				return t;
		}

		for(Thread* t : *ThreadList_NormPrio)
		{
			if(t->ThreadID == tid)
				return t;
		}

		for(Thread* t : *ThreadList_HighPrio)
		{
			if(t->ThreadID == tid)
				return t;
		}

		for(Thread* t : *SleepList)
		{
			if(t->ThreadID == tid)
				return t;
		}

		return 0;
	}
}
}
}




