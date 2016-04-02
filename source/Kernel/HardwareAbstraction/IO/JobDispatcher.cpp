// JobDispatcher.cpp
// Copyright (c) 2014 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <Synchro.hpp>

using namespace Kernel::HardwareAbstraction;

namespace Kernel {
namespace JobDispatch
{
	static Mutex queueMtx;
	static rde::list<Job>* jobQueue = 0;


	void AddJob(Job job)
	{
		LOCK(queueMtx);
		jobQueue->push_back(job);
		UNLOCK(queueMtx);
	}

	void RemoveJob(Job* job)
	{
		// todo.
		(void) job;
	}


	void DispatcherThread()
	{
		while(true)
		{
			size_t s = jobQueue->size();
			for(size_t i = 0; i < s; i++)
			{
				// Log("in job dispatcher, delta %ld", Kernel::TickCounter() - __debug_flag__);
				// __debug_flag__ = Kernel::TickCounter();

				LOCK(queueMtx);
				Job job = jobQueue->pop_front();
				UNLOCK(queueMtx);


				// Log("dispatching job, delta %ld", Kernel::TickCounter() - __debug_flag__);
				// __debug_flag__ = Kernel::TickCounter();

				job.handle(job.param);
			}

			YieldCPU();
		}
	}

	void Initialise()
	{
		jobQueue = new rde::list<Job>();
		// queueMtx = new Mutex();

		Multitasking::Thread* th = Multitasking::CreateKernelThread(DispatcherThread, 2);
		Multitasking::AddToQueue(th);
	}

}
}



