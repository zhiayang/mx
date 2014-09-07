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
		if(Lock->owner == GetCurrentThread() && Lock->lock)
		{
			Lock->recursion++;
			return;
		}

		int64_t lindex = -1;
		if(Lock->lock)
		{
			if(Lock->contestants && Lock->contestants->size() >= MaxContestants)
			{
				while(Lock->contestants->size() >= MaxContestants)
					YieldCPU();
			}

			if(!Lock->contestants)
				Lock->contestants = new Library::LinkedList<Thread>();

			lindex = Lock->contestants->size();
			Lock->contestants->push_back(GetCurrentThread());
		}

		while(__sync_lock_test_and_set(&Lock->lock, 1))
			BLOCK();

		__sync_lock_test_and_set(&Lock->lock, 1);
		Lock->owner = GetCurrentThread();
		Lock->recursion = 1;

		if(lindex >= 0)
		Lock->contestants->RemoveAt(lindex);
	}

	void UnlockMutex(Mutex* Lock)
	{
		if(NumThreads <= 1){ return; }

		if(Lock->owner != GetCurrentThread())
			return;

		// check if this is but one dream in the sequence
		if(Lock->recursion > 1)
		{
			Lock->recursion--;
			return;
		}

		Thread* o = Lock->owner;

		Lock->owner = 0;
		Lock->recursion = 0;
		__sync_lock_release(&Lock->lock);

		if(Lock->contestants)
		{
			for(uint64_t i = 0; i < Lock->contestants->size(); i++)
			{
				Thread* t = Lock->contestants->get(i);

				if(t != o)
					WakeForMessage(t);
			}
		}
	}

	bool TryLockMutex(Mutex* Lock)
	{
		// if current was 1, since CheckLock gives us the old value then it's still locked.
		// since this is a trylock, return.
		if(__sync_lock_test_and_set(&Lock->lock, 1))
			return false;

		Lock->owner = GetCurrentThread();
		Lock->recursion = 1;

		// if not, we already locked it.
		return true;
	}
}
}





