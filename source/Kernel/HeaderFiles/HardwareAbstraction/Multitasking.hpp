// Multitasking.hpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under Creative Commons Attribution ShareAlike 3.0 Unported.


#pragma once
#include <stdint.h>
#include "Filesystems.hpp"
#include "IPC.hpp"
#include "MemoryManager/Virtual.hpp"
#include <Mutexes.hpp>
#include <defs/_pthreadstructs.h>

#include <signal.h>

extern "C" void YieldCPU();
namespace Kernel {
namespace HardwareAbstraction
{
	namespace Multitasking
	{
		struct Process;

		struct Thread
		{
			uint64_t ThreadID;
			uint64_t StackPointer;
			uint64_t TopOfStack;
			uint8_t State;
			uint32_t Sleep;
			uint8_t Priority;
			uint64_t InstructionPointer;
			uint8_t flags;

			rde::list<uintptr_t>* messagequeue;
			uint16_t ExecutionTime;

			Process* Parent;
			ThreadRegisterState_type* CrashState;
			void* tlsptr;

			void* returnval;
			void (*Thread)();
		};

		struct Process
		{
			uint64_t ProcessID;			// Process ID
			uint8_t Flags;
			uint64_t CR3;
			char Name[64];				// Task's name
			size_t tlssize;

			Filesystems::IOContext* iocontext;

			rde::vector<uint64_t>* AllocatedPageList;
			MemoryManager::Virtual::VirtualAddressSpace* VAS;
			sighandler_t* SignalHandlers;

			Process* Parent;
			rde::list<Thread*>* Threads;
		};


		#define STATE_SUSPEND		0
		#define STATE_NORMAL		1
		#define STATE_AWAITDEATH		2
		#define STATE_BLOCKING		3
		#define STATE_DEAD			255

		#define BLOCK_MESSAGE		0

		#define FLAG_USERSPACE		0x1
		#define FLAG_DETACHED		0x2


		extern rde::list<Process*>* ProcessList;
		extern rde::list<Thread*>* SleepList;

		extern rde::list<Thread*>* ThreadList_LowPrio;
		extern rde::list<Thread*>* ThreadList_NormPrio;
		extern rde::list<Thread*>* ThreadList_HighPrio;

		extern Mutex* listlock;

		extern uint64_t NumThreads;
		extern uint64_t NumProcesses;
		extern bool SchedulerEnabled;

		void Initialise();
		uint64_t GetNumberOfThreads();
		uint64_t GetCurrentCR3();
		Thread* GetThread(uint64_t id);
		Process* GetProcess(uint64_t id);

		void SetTLS(uint64_t tlsptr);

		uint64_t GetCurrentThreadID();
		uint64_t GetCurrentProcessID();

		bool CurrentProcessInRing3();
		Thread* GetCurrentThread();
		Process* GetCurrentProcess();

		Thread* GetNextThread();
		void Sleep(int64_t Miliseconds);
		extern "C" void YieldCPU();
		void Block(uint8_t purpose = 0);
		rde::list<Thread*>* GetThreadList(Thread* t);
		Thread* FetchAndRemoveThread(Thread* t);

		void Suspend(Thread* p);
		void Resume(Thread* p);
		void Kill(Thread* p);
		void TerminateCurrentThread(ThreadRegisterState_type* r);
		extern "C" void ExitThread();
		extern "C" void ExitThread_Userspace();

		void Suspend(Process* p);
		void Resume(Process* p);
		void Kill(Process* p);

		void Suspend(const char* p);
		void Resume(const char* p);
		void Kill(const char* p);

		void DisableScheduler();
		void EnableScheduler();


		rde::list<Thread*>* SearchByName(const char* n);
		Thread* GetProcessByName(const char* n);

		void AddToQueue(Process* Proc);
		void AddToQueue(Thread* t);

		void WakeForMessage(Process* Process);
		void WakeForMessage(Thread* Thread);


		Thread* CreateThread(Process* Parent, void (*Function)(), uint8_t Priority = 1, void* p1 = 0, void* p2 = 0, void* p3 = 0, void* p4 = 0, void* p5 = 0, void* p6 = 0) __attribute__ ((warn_unused_result));

		Thread* CreateThread(Process* Parent, void (*Function)(), Thread_attr* attribs)
			__attribute__ ((warn_unused_result));

		Thread* CreateKernelThread(void (*Function)(), uint8_t Priority = 1, void* p1 = 0, void* p2 = 0, void* p3 = 0, void* p4 = 0, void* p5 = 0, void* p6 = 0) 	__attribute__ ((warn_unused_result));



		Process* CreateProcess(const char name[64], uint8_t Flags, void (*Function)())
			__attribute__ ((warn_unused_result));

		Process* CreateProcess(const char name[64], uint8_t Flags, void (*Function)(), uint8_t prio = 1, void* a1 = 0, void* a2 = 0, void* a3 = 0, void* a4 = 0, void* a5 = 0, void* a6 = 0)
			__attribute__ ((warn_unused_result));

		Process* CreateProcess(const char name[64], uint8_t Flags, size_t tlssize, void (*Function)(), uint8_t prio = 1, void* a1 = 0, void* a2 = 0, void* a3 = 0, void* a4 = 0, void* a5 = 0, void* a6 = 0)
			__attribute__ ((warn_unused_result));




		void LockMutex(uint64_t* Lock);
		void UnlockMutex(uint64_t* Lock);

		void LockMutex(Mutex* Lock);
		void UnlockMutex(Mutex* Lock);
	}
}
}
