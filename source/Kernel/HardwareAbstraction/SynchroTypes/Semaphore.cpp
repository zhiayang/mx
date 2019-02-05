// Semaphore.cpp
// Copyright (c) 2014 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
using namespace Kernel::HardwareAbstraction::Multitasking;

namespace Kernel
{
	AutoSemaphore::AutoSemaphore(Semaphore& _sem) : sem(_sem)
	{
		AquireSemaphore(this->sem);
	}

	AutoSemaphore& AutoSemaphore::operator = (AutoSemaphore&& other)
	{
		this->sem.contestants	= other.sem.contestants;
		this->sem.value			= other.sem.value;

		AquireSemaphore(this->sem);
		return *this;
	}

	AutoSemaphore::~AutoSemaphore()
	{
		ReleaseSemaphore(this->sem);
	}

	void AquireSemaphore(Semaphore& sem)
	{
		if(NumThreads <= 1) { return; }

		// check if we have zero shits left to give
		if(__sync_fetch_and_sub(&sem.value, 1) < 0)
		{
			// we do
			// queue up for the toilet.

			// add ourselves to the waiting list
			sem.contestants.push_back(GetCurrentThread());
		}

		while(sem.value <= 0)
			BLOCK();
	}

	void ReleaseSemaphore(Semaphore& sem)
	{
		if(NumThreads <= 1) { return; }

		__sync_add_and_fetch(&sem.value, 1);
		if(sem.value < 0 && sem.contestants.size() > 0)
		{
			auto front = sem.contestants.front();
			sem.contestants.erase(sem.contestants.begin());
			WakeForMessage(front);
		}
	}

	bool TrySemaphore(Semaphore& sem)
	{
		if(NumThreads <= 1) { return true; }

		if(sem.value <= 0)
			return false;

		__sync_sub_and_fetch(&sem.value, 1);
		return true;
	}
}










