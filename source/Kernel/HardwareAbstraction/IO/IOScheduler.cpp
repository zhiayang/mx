// IOScheduler.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <rdestl/list.h>

using namespace Library;
using namespace Kernel::HardwareAbstraction::Devices;

namespace Kernel {
namespace HardwareAbstraction {
namespace IO
{
	#define DIRECTOP	0

	struct IOTransfer
	{
		IOTransfer() { this->magic = 0xAF; }
		Multitasking::Thread* owningthread;
		IODevice* device;

		// args to storagedevice.
		uint64_t pos = 0;
		uint64_t out = 0;
		uint64_t count = 0;

		uint8_t magic = 0;
		bool blockop = 0;
		bool writeop = 0;
		bool completed = 0;
	};

	static rde::vector<IOTransfer>* Transfers;
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
				Transfers->erase(Transfers->begin());

				UNLOCK(listmtx);
				assert(req.magic == 0xAF);

				assert(req.completed == false);
				assert(req.device);
				assert(req.owningthread);

				// both operations should, at the lowest level, block until the operation is done.
				// it doesn't matter anyway (although this does mean that we can only do ops on one device at a time)
				// TODO (spawn new thread for each tertiary IO device (hdd)?)

				using namespace MemoryManager;
				if(req.writeop)
				{
					uint64_t outbuf = req.out;
					if(req.owningthread->Parent != Multitasking::GetProcess(0))
					{
						// if we're writing from userspace, we need to copy the buffer to kernel space *before* the write.
						outbuf = Virtual::AllocatePage((req.count + 0xFFF) / 0x1000);
						Virtual::CopyToKernel(req.out, outbuf, req.count, &req.owningthread->Parent->VAS);
					}

					IOResult iores = req.device->Write(req.pos, outbuf, req.count);
					(void) iores;

					if(outbuf != req.out)
					{
						Virtual::FreePage(outbuf, (req.count + 0xFFF) / 0x1000);
					}
				}
				else
				{

					IOResult iores = req.device->Read(req.pos, req.out, req.count);

					// if this is a read from kernel space, just do shit.
					if(req.owningthread->Parent == Multitasking::GetProcess(0))
					{
						Memory::Copy((void*) req.out, (void*) iores.allocatedBuffer.virt, req.count);
					}
					else
					{
						Virtual::CopyFromKernel(iores.allocatedBuffer.virt, req.out, req.count, &req.owningthread->Parent->VAS);
					}

					// if the bufferSizeInPages is zero, then we don't free anything
					// these buffers may be device specific, like NIC Rx/Tx buffers.
					if(iores.bufferSizeInPages > 0)
					{
						Physical::FreeDMA(iores.allocatedBuffer, iores.bufferSizeInPages);
					}
				}



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
		Transfers = new rde::vector<IOTransfer>();
		listmtx = new Mutex();

		if(!DIRECTOP)
			Multitasking::AddToQueue(Multitasking::CreateKernelThread(Scheduler, 2));
	}

	void Read(IODevice* dev, uint64_t pos, uint64_t buf, uint64_t bytes)
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

			req.device			= dev;
			req.pos				= pos;
			req.out				= buf;
			req.count			= bytes;

			req.owningthread	= Multitasking::GetCurrentThread();
			req.writeop 		= false;
			req.blockop			= true;


			LOCK(listmtx);
			Transfers->push_back(req);
			UNLOCK(listmtx);

			BLOCK();
		}
	}

	void Write(IODevice* dev, uint64_t pos, uint64_t buf, uint64_t bytes)
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
			req.device			= dev;
			req.pos				= pos;
			req.out				= buf;
			req.count			= bytes;

			req.owningthread	= Multitasking::GetCurrentThread();
			req.writeop 		= true;
			req.blockop			= true;

			LOCK(listmtx);
			Transfers->push_back(req);
			UNLOCK(listmtx);

			BLOCK();
		}
	}






	// non blocking.
	// returns void pointer (opaque type essentially) to check status.
	void* ScheduleRead(IODevice* dev, uint64_t pos, uint64_t buf, uint64_t bytes)
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

	void* ScheduleWrite(IODevice* dev, uint64_t pos, uint64_t buf, uint64_t bytes)
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







