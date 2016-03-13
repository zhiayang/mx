// Multitasking.hpp
// Copyright (c) 2014 - 2016, zhiayang@gmail.com
// Licensed under Creative Commons Attribution ShareAlike 3.0 Unported.


#pragma once
#include <stdint.h>
#include "Filesystems.hpp"
#include "IPC.hpp"
#include "MemoryManager/Virtual.hpp"
#include <Synchro.hpp>
#include <defs/_pthreadstructs.h>
#include <stl/vector.h>

#include <signal.h>

extern "C" void YieldCPU();

namespace Kernel {

void Log(uint8_t level, const char* str, ...);
void Log(const char* str, ...);

namespace HardwareAbstraction
{
	namespace Multitasking
	{
		#define NUM_PRIO	4
		struct Process;

		struct Thread
		{
			pid_t ThreadID			= 0;
			uint64_t StackPointer	= 0;
			uint64_t TopOfStack		= 0;
			uint64_t StackSize		= 0;
			uint32_t Sleep			= 0;
			uint8_t Priority		= 0;
			uint8_t flags			= 0;
			uint16_t ExecutionTime	= 0;
			uint64_t State			= 0;

			void* tlsptr			= 0;
			Process* Parent			= 0;

			// a bit hacky, but this stores the current thread errno.
			int64_t currenterrno	= 0;

			rde::list<uintptr_t> messagequeue;

			ThreadRegisterState_type* CrashState = 0;

			rde::vector<Thread*> watchers;
			rde::vector<Thread*> watching;


			void* returnval = 0;
			void (*funcpointer)() = 0;

			Thread() { }

			Thread(const Thread&) = delete;
			Thread(Thread&&) = delete;

			Thread& operator =(const Thread&) = delete;
			Thread& operator =(Thread&&) = delete;
		};

		struct Process
		{
			explicit Process(MemoryManager::Virtual::PageMapStructure* pml) : VAS(pml) { }

			pid_t ProcessID	= 0;		// Process ID
			uint8_t Flags	= 0;
			size_t tlssize	= 0;
			char Name[64];		// Task's name

			Filesystems::IOContext iocontext;
			MemoryManager::Virtual::VirtualAddressSpace VAS;
			sighandler_t* SignalHandlers = 0;

			Process* Parent = 0;
			rde::vector<Thread*> Threads;
		};

		void DisableScheduler();
		void EnableScheduler();

		struct RunQueue
		{
			RunQueue()
			{
				this->thelock = new Mutex();
			}

			RunQueue(const RunQueue&) = delete;
			RunQueue& operator = (const RunQueue&) = delete;

			RunQueue(RunQueue&& copy)
			{
				this->thelock = copy.thelock;
				copy.thelock = 0;
			}

			RunQueue& operator = (RunQueue&& copy)
			{
				this->thelock = copy.thelock;
				copy.thelock = 0;
				return *this;
			}

			void lock()
			{
				DisableScheduler();
				LockMutex(*this->thelock);
			}

			void unlock()
			{
				UnlockMutex(*this->thelock);
				EnableScheduler();
			}

			Mutex* thelock;
			rde::vector<Thread*> queue[NUM_PRIO];
		};


		#define STATE_INVALID		0
		#define STATE_SUSPEND		1
		#define STATE_NORMAL		2
		#define STATE_AWAITDEATH	3
		#define STATE_BLOCKING		4

		#define STATE_DEAD			255

		#define BLOCK_MESSAGE		0

		#define FLAG_USERSPACE		0x1
		#define FLAG_DETACHED		0x2
		#define FLAG_DYING			0x80

		extern rde::vector<Process*> ProcessList;
		extern rde::vector<Thread*> SleepList;

		extern uint64_t NumThreads;
		extern uint64_t NumProcesses;
		extern bool SchedulerEnabled;

		void Initialise();
		RunQueue& GetRunQueue();
		uint64_t GetNumberOfThreads();
		uint64_t GetCurrentCR3();
		Thread* GetThread(pid_t id);
		Process* GetProcess(pid_t id);

		void SetTLS(uint64_t tlsptr);

		pid_t GetCurrentThreadID();
		pid_t GetCurrentProcessID();

		bool CurrentProcessInRing3();
		Thread* GetCurrentThread();
		Process* GetCurrentProcess();

		Thread* GetNextThread();
		void Sleep(int64_t Miliseconds);
		extern "C" void YieldCPU();
		void Block(uint8_t purpose = 0);
		rde::vector<Thread*>& GetThreadList(Thread* t);
		Thread* FetchAndRemoveThread(Thread* t);

		void Suspend(Thread* p);
		void Resume(Thread* p);
		void Kill(Thread* p);
		void TerminateCurrentThread(ThreadRegisterState_type* r);
		extern "C" void ExitThread();
		extern "C" void ExitThread_Userspace();

		void SetThreadErrno(int errno);


		void Suspend(Process* p);
		void Resume(Process* p);
		void Kill(Process* p);

		void Suspend(const char* p);
		void Resume(const char* p);
		void Kill(const char* p);

		void WatchThread(pthread_t tid);
		void UnwatchThread(pthread_t tid);

		void AddToQueue(Process* Proc);
		void AddToQueue(Thread* t);

		void WakeForMessage(Process* Process);
		void WakeForMessage(Thread* Thread);


		// lol @ overloads
		Thread* CreateThread(Process* Parent, void (*Function)(), uint8_t Priority = 1, void* p1 = 0, void* p2 = 0, void* p3 = 0, void* p4 = 0, void* p5 = 0, void* p6 = 0) __attribute__ ((warn_unused_result));

		Thread* CreateThread(Process* Parent, void (*Function)(), Thread_attr* attribs)
			__attribute__ ((warn_unused_result));

		Thread* CreateKernelThread(void (*Function)(), uint8_t Priority = 1, void* p1 = 0, void* p2 = 0, void* p3 = 0, void* p4 = 0, void* p5 = 0, void* p6 = 0) 	__attribute__ ((warn_unused_result));

		Process* CreateProcess(const char name[64], uint8_t Flags, void (*Function)(), uint8_t prio = 1, void* a1 = 0, void* a2 = 0, void* a3 = 0, void* a4 = 0, void* a5 = 0, void* a6 = 0)
			__attribute__ ((warn_unused_result));

		Process* CreateProcess(const char name[64], uint8_t Flags, size_t tlssize, void (*Function)(), uint8_t prio = 1, void* a1 = 0, void* a2 = 0, void* a3 = 0, void* a4 = 0, void* a5 = 0, void* a6 = 0)
			__attribute__ ((warn_unused_result));

		Process* ForkProcess(const char name[64], Thread_attr* attr) __attribute__ ((warn_unused_result));

	}
}
}
