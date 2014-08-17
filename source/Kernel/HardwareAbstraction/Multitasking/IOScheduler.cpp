// IOScheduler.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <rdestl/list.h>

using namespace Library;
using namespace Kernel::HardwareAbstraction::Devices::Storage;

namespace Kernel {
namespace HardwareAbstraction {
namespace IO
{
	#define MAX_TRANSFERS	-1

	struct IOTransfer
	{
		IOTransfer(StorageDevice* d, uint64_t p, uint64_t b, uint64_t s) : device(d), pos(p), out(b), count(s) { }
		Multitasking::Thread* owningthread;
		StorageDevice* device;

		// args to storagedevice.
		uint64_t pos;
		uint64_t out;
		uint64_t count;

		bool blockop;
		bool writeop;
		bool completed;
	};

	static rde::list<IOTransfer*>* Transfers;
	static Mutex* listmtx;

	static void Scheduler()
	{
		while(true)
		{
			if(Transfers->size() == 0)
				YieldCPU();

			else
			{
				// get a lock
				LOCK(listmtx);

				// get the next request, then run it.
				IOTransfer* req = Transfers->front();
				Transfers->pop_front();

				assert(req->completed == false);

				// both operations should, at the lowest level, block until the operation is done.
				// it doesn't matter anyway (although this does mean that we can only do ops on one device at a time)
				// TODO.
				if(req->writeop)
					req->device->Write(req->pos, req->out, req->count);

				else
					req->device->Read(req->pos, req->out, req->count);

				// it's most probably done.
				req->completed = true;

				// wake the calling thread from its sleep.
				if(req->blockop)
				{
					assert(req->owningthread);
					Multitasking::WakeForMessage(req->owningthread);
				}

				UNLOCK(listmtx);
			}
		}
	}









	void Initialise()
	{
		Transfers = new rde::list<IOTransfer*>();
		listmtx = new Mutex();
		Multitasking::AddToQueue(Multitasking::CreateKernelThread(Scheduler, 2));
	}

	void Read(StorageDevice* dev, uint64_t pos, uint64_t buf, uint64_t bytes)
	{
		// only returns when the data is read, therefore is blocking.
		assert(dev);
		if(bytes == 0 || buf == 0)
		{
			HALT("");
			return;
		}

		IOTransfer* req = new IOTransfer(dev, pos, buf, bytes);

		req->owningthread = Multitasking::GetCurrentThread();
		req->writeop = false;
		req->blockop = true;
		req->completed = false;

		LOCK(listmtx);
		Transfers->push_back(req);
		UNLOCK(listmtx);

		BLOCK();
		assert(req->completed);

		delete req;
		return;
	}

	void Write(StorageDevice* dev, uint64_t pos, uint64_t buf, uint64_t bytes)
	{
		// only returns when the data is read, therefore is blocking.
		assert(dev);
		if(bytes == 0 || buf == 0)
			return;

		IOTransfer* req = new IOTransfer(dev, pos, buf, bytes);

		req->owningthread = Multitasking::GetCurrentThread();
		req->writeop = true;
		req->blockop = true;
		req->completed = false;

		LOCK(listmtx);
		Transfers->push_back(req);
		UNLOCK(listmtx);

		BLOCK();

		assert(req->completed == true);
		delete req;
		return;
	}






	// non blocking.
	// returns void pointer (opaque type essentially) to check status.
	void* ScheduleRead(StorageDevice* dev, uint64_t pos, uint64_t buf, uint64_t bytes)
	{
		// only returns when the data is read, therefore is blocking.
		assert(dev);
		if(bytes == 0 || buf == 0)
			return nullptr;

		IOTransfer* req = new IOTransfer(dev, pos, buf, bytes);

		req->owningthread = Multitasking::GetCurrentThread();
		req->writeop = false;
		req->blockop = false;
		req->completed = false;

		auto m = AutoMutex(listmtx);
		Transfers->push_back(req);
		return (void*) req;
	}

	void* ScheduleWrite(StorageDevice* dev, uint64_t pos, uint64_t buf, uint64_t bytes)
	{
		// only returns when the data is read, therefore is blocking.
		assert(dev);
		if(bytes == 0 || buf == 0)
			return nullptr;

		IOTransfer* req = new IOTransfer(dev, pos, buf, bytes);

		req->owningthread = Multitasking::GetCurrentThread();
		req->writeop = true;
		req->blockop = false;
		req->completed = false;

		auto m = AutoMutex(listmtx);
		Transfers->push_back(req);
		return (void*) req;
	}

	// if it returns true, expect the object to have been deleted.
	// polling of some kind (ie. don't use async kids)
	bool CheckStatus(void* request)
	{
		IOTransfer* req = (IOTransfer*) request;
		assert(req);
		assert(req->owningthread);
		assert(req->device);

		bool comp = req->completed;
		if(comp)
			delete req;

		return comp;
	}
}
}
}







