// Scheduler.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#include <Kernel.hpp>
#include <StandardIO.hpp>
#include <Memory.hpp>

namespace Kernel {

// Mutex list
namespace Mutexes
{
	Mutex* ConsoleOutput;
	Mutex* SystemTime;
	Mutex* KernelHeap;
	Mutex* SerialLog;
	Mutex* WindowDispatcher;
	Mutex* TestMutex;

	void Initialise()
	{
		// initialise the mutexes.
		Mutexes::ConsoleOutput = new Mutex();
		Mutexes::KernelHeap = new Mutex();
		Mutexes::SystemTime = new Mutex();
		Mutexes::SerialLog = new Mutex();
		Mutexes::WindowDispatcher = new Mutex();

		Mutexes::TestMutex = new Mutex();
	}
}

AutoMutex::AutoMutex(Mutex* l) { lock = l; HardwareAbstraction::Multitasking::LockMutex(l); }
AutoMutex::AutoMutex(const AutoMutex& m) { this->lock = m.lock; HardwareAbstraction::Multitasking::LockMutex(this->lock); }
AutoMutex::~AutoMutex() { HardwareAbstraction::Multitasking::UnlockMutex(lock); }

static const uint8_t MaxContestants = 32;

namespace HardwareAbstraction {
namespace Multitasking
{
	void LockMutex(Mutex* Lock)
	{
		if(NumThreads <= 1){ return; }

		// check if we already own this mutex
		if(Lock->owner == GetCurrentThread()->ThreadID && Lock->lock)
		{
			Lock->recursion++;
			return;
		}

		if(Lock->lock)
		{
			if(Lock->numcontestants == MaxContestants)
			{
				while(Lock->numcontestants == MaxContestants)
					YieldCPU();
			}

			if(!Lock->contestants)
				Lock->contestants = new uint64_t[MaxContestants];

			Lock->contestants[Lock->numcontestants] = GetCurrentThread()->ThreadID;
			Lock->numcontestants++;
		}

		while(Lock->lock)
			BLOCK();

		Lock->lock = true;
		Lock->owner = GetCurrentThread()->ThreadID;
		Lock->recursion = 1;
	}

	void UnlockMutex(Mutex* Lock)
	{
		if(NumThreads <= 1){ return; }

		if(Lock->owner != GetCurrentThread()->ThreadID)
			return;


		// check if this is but one dream in the sequence
		if(Lock->recursion > 1)
		{
			Lock->recursion--;
			return;
		}

		uint64_t nc = Lock->numcontestants;
		uint64_t o = Lock->owner;

		Lock->owner = 0;
		Lock->recursion = 0;
		Lock->lock = false;
		Lock->numcontestants = 0;

		for(uint64_t i = 0; i < nc; i++)
		{
			if(Lock->contestants[i] != o)
				Multitasking::WakeForMessage(Multitasking::GetThread(Lock->contestants[i]));
		}
	}
}
}
}





