// Scheduler.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#include <Kernel.hpp>
#include <StandardIO.hpp>
#include <Memory.hpp>

namespace Kernel {

AutoMutex::AutoMutex(Mutex* l) { lock = l; Mutexes::LockMutex(l); }
AutoMutex::AutoMutex(const AutoMutex& m) { this->lock = m.lock; Mutexes::LockMutex(this->lock); }
AutoMutex::~AutoMutex() { Mutexes::UnlockMutex(lock); }

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

	using namespace Kernel::HardwareAbstraction::Multitasking;
	static const uint8_t MaxContestants = 32;
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
				WakeForMessage(GetThread(Lock->contestants[i]));
		}
	}

	bool TryLockMutex(Mutex* Lock)
	{
		// uint64_t current = __sync_lock_test_and_set(&Lock->lock, 1);

		// if current was 1, since CheckLock gives us the old value then it's still locked.
		// since this is a trylock, return.
		if(Lock->lock)
			return false;

		Lock->lock = 1;
		Lock->owner = GetCurrentThread()->ThreadID;
		Lock->recursion = 1;

		// if not, we already locked it.
		return true;
	}
}
}





