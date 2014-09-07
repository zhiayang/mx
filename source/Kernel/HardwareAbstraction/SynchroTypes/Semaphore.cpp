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
		if(NumThreads <= 1) { return; }
		assert(sem);

		// check if we have zero shits left to give
		if(__sync_fetch_and_sub(&sem->value, 1) < 0)
		{
			// we do
			// queue up for the toilet.

			// add ourselves to the waiting list
			if(!sem->contestants)
				sem->contestants = new Library::LinkedList<Thread>();

			sem->contestants->push_back(GetCurrentThread());
		}

		while(sem->value <= 0)
			BLOCK();
	}

	void ReleaseSemaphore(Semaphore* sem)
	{
		if(NumThreads <= 1) { return; }
		assert(sem);

		__sync_add_and_fetch(&sem->value, 1);
		if(sem->value < 0 && sem->contestants && sem->contestants->size() > 0)
			WakeForMessage(sem->contestants->pop_front());
	}

	bool TrySemaphore(Semaphore* sem)
	{
		if(NumThreads <= 1) { return true; }
		assert(sem);

		if(sem->value <= 0)
			return false;

		__sync_sub_and_fetch(&sem->value, 1);
		return true;
	}
}










