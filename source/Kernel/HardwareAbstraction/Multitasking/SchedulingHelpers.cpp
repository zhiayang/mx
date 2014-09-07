// Scheduler.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#include <Kernel.hpp>
#include <StandardIO.hpp>
#include <List.hpp>
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
		Thread* t = GetThread(tid);
		if(t == 0)
		{
			SetThreadErrno(EINVAL);
		}

		void* retval = 0;

		WatchThread(tid);
		BLOCK();

		retval = t->returnval;

		delete t;
		return (void*) retval;
	}

	extern "C" void Syscall_DetachThread(pthread_t tid)
	{
		volatile Thread* volatile t = GetThread(tid);
		if(t == 0)
		{
			SetThreadErrno(EINVAL);
		}

		t->flags |= FLAG_DETACHED;
	}

	extern "C" pthread_t Syscall_GetTID()
	{
		return (pthread_t) GetCurrentThreadID();
	}

	void WatchThread(pthread_t tid)
	{
		Thread* target = GetThread(tid);
		if(!target->watchers)
			target->watchers = new Library::LinkedList<Thread>();

		Thread* cur = GetCurrentThread();
		if(!cur->watching)
			cur->watching = new Library::LinkedList<Thread>();

		target->watchers->push_back(cur);
		cur->watching->push_back(target);
	}

	void UnwatchThread(pthread_t tid)
	{
		// make sure we're watching it first
		Thread* cur = GetCurrentThread();
		if(!cur->watching)
			return;

		Thread* target = GetThread(tid);
		if(!target->watchers)
			return;

		target->watchers->RemoveAt(target->watchers->IndexOf(cur));
		cur->watching->RemoveAt(cur->watching->IndexOf(target));
	}




	void Cleanup(Thread* t)
	{
		// delete all the thread's resources.
		assert(t);

		if(t->messagequeue)		delete t->messagequeue;
		if(t->watching)			delete t->watching;
		if(t->watchers)			delete t->watchers;
		if(t->CrashState)		delete t->CrashState;

		delete t;
	}

	void Cleanup(Process* p)
	{
		// todo: close all file handles
		// todo: delete ioctx

		assert(p);
		assert(p->Threads);
		assert(p->Threads->size() == 0);
		delete p->Threads;

		// destroy its address space
		assert(p->VAS);
		MemoryManager::Virtual::DestroyVAS(p->VAS);

		delete p;
	}

	void SetThreadErrno(int errno)
	{
		// set here in case we get switched
		// the 'ol bait 'n' switch
		GetCurrentThread()->currenterrno = errno;

		// set here for return asm to find.
		*((int64_t*) 0x2610) = errno;
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
			return nullptr;
		}

		return GetThreadList(thread)->RemoveAt((uint64_t) id);
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
			GetThreadList(thread)->push_front(FetchAndRemoveThread(thread));
		}
		else
		{
			// thread is blocking, shoo it out of the blocking queue.
			int64_t id = SleepList->IndexOf(thread);

			if(id < 0)
				HALT("Thread corrupted");

			Thread* t = SleepList->RemoveAt((uint64_t) id);
			t->State = STATE_NORMAL;
			GetThreadList(t)->push_front(t);
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
		SleepList->push_back(thread);

		// if(purpose == 0)
		(void) purpose;
		YieldCPU();
	}








	void AddToQueue(Process* Proc)
	{
		ProcessList->push_front(Proc);

		for(uint8_t d = 0; d < Proc->Threads->size(); d++)
		{
			AddToQueue(Proc->Threads->get(d));
		}
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

			GetThreadList(p)->RemoveAt((uint64_t) GetThreadList(p)->IndexOf(p));
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

			int64_t id = par->Threads->IndexOf(p);

			if(id < 0)
				HALT("Thread corrupted");

			par->Threads->RemoveAt((uint64_t) id);
			GetThreadList(p)->RemoveAt((uint64_t) GetThreadList(p)->IndexOf(p));
			SleepList->push_front(p);

			// wake up the watching threads.
			if(p->watchers)
			{
				for(size_t i = 0, s = p->watchers->size(); i < s; i++)
				{
					Thread* w = p->watchers->pop_front();
					assert(w->watching);

					w->watching->RemoveAt(w->watching->IndexOf(p));
					WakeForMessage(w);
				}
			}
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
		for(uint64_t k = 0; k < p->Threads->size(); k++)
		{
			Suspend(p->Threads->get(k));
		}
	}

	void Resume(Process* p)
	{
		for(uint64_t k = 0; k < p->Threads->size(); k++)
		{
			Resume(p->Threads->get(k));
		}
	}

	void Kill(Process* p)
	{
		Log("Killing %d thread%s of process %s", p->Threads->size(), p->Threads->size() == 1 ? "" : "s", p->Name);
		for(uint64_t k = 0; k < p->Threads->size(); k++)
		{
			Kill(p->Threads->get(k));
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
		for(uint64_t s = 0; s < ThreadList_HighPrio->size(); s++)
		{
			if(String::Compare(n, ThreadList_HighPrio->get(s)->Parent->Name) == 0)
				list->push_front(ThreadList_HighPrio->get(s));
		}
		for(uint64_t s = 0; s < ThreadList_HighPrio->size(); s++)
		{
			if(String::Compare(n, ThreadList_NormPrio->get(s)->Parent->Name) == 0)
				list->push_front(ThreadList_NormPrio->get(s));
		}
		for(uint64_t s = 0; s < ThreadList_LowPrio->size(); s++)
		{
			if(String::Compare(n, ThreadList_LowPrio->get(s)->Parent->Name) == 0)
				list->push_front(ThreadList_LowPrio->get(s));
		}




		for(uint64_t s = 0; s < SleepList->size(); s++)
		{
			if(String::Compare(n, SleepList->get(s)->Parent->Name) == 0)
				list->push_front(SleepList->get(s));
		}

		return list;
	}

	Thread* GetProcessByName(const char* n)
	{
		return SearchByName(n)->size() == 0 ? 0 : SearchByName(n)->front();
	}

	Process* GetProcess(uint64_t pid)
	{
		for(uint64_t i = 0; i < ProcessList->size(); i++)
		{
			if(ProcessList->get(i)->ProcessID == pid)
				return ProcessList->get(i);
		}

		return 0;
	}

	Thread* GetThread(uint64_t tid)
	{
		for(uint64_t i = 0; i < ThreadList_HighPrio->size(); i++)
		{
			if(ThreadList_HighPrio->get(i)->ThreadID == tid)
				return ThreadList_HighPrio->get(i);
		}

		for(uint64_t i = 0; i < ThreadList_NormPrio->size(); i++)
		{
			if(ThreadList_NormPrio->get(i)->ThreadID == tid)
				return ThreadList_NormPrio->get(i);
		}

		for(uint64_t i = 0; i < ThreadList_LowPrio->size(); i++)
		{
			if(ThreadList_LowPrio->get(i)->ThreadID == tid)
				return ThreadList_LowPrio->get(i);
		}

		for(uint64_t i = 0; i < SleepList->size(); i++)
		{
			if(SleepList->get(i)->ThreadID == tid)
				return SleepList->get(i);
		}

		return 0;
	}
}
}
}




