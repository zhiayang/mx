// Scheduler.cpp
// Copyright (c) 2013 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#include <Kernel.hpp>
#include <errno.h>
#include <pthread.h>

using namespace Kernel::HardwareAbstraction::Multitasking;

namespace Kernel
{
	AutoMutex::AutoMutex(Mutex* l) { lock = l; LockMutex(l); }
	AutoMutex::AutoMutex(const AutoMutex& m) { this->lock = m.lock; LockMutex(this->lock); }
	AutoMutex::~AutoMutex() { UnlockMutex(lock); }


	void LockMutex(Mutex* Lock)
	{
		if(NumThreads <= 1) { return; }
		assert(Lock);

		// check if we already own this mutex
		if(Lock->owner == GetCurrentThread() && Lock->lock)
		{
			Lock->recursion++;
			return;
		}

		if(Lock->lock)
		{
			Lock->contestants.push_back(GetCurrentThread());
		}

		while(__sync_lock_test_and_set(&Lock->lock, 1))
			BLOCK();

		__sync_lock_test_and_set(&Lock->lock, 1);
		Lock->owner = GetCurrentThread();
		Lock->recursion = 1;
	}

	void UnlockMutex(Mutex* Lock)
	{
		if(NumThreads <= 1) { return; }
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

		if(Lock->contestants.size() > 0)
		{
			if(Lock->contestants.front() != o)
			{
				auto front = Lock->contestants.front();
				Lock->contestants.erase(Lock->contestants.begin());
				WakeForMessage(front);
			}
			else
			{
				Lock->contestants.erase(Lock->contestants.begin());
			}
		}
	}

	bool TryLockMutex(Mutex* Lock)
	{
		if(NumThreads <= 1) { return true; }
		assert(Lock);

		// if current was 1, since CheckLock gives us the old value then it's still locked.
		// since this is a trylock, return.
		if(__sync_lock_test_and_set(&Lock->lock, 1))
			return false;

		Lock->owner = GetCurrentThread();
		Lock->recursion = 1;

		// if not, we already locked it.
		return true;
	}



	// id-based, as usual.
	// for userspace.
	static rde::hash_map<pthread_mutex_t, Mutex*>* mtxmap;
	static pthread_mutex_t curmtxid = 0;

	static bool __checkmap(pthread_mutex_t id)
	{
		if(!mtxmap || mtxmap->find(id) == mtxmap->end())
		{
			SetThreadErrno(EINVAL);
			return false;
		}
		return true;
	}

	extern "C" void Syscall_CreateMutex(pthread_mutex_t* mtx, const pthread_mutexattr_t* attr)
	{
		*mtx = curmtxid++;
		if(!mtxmap)
			mtxmap = new rde::hash_map<pthread_mutex_t, Mutex*>();

		Mutex* m = new Mutex;
		if(attr != NULL)
			m->type = attr->type;

		else
			m->type = PTHREAD_MUTEX_DEFAULT;

		(*mtxmap)[*mtx] = m;
	}

	extern "C" void Syscall_DestroyMutex(pthread_mutex_t* mtx)
	{
		pthread_mutex_t id = *mtx;
		if(!__checkmap(id))
			return;

		Mutex* m = (*mtxmap)[id];
		assert(m);

		if(m->lock || m->contestants.size() > 0)
		{
			SetThreadErrno(EBUSY);
			return;
		}

		mtxmap->erase(id);
		delete m;
	}

	extern "C" int64_t Syscall_LockMutex(pthread_mutex_t* mtx)
	{
		assert(mtx);
		pthread_mutex_t id = *mtx;
		if(!__checkmap(id))
			return -1;

		// get the mutex object
		Mutex* m = (*mtxmap)[id];
		assert(m);

		// depending on our type:
		if(m->type == PTHREAD_MUTEX_NORMAL || m->type == PTHREAD_MUTEX_ERRORCHECK)
		{
			if(m->lock && m->owner == GetCurrentThread())
			{
				SetThreadErrno(EDEADLK);
				return -1;
			}
		}

		LockMutex(m);
		return 0;
	}

	extern "C" int64_t Syscall_UnlockMutex(pthread_mutex_t* mtx)
	{
		assert(mtx);
		pthread_mutex_t id = *mtx;
		if(!__checkmap(id))
			return -1;

		// get the mutex object
		Mutex* m = (*mtxmap)[id];
		assert(m);

		if(m->owner != GetCurrentThread())
		{
			SetThreadErrno(EPERM);
			return -1;
		}

		UnlockMutex(m);
		return 0;
	}

	extern "C" int64_t Syscall_TryLockMutex(pthread_mutex_t* mtx)
	{
		assert(mtx);
		pthread_mutex_t id = *mtx;
		if(!__checkmap(id))
			return -1;

		// get the mutex object
		Mutex* m = (*mtxmap)[id];
		assert(m);

		bool result = TryLockMutex(m);
		if(result)
			return 0;

		else
		{
			SetThreadErrno(EBUSY);
			return -1;
		}
	}
}
























