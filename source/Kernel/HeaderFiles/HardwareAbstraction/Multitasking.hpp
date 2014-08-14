// Multitasking.hpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under Creative Commons Attribution ShareAlike 3.0 Unported.


#pragma once
#include <stdint.h>
#include "Filesystems.hpp"
#include "IPC.hpp"
#include "MemoryManager/Virtual.hpp"
#include <List.hpp>
#include <Mutexes.hpp>
#include <Vector.hpp>

#include <signal.h>

extern "C" void YieldCPU();
namespace Kernel {
namespace HardwareAbstraction
{
	namespace Multitasking
	{
		enum class ThreadType
		{
			NormalApplication,
			BackgroundService
		};

		struct Process;

		struct ThreadRegisterState_type
		{
			uint64_t rdi, rsi, rbp;
			uint64_t rax, rbx, rcx, rdx;
			uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
		};

		struct Thread
		{
			uint64_t ThreadID;
			uint64_t StackPointer;
			uint64_t TopOfStack;
			uint8_t State;
			uint32_t Sleep;
			uint8_t Priority;
			uint64_t InstructionPointer;
			ThreadType Type;

			uint16_t ExecutionTime;

			uint64_t CurrentSharedMemoryOffset;
			Library::LinkedList<IPC::SimpleMessage>* SimpleMessageQueue;
			Process* Parent;
			ThreadRegisterState_type CrashState;

			void (*Thread)();
		};


		struct Process
		{
			uint64_t ProcessID;			// Process ID
			uint8_t Flags;
			uint64_t CR3;
			uint64_t DataPagePhys;
			char Name[64];				// Task's name

			uint64_t CurrentSharedMemoryOffset;
			Library::LinkedList<IPC::SimpleMessage>* SimpleMessageQueue;
			Filesystems::VFS::FileDescriptor* FileDescriptors;
			Library::Vector<uint64_t>* AllocatedPageList;
			MemoryManager::Virtual::VirtualAddressSpace* VAS;
			uint64_t CurrentFDIndex;
			sighandler_t* SignalHandlers;

			Process* Parent;
			Library::LinkedList<Thread>* Threads;
		};


		#define STATE_SUSPEND		0
		#define STATE_NORMAL		1
		#define STATE_AWAITDEATH		2
		#define STATE_BLOCKING		3

		#define BLOCK_MESSAGE		0


		extern Library::LinkedList<Process>* ProcessList;
		extern Library::LinkedList<Thread>* SleepList;

		extern Library::LinkedList<Thread>* ThreadList_LowPrio;
		extern Library::LinkedList<Thread>* ThreadList_NormPrio;
		extern Library::LinkedList<Thread>* ThreadList_HighPrio;

		extern Mutex* listlock;

		extern uint64_t NumThreads;
		extern uint64_t NumProcesses;
		extern bool SchedulerEnabled;

		void Initialise();
		uint64_t GetNumberOfThreads();
		uint64_t GetCurrentCR3();
		Thread* GetThread(uint64_t id);
		Process* GetProcess(uint64_t id);

		uint64_t GetCurrentThreadID();
		uint64_t GetCurrentProcessID();

		bool CurrentProcessInRing3();
		Thread* GetCurrentThread();
		Process* GetCurrentProcess();

		Thread* GetNextThread();
		void Sleep(int64_t Miliseconds);
		extern "C" void YieldCPU();
		void Block(uint8_t purpose = 0);
		Library::LinkedList<Thread>* GetThreadList(Thread* t);
		Thread* FetchAndRemoveThread(Thread* t);

		void Suspend(Thread* p);
		void Resume(Thread* p);
		void Kill(Thread* p);
		void TerminateCurrentThread(ThreadRegisterState_type* r);
		extern "C" void ExitThread();

		void Suspend(Process* p);
		void Resume(Process* p);
		void Kill(Process* p);

		void Suspend(const char* p);
		void Resume(const char* p);
		void Kill(const char* p);

		void DisableScheduler();
		void EnableScheduler();


		Library::LinkedList<Thread>* SearchByName(const char* n);
		Thread* GetProcessByName(const char* n);

		void AddToQueue(Process* Proc);
		void AddToQueue(Thread* t);

		void WakeForMessage(Process* Process);
		void WakeForMessage(Thread* Thread);


		Thread* CreateThread(Process* Parent, void (*Function)(), uint8_t Priority = 1, void* p1 = 0, void* p2 = 0, void* p3 = 0, void* p4 = 0, void* p5 = 0, void* p6 = 0) __attribute__ ((warn_unused_result));

		Thread* CreateKernelThread(void (*Function)(), uint8_t Priority = 1, void* p1 = 0, void* p2 = 0, void* p3 = 0, void* p4 = 0, void* p5 = 0, void* p6 = 0) 	__attribute__ ((warn_unused_result));

		Process* CreateProcess(const char name[64], uint8_t Flags, void (*Function)(), uint8_t Priority = 1)
			__attribute__ ((warn_unused_result));

		void LockMutex(uint64_t* Lock);
		void UnlockMutex(uint64_t* Lock);

		void LockMutex(Mutex* Lock);
		void UnlockMutex(Mutex* Lock);
	}
}
}
