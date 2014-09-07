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
	#define DIRECTOP	0

	struct IOTransfer
	{
		IOTransfer() { }
		Multitasking::Thread* owningthread;
		StorageDevice* device;

		// args to storagedevice.
		uint64_t pos;
		uint64_t out;
		uint64_t count;

		bool blockop;
		bool writeop;
		bool completed;

		uint64_t pad;
	};

	static rde::list<IOTransfer>* Transfers;
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
				IOTransfer req = Transfers->front();
				Transfers->pop_front();

				UNLOCK(listmtx);

				assert(req.completed == false);
				assert(req.device);
				assert(req.owningthread);

				// both operations should, at the lowest level, block until the operation is done.
				// it doesn't matter anyway (although this does mean that we can only do ops on one device at a time)
				// TODO.
				if(req.writeop)
					req.device->Write(req.pos, req.out, req.count);

				else
					req.device->Read(req.pos, req.out, req.count);

				// it's most probably done.
				req.completed = true;

				// wake the calling thread from its sleep.
				if(req.blockop)
				{
					assert(req.owningthread);
					Multitasking::WakeForMessage(req.owningthread);
				}
			}
		}
	}









	void Initialise()
	{
		Transfers = new rde::list<IOTransfer>();
		listmtx = new Mutex();

		if(!DIRECTOP)
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

		if(DIRECTOP)
		{
			dev->Read(pos, buf, bytes);
		}
		else
		{
			IOTransfer req;

			req.device		= dev;
			req.pos			= pos;
			req.out			= buf;
			req.count		= bytes;

			req.owningthread	= Multitasking::GetCurrentThread();
			req.writeop 		= false;
			req.blockop		= true;


			// LOCK(listmtx);
			Transfers->push_back(req);
			// UNLOCK(listmtx);

			BLOCK();
		}
	}

	void Write(StorageDevice* dev, uint64_t pos, uint64_t buf, uint64_t bytes)
	{
		// only returns when the data is read, therefore is blocking.
		assert(dev);
		if(bytes == 0 || buf == 0)
			return;

		if(DIRECTOP)
		{
			dev->Write(pos, buf, bytes);
		}
		else
		{
			IOTransfer req;
			if(pos == 0x1D6A)
			{
				MemoryManager::KernelHeap::Print();
				UHALT();
			}
			req.device		= dev;
			req.pos			= pos;
			req.out			= buf;
			req.count		= bytes;

			req.owningthread	= Multitasking::GetCurrentThread();
			req.writeop 		= true;
			req.blockop		= true;

			LOCK(listmtx);
			Transfers->push_back(req);
			UNLOCK(listmtx);

			BLOCK();
		}
	}






	// non blocking.
	// returns void pointer (opaque type essentially) to check status.
	void* ScheduleRead(StorageDevice* dev, uint64_t pos, uint64_t buf, uint64_t bytes)
	{
		// // only returns when the data is read, therefore is blocking.
		// assert(dev);
		// if(bytes == 0 || buf == 0)
		// 	return nullptr;

		// IOTransfer* req = new IOTransfer();
		// req->device	= dev;
		// req->pos	= pos;
		// req->out	= buf;
		// req->count	= bytes;

		// req->owningthread = Multitasking::GetCurrentThread();
		// req->writeop = false;
		// req->blockop = false;
		// req->completed = false;

		// auto m = AutoMutex(listmtx);
		// Transfers->push_back(req);
		// return (void*) req;

		(void) dev;
		(void) pos;
		(void) buf;
		(void) bytes;
		return 0;
	}

	void* ScheduleWrite(StorageDevice* dev, uint64_t pos, uint64_t buf, uint64_t bytes)
	{
		// // only returns when the data is read, therefore is blocking.
		// assert(dev);
		// if(bytes == 0 || buf == 0)
		// 	return nullptr;

		// IOTransfer* req = new IOTransfer();
		// req->device	= dev;
		// req->pos	= pos;
		// req->out	= buf;
		// req->count	= bytes;

		// req->owningthread = Multitasking::GetCurrentThread();
		// req->writeop = true;
		// req->blockop = false;
		// req->completed = false;

		// auto m = AutoMutex(listmtx);
		// Transfers->push_back(req);
		// return (void*) req;

		(void) dev;
		(void) pos;
		(void) buf;
		(void) bytes;
		return 0;
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







