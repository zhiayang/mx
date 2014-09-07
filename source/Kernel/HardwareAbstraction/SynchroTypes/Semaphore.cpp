// Semaphore.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
using namespace Kernel::HardwareAbstraction::Multitasking;

namespace Kernel
{
	AutoSemaphore::AutoSemaphore(Semaphore* _sem)
	{
		this->sem = _sem;
		assert(this->sem);
		AquireSemaphore(this->sem);
	}

	AutoSemaphore::AutoSemaphore(const AutoSemaphore& as)
	{
		this->sem = as.sem;
		assert(this->sem);
		AquireSemaphore(this->sem);
	}

	AutoSemaphore::~AutoSemaphore()
	{
		assert(this->sem);
		ReleaseSemaphore(this->sem);
	}

	void AquireSemaphore(Semaphore* sem)
	{
		if(NumThreads <= 1){ return; }

		assert(sem);

		// check if we already own this mutex
		if(sem->owner == GetCurrentThread())
		{
			sem->recursion++;
			return;
		}

		// check if we have zero shits left to give
		if(__sync_fetch_and_sub(&sem->value, 1) < 0)
		{
			// add ourselves to the waiting list
			if(!sem->contestants)
				sem->contestants = new Library::LinkedList<Thread>();

			sem->contestants->push_back(GetCurrentThread());
		}
	}


	void ReleaseSemaphore(Semaphore* sem)
	{

	}
	bool TrySemaphore(Semaphore* sem);
}
