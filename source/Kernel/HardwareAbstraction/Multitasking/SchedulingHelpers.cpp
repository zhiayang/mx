// Scheduler.cpp
// Copyright (c) 2013 - 2016, zhiayang@gmail.com
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











	rde::vector<Thread*>& GetThreadList(Thread* t)
	{
		return GetRunQueue().queue[t->Priority];
	}

	Thread* FetchAndRemoveThread(Thread* thread)
	{
		assert(thread);
		GetRunQueue().lock();

		GetThreadList(thread).remove(thread);

		GetRunQueue().unlock();
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
		GetRunQueue().lock();
		assert(thread);

		auto list = GetThreadList(thread);
		if(thread->State != STATE_BLOCKING && thread->State != STATE_SUSPEND)
		{
			assert(list.contains(thread));
			// list->insert(list->begin(), FetchAndRemoveThread(thread));
			list.push_back(FetchAndRemoveThread(thread));
		}
		else
		{
			assert(SleepList.contains(thread));
			// thread is blocking, shoo it out of the blocking queue.

			SleepList.remove(thread);
			thread->State = STATE_NORMAL;
			// GetThreadList(thread)->insert(GetThreadList(thread)->begin(), thread);
			GetThreadList(thread).push_back(thread);
		}

		GetRunQueue().unlock();
		YieldCPU();
	}


	void Block(uint8_t purpose)
	{
		GetRunQueue().lock();
		if(GetCurrentThread()->State == STATE_BLOCKING)
		{
			Log("BLOCK (%d, %d, %x)", GetCurrentThread()->ThreadID, GetCurrentThread()->State, __builtin_return_address(0));
			HALT("");
		}

		Thread* thread = FetchAndRemoveThread(GetCurrentThread());

		thread->State = STATE_BLOCKING;
		SleepList.push_back(thread);

		(void) purpose;
		GetRunQueue().unlock();
		YieldCPU();
	}











	void AddToQueue(Process* Proc)
	{
		ProcessList.push_back(Proc);
		for(auto t : Proc->Threads)
			AddToQueue(t);
	}

	void AddToQueue(Thread* t)
	{
		GetRunQueue().lock();
		GetRunQueue().queue[t->Priority].push_back(t);
		GetRunQueue().unlock();
	}

	extern "C" void Syscall_TerminateCrashedThread()
	{
		TerminateCurrentThread(0);
	}


	void TerminateCurrentThread(ThreadRegisterState_type* r)
	{
		Thread* p = GetCurrentThread();
		if(!p)
			return;

		Log("Killed thread %d, name: %s", p->ThreadID, p->Parent->Name);
		Kill(p);

		r = p->CrashState;

		// this is where we would say do a core dump.
		// Utilities::StackDump((uint64_t*) r->__rsp, 10);
		// Utilities::GenerateStackTrace(r->__rsp, 10);
		YieldCPU();

		while(true);
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

		// close all files
		Filesystems::CloseAll(p);


		// destroy its address space
		MemoryManager::Virtual::DestroyVAS(&p->VAS);
		Log("Cleaned up process %s", p->Name);

		delete p;
	}




	// direct
	void Suspend(Thread* p)
	{
		GetRunQueue().lock();
		if(p && p->State == STATE_NORMAL)
		{
			Log("Suspended thread %d, name: %s", p->ThreadID, p->Parent->Name);

			// GetThreadList(p)->RemoveAt((uint64_t) GetThreadList(p)->IndexOf(p));

			assert(p);
			GetThreadList(p).remove(p);
			SleepList.push_back(p);
			p->State = STATE_SUSPEND;
		}

		GetRunQueue().unlock();
	}

	void Resume(Thread* p)
	{
		GetRunQueue().lock();
		if(p && p->State == STATE_SUSPEND)
		{
			Log("Resumed thread %d, name: %s", p->ThreadID, p->Parent->Name);

			p->Sleep = 0;
			p->State = STATE_NORMAL;
		}
		GetRunQueue().unlock();
	}

	void Kill(Thread* p)
	{
		if(p && p->State == STATE_NORMAL)
		{
			GetRunQueue().lock();
			p->State = STATE_AWAITDEATH;

			// remove the thread from its parent process's list.
			Process* par = p->Parent;
			par->Threads.remove(p);

			GetThreadList(p).remove(p);
			SleepList.push_back(p);


			if(!(par->Flags & FLAG_DYING) && par->Threads.empty())
			{
				Log("Killed all threads of process %s, cleaning up", par->Name);
				Cleanup(par);
			}

			GetRunQueue().unlock();


			// wake up the watching threads.
			for(size_t i = 0, s = p->watchers.size(); i < s; i++)
			{
				Thread* w = p->watchers.front();
				p->watchers.erase(p->watchers.begin());

				w->watching.remove(p);
				WakeForMessage(w);
			}
		}
		else if(p && (p->State == STATE_BLOCKING || p->State == STATE_SUSPEND))
		{
			assert(SleepList.contains(p));
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
		Log("Killing process %s, disposing of %d thread%s", p->Name, p->Threads.size(), p->Threads.size() == 1 ? "" : "s");

		// in the process of dying.
		// p->Flags |= FLAG_DYING;

		// the flag stops Kill(thread) from calling Cleanup() on the parent process.
		for(auto t : p->Threads)
			Kill(t);

		// Cleanup(p);
	}


	bool CurrentProcessInRing3()
	{
		if(GetCurrentThread() && GetCurrentThread()->Parent)
			return GetCurrentThread()->Parent->Flags & 0x1;

		return false;
	}

	Process* GetProcess(pid_t pid)
	{
		for(auto p : ProcessList)
		{
			if(p->ProcessID == pid)
				return p;
		}

		return 0;
	}

	Thread* GetThread(pid_t tid)
	{
		auto& runqueue = GetRunQueue();
		runqueue.lock();

		Thread* ret = 0;

		for(int i = 0; i < NUM_PRIO; i++)
		{
			for(auto t : runqueue.queue[i])
			{
				if(t->ThreadID == tid)
					ret = t;
			}
		}

		for(auto t : SleepList)
		{
			if(t->ThreadID == tid)
				ret = t;
		}

		runqueue.unlock();
		return ret;
	}
}
}
}




