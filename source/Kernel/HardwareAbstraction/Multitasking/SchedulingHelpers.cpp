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
		Thread* cur = GetCurrentThread();

		target->watchers.push_back(cur);
		cur->watching.push_back(target);
	}

	void UnwatchThread(pthread_t tid)
	{
		// make sure we're watching it first
		Thread* cur = GetCurrentThread();
		Thread* target = GetThread(tid);

		for(auto i = target->watchers.begin(); i != target->watchers.end(); i++)
		{
			if(*i == cur)
				target->watchers.erase(i);
		}

		for(auto i = cur->watching.begin(); i != cur->watching.end(); i++)
		{
			if(*i == target)
				cur->watching.erase(i);
		}
	}




	void Cleanup(Thread* t)
	{
		// delete all the thread's resources.
		assert(t);

		if(t->CrashState)		delete t->CrashState;

		delete t;
	}

	void Cleanup(Process* p)
	{
		// todo: close all file handles
		// todo: delete ioctx

		assert(p);
		assert(p->Threads.size() == 0);

		// destroy its address space
		assert(p->VAS);
		MemoryManager::Virtual::DestroyVAS(p->VAS);
		Log("Cleaned up process %s", p->Name);

		// delete p;
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
			"mov $0x2B, %%bx			\n\t"
			"mov %%bx, %%fs				\n\t"

			"movl $0xC0000100, %%ecx	\n\t"
			"movl %[lo], %%eax			\n\t"
			"movl %[hi], %%edx			\n\t"
			"wrmsr						\n\t"

			:: [lo]"g"(low), [hi]"g"(high) : "memory", "rax", "rbx", "rcx", "rdx");
	}

	rde::list<Thread*>* GetThreadList(Thread* t)
	{
		return getRunQueue()->queue[t->Priority];
	}

	Thread* FetchAndRemoveThread(Thread* thread)
	{
		assert(thread);
		GetThreadList(thread)->remove(thread);
		return thread;
	}

	// waking for IPC will place the process in the front of the run queue of its current priority.
	void WakeForMessage(Process* process)
	{
		// not supported.
		(void) process;
	}

	void WakeForMessage(Thread* thread)
	{
		getRunQueue()->lock();
		assert(thread);

		auto list = GetThreadList(thread);
		if(thread->State != STATE_BLOCKING && thread->State != STATE_SUSPEND)
		{
			assert(list->contains(thread));
			list->push_front(FetchAndRemoveThread(thread));
		}
		else
		{
			assert(SleepList->contains(thread));
			// thread is blocking, shoo it out of the blocking queue.

			SleepList->remove(thread);
			thread->State = STATE_NORMAL;
			GetThreadList(thread)->push_front(thread);
		}

		getRunQueue()->unlock();
		YieldCPU();
	}



	void Block(uint8_t purpose)
	{
		getRunQueue()->lock();
		if(GetCurrentThread()->State == STATE_BLOCKING)
		{
			Log("BLOCK (%d, %d, %x)", GetCurrentThread()->ThreadID, GetCurrentThread()->State, __builtin_return_address(0));
			HALT("");
		}

		Thread* thread = FetchAndRemoveThread(GetCurrentThread());

		thread->State = STATE_BLOCKING;
		SleepList->push_back(thread);

		(void) purpose;
		getRunQueue()->unlock();
		YieldCPU();
	}








	void AddToQueue(Process* Proc)
	{
		ProcessList->push_front(Proc);
		for(auto t : Proc->Threads)
			AddToQueue(t);
	}

	void AddToQueue(Thread* t)
	{
		getRunQueue()->lock();
		getRunQueue()->queue[t->Priority]->push_front(t);
		getRunQueue()->unlock();
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
		getRunQueue()->lock();
		if(p && p->State == STATE_NORMAL)
		{
			Log("Suspended thread %d, name: %s", p->ThreadID, p->Parent->Name);

			// GetThreadList(p)->RemoveAt((uint64_t) GetThreadList(p)->IndexOf(p));

			assert(p);
			GetThreadList(p)->remove(p);
			SleepList->push_front(p);
			p->State = STATE_SUSPEND;
		}
		getRunQueue()->unlock();
	}

	void Resume(Thread* p)
	{
		getRunQueue()->lock();
		if(p && p->State == STATE_SUSPEND)
		{
			Log("Resumed thread %d, name: %s", p->ThreadID, p->Parent->Name);

			p->Sleep = 0;
			p->State = STATE_NORMAL;
		}
		getRunQueue()->unlock();
	}

	void Kill(Thread* p)
	{
		if(p && p->State == STATE_NORMAL)
		{
			getRunQueue()->lock();
			p->State = STATE_AWAITDEATH;

			// remove the thread from its parent process's list.
			Process* par = p->Parent;
			par->Threads.remove(p);

			// GetThreadList(p)->remove(p);
			SleepList->push_front(p);

			getRunQueue()->unlock();

			if(!(par->Flags & FLAG_DYING) && par->Threads.empty())
				Cleanup(par);

			// wake up the watching threads.
			for(size_t i = 0, s = p->watchers.size(); i < s; i++)
			{
				Thread* w = p->watchers.front();
				p->watchers.pop_front();

				w->watching.remove(p);
				WakeForMessage(w);
			}
		}
		else if(p && (p->State == STATE_BLOCKING || p->State == STATE_SUSPEND))
		{
			assert(SleepList->contains(p));
			p->State = STATE_AWAITDEATH;
			Process* par = p->Parent;
			par->Threads.remove(p);
		}
		else
		{
			Log(3, "thread: %x, RA: %x", p, __builtin_return_address(0));
			if(p)
			{
				Log(3, "contents: parent %s:%d, tid %d, state %d", p->Parent->Name, p->Parent->ProcessID, p->ThreadID, p->State);
			}
			HALT("");
		}
	}



	// by process
	void Suspend(Process* p)
	{
		for(auto t : p->Threads)
			Suspend(t);
	}

	void Resume(Process* p)
	{
		for(auto t : p->Threads)
			Resume(t);
	}

	void Kill(Process* p)
	{
		assert(p);
		Log("Killing %d thread%s of process %s", p->Threads.size(), p->Threads.size() == 1 ? "" : "s", p->Name);

		// in the process of dying.
		p->Flags |= FLAG_DYING;
		for(auto t : p->Threads)
			Kill(t);

		Cleanup(p);
	}


	bool CurrentProcessInRing3()
	{
		if(GetCurrentThread() && GetCurrentThread()->Parent)
			return GetCurrentThread()->Parent->Flags & 0x1;

		return false;
	}

	Process* GetProcess(uint64_t pid)
	{
		// for(uint64_t i = 0; i < ProcessList->size(); i++)
		// {
		// 	if(ProcessList->get(i)->ProcessID == pid)
		// 		return ProcessList->get(i);
		// }

		for(auto p : *ProcessList)
		{
			if(p->ProcessID == pid)
				return p;
		}

		return 0;
	}

	Thread* GetThread(uint64_t tid)
	{
		auto queue = getRunQueue();
		queue->lock();

		Thread* ret = 0;

		for(int i = 0; i < NUM_PRIO; i++)
		{
			for(auto t : *getRunQueue()->queue[i])
			{
				if(t->ThreadID == tid)
					ret = t;
			}
		}

		for(auto t : *SleepList)
		{
			if(t->ThreadID == tid)
				ret = t;
		}

		queue->unlock();
		return ret;
	}
}
}
}




