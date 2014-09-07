// Scheduler.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#include <Kernel.hpp>

using namespace Kernel::HardwareAbstraction::Multitasking;

namespace Kernel
{
	AutoMutex::AutoMutex(Mutex* l) { lock = l; LockMutex(l); }
	AutoMutex::AutoMutex(const AutoMutex& m) { this->lock = m.lock; LockMutex(this->lock); }
	AutoMutex::~AutoMutex() { UnlockMutex(lock); }

	void LockMutex(Mutex* Lock)
	{
		if(NumThreads <= 1){ return; }
		assert(Lock);

		// check if we already own this mutex
		if(Lock->owner == GetCurrentThread() && Lock->lock)
		{
			Lock->recursion++;
			return;
		}

		if(Lock->lock)
		{
			if(!Lock->contestants)
				Lock->contestants = new Library::LinkedList<Thread>();

			Lock->contestants->push_back(GetCurrentThread());
		}

		while(__sync_lock_test_and_set(&Lock->lock, 1))
			BLOCK();

		__sync_lock_test_and_set(&Lock->lock, 1);
		Lock->owner = GetCurrentThread();
		Lock->recursion = 1;
	}

	void UnlockMutex(Mutex* Lock)
	{
		if(NumThreads <= 1){ return; }
		assert(Lock);

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

		if(Lock->contestants && Lock->contestants->size() > 0)
		{
			if(Lock->contestants->front() != o)
				WakeForMessage(Lock->contestants->pop_front());

			else
				Lock->contestants->pop_front();
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





