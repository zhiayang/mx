// Scheduler.cpp
// Copyright (c) 2013 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#include <Kernel.hpp>
#include <errno.h>
#include <pthread.h>

using namespace Kernel::HardwareAbstraction::Multitasking;

namespace Kernel
{
	AutoMutex::AutoMutex(Mutex& l) : lock(l) { LockMutex(l); }
	AutoMutex::~AutoMutex() { UnlockMutex(this->lock); }
	AutoMutex& AutoMutex::operator = (AutoMutex&& other)
	{
		this->lock.nextOwner	= other.lock.nextOwner;
		this->lock.owner		= other.lock.owner;
		this->lock.lock			= other.lock.lock;
		this->lock.recursion	= other.lock.recursion;
		this->lock.type			= other.lock.type;

		return *this;
	}


	void LockMutex(Mutex& mtx)
	{
		if(NumThreads <= 1) { return; }

		// check if we already own this mutex
		if(mtx.owner == GetCurrentThread() && mtx.lock)
		{
			mtx.recursion++;
			return;
		}

		if(mtx.lock)
		{
			if(mtx.nextOwner == 0)
				mtx.nextOwner = GetCurrentThread();
		}

		while(__sync_lock_test_and_set(&mtx.lock, 1))
			BLOCK();

		__sync_lock_test_and_set(&mtx.lock, 1);
		mtx.owner = GetCurrentThread();
		mtx.recursion = 1;
	}

	void UnlockMutex(Mutex& mtx)
	{
		if(NumThreads <= 1) { return; }

		if(mtx.owner != GetCurrentThread())
			return;

		// check if this is but one dream in the sequence
		if(mtx.recursion > 1)
		{
			mtx.recursion--;
			return;
		}

		Thread* o = mtx.owner;

		mtx.owner = 0;
		mtx.recursion = 0;
		__sync_lock_release(&mtx.lock);

		if(mtx.nextOwner != 0)
		{
			if(mtx.nextOwner != o)
			{
				mtx.nextOwner = 0;
				WakeForMessage(mtx.nextOwner);
			}
			else
			{
				mtx.nextOwner = 0;
			}
		}
	}

	bool TryLockMutex(Mutex& mtx)
	{
		if(NumThreads <= 1) { return true; }

		// if current was 1, since CheckLock gives us the old value then it's still locked.
		// since this is a trylock, return.
		if(__sync_lock_test_and_set(&mtx.lock, 1))
			return false;

		mtx.owner = GetCurrentThread();
		mtx.recursion = 1;

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

		if(m->lock || m->nextOwner != 0)
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

		LockMutex(*m);
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

		UnlockMutex(*m);
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

		bool result = TryLockMutex(*m);
		if(result)
			return 0;

		else
		{
			SetThreadErrno(EBUSY);
			return -1;
		}
	}
}
























