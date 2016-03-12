// DMA.cpp
// Copyright (c) 2013 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <HardwareAbstraction/Devices/StorageDevice.hpp>
#include <HardwareAbstraction/Interrupts.hpp>
#include <HardwareAbstraction/Devices/IOPort.hpp>
#include <Memory.hpp>
#include <StandardIO.hpp>

#include <orion.h>

using namespace Kernel::HardwareAbstraction::MemoryManager;

namespace Kernel {
namespace HardwareAbstraction {
namespace Devices {
namespace Storage {
namespace ATA {
namespace DMA
{
	const uint8_t DMACommandRead		= 0x8;
	const uint8_t DMACommandStart		= 0x1;
	const uint8_t DMACommandStop		= 0x0;
	const uint8_t DMACommandWrite		= 0x0;

	const uint8_t ATA_ReadSectors28DMA	= 0xC8;
	const uint8_t ATA_ReadSectors48DMA	= 0x25;
	const uint8_t ATA_WriteSectors28DMA	= 0xCA;
	const uint8_t ATA_WriteSectors48DMA	= 0x35;

	const uint16_t PrimaryControl		= 0x3F6;
	const uint16_t SecondaryControl		= 0x376;

	static volatile bool _WaitingDMA14	= false;
	static volatile bool _WaitingDMA15	= false;

	static ATADrive* PreviousDevice = 0;

	struct PRDEntry
	{
		uint32_t bufferPhysAddr;
		uint16_t byteCount;
		uint16_t lastEntry;

	} __attribute__((packed));

	struct PRDTableCache
	{
		DMAAddr address;
		uint32_t length;
		uint32_t used;
	};

	#define MaxCachedTables 16
	static rde::vector<PRDTableCache> cachedPRDTables;

	void Initialise()
	{
		using namespace Kernel::HardwareAbstraction::Devices::PCI;
		PCIDevice* ata = PCI::GetDeviceByClassSubclass(01, 01);

		assert(ata);

		// enable bus mastering
		uint32_t f = ata->GetRegisterData(0x4, 0, 2);
		ata->WriteRegisterData(0x4, 0, 2, f | 0x4);

		uint32_t mmio = (uint32_t) ata->GetBAR(4);
		assert(ata->IsBARIOPort(4));

		cachedPRDTables = rde::vector<PRDTableCache>();

		for(int i = 0; i < MaxCachedTables; i++)
		{
			// precreate these
			PRDTableCache tcache;
			tcache.address = Physical::AllocateDMA(1);
			tcache.length = 0x1000;
			tcache.used = 0;

			cachedPRDTables.push_back(tcache);
		}

		IOPort::WriteByte((uint16_t) mmio + 2, 0x4);
		IOPort::WriteByte((uint16_t) mmio + 10, 0x4);

		IOPort::WriteByte(PrimaryControl, 0);
		IOPort::WriteByte(SecondaryControl, 0);

		Log("Initialised Busmastering DMA with BaseIO %x", mmio);
	}







	static IOResult PerformDMARead(ATADrive* dev, uint64_t Sector, uint64_t Bytes);

	IOResult ReadBytes(ATADrive* dev, uint64_t Sector, uint64_t Bytes)
	{
		const uint64_t MaxBytesPerOp = 256 * 512;
		if(Bytes > MaxBytesPerOp)
		{
			// allocate a new buffer.
			DMAAddr paddr = Physical::AllocateDMA((Bytes + 0xFFF) / 0x1000);

			uint64_t done = 0;
			while(done < Bytes)
			{
				uint64_t runlength = __min(MaxBytesPerOp, Bytes - done);

				IOResult res = PerformDMARead(dev, Sector + (done / 512), runlength);
				Memory::Copy((void*) (paddr.virt + done), (void*) res.allocatedBuffer.virt, runlength);

				done += runlength;

				// if the bufferSizeInPages is zero, then we don't free anything
				assert(res.bufferSizeInPages > 0);
				Physical::FreeDMA(res.allocatedBuffer, res.bufferSizeInPages);
			}

			return IOResult(Bytes, paddr, (Bytes + 0xFFF) / 0x1000);
		}
		else
		{
			return PerformDMARead(dev, Sector, Bytes);
		}
	}











	static IOResult PerformDMARead(ATADrive* dev, uint64_t Sector, uint64_t Bytes)
	{
		if(Bytes <= 512)
		{
			uint64_t sectors = (Bytes + 511) / 512;
			DMAAddr a = Physical::AllocateDMA((Bytes + 0xFFF) / 0x1000);
			// DMAAddr a;
			// a.phys = 0;
			// a.virt = (uintptr_t) (&dev->Data[0]);

			uint64_t have = 0;

			for(uint64_t i = 0; i < sectors; i++)
			{
				PIO::ReadSector(dev, Sector + i);
				Memory::Copy((void*) (a.virt + have), &dev->Data[0], __min(512, Bytes - have));

				have += __min(512, Bytes - have);
			}

			return IOResult(Bytes, a, (Bytes + 0xFFF) / 0x1000);
		}


		uint32_t mmio = (uint32_t) dev->ParentPCI->GetBAR(4);

		// get a prd
		bool found = false;
		PRDTableCache prdCache;
		for(PRDTableCache& cache : cachedPRDTables)
		{
			if(!cache.used)
			{
				prdCache = cache;
				cache.used = 1;
				found = true;
				break;
			}
		}

		if(!found)
		{
			prdCache.address = Physical::AllocateDMA(1);
			prdCache.length = 0x1000;
			prdCache.used = 1;

			cachedPRDTables.push_back(prdCache);
		}


		COMPILE_TIME_ASSERT(UINT16_MAX == 65535);
		COMPILE_TIME_ASSERT(INT16_MAX == 32767);

		PRDEntry* prd = (PRDEntry*) prdCache.address.virt;
		DMAAddr paddr = Physical::AllocateDMA((Bytes + 0xFFF) / 0x1000);
		// Log("dma read: (v = %x, p = %x) (%d pages, %d bytes)", paddr.virt, paddr.phys, (Bytes + 0xFFF) / 0x1000, Bytes);

		uint64_t numprds = (Bytes + (UINT16_MAX - 1)) / UINT16_MAX;
		if(numprds > (0x1000 / sizeof(PRDEntry)))
			HALT("Too many bytes!");


		for(uint64_t i = 0, done = 0; i < numprds; i++)
		{
			uint64_t toread = 0;
			if(Bytes - done >= UINT16_MAX)
				toread = 0;

			else
				toread = Bytes - done;

			prd[i].bufferPhysAddr = (uint32_t) (paddr.phys + done);
			prd[i].byteCount = (uint16_t) toread;
			prd[i].lastEntry = ((i == numprds - 1) ? 0x8000 : 0);

			done += (toread == 0 ? 65536 : toread);
			// Log("batch %d: %d bytes %s", i, (toread == 0 ? 65536 : toread), (prd[i].lastEntry & 0x8000) ? "(last)" : "");
		}

		// Log("used %d prds.", numprds);

		// write the bytes of address into register
		IOPort::Write32((uint16_t) (mmio + (dev->GetBus() ? 8 : 0) + 4), (uint32_t) prdCache.address.phys);

		// todo
		PreviousDevice = dev;

		if(Bytes / dev->GetSectorSize() > UINT16_MAX + 1)
			HALT("Too many bytes!");

		uint32_t SecCount = (uint32_t) Bytes / dev->GetSectorSize();
		// Log(">> reading %d sectors (%x | %x)", SecCount, (SecCount & 0xFF00) >> 8, SecCount & 0xFF);

		PIO::SendCommandData(dev, Sector, SecCount, true);

		IOPort::WriteByte(dev->GetBaseIO() + 7, Sector > 0x0FFFFFFF ? ATA_ReadSectors48DMA : ATA_ReadSectors28DMA);
		IOPort::WriteByte((uint16_t) (mmio + (dev->GetBus() ? 8 : 0) + 0), DMA::DMACommandRead | DMA::DMACommandStart);

		_WaitingDMA14 = !dev->GetBus();
		_WaitingDMA15 = dev->GetBus();


		uint64_t no = Time::Now();
		while(_WaitingDMA14 || _WaitingDMA15)
		{
			if(no + 5000 < Time::Now())
			{
				Log(1, "DMA Operation timeout (waited 5000 ms)");
				break;
			}
		}

		_WaitingDMA14 = false;
		_WaitingDMA15 = false;

		// stop
		IOPort::WriteByte((uint16_t) (mmio + (dev->GetBus() ? 8 : 0) + 0), DMA::DMACommandRead | DMA::DMACommandStop);

		// Utilities::DumpBytes(paddr.virt, Bytes);

		// release cache
		prdCache.used = 0;

		return IOResult(Bytes, paddr, (Bytes + 0xFFF) / 0x1000);
	}

























	// sector, data, size.
	IOResult WriteBytes(ATADrive*, uint64_t, uint64_t, uint64_t)
	{
		// todo: something
		HALT("DMA WRITE NOT SUPPORTED");
		return IOResult();

		// uint32_t mmio = (uint32_t) dev->ParentPCI->GetBAR(4);

		// // get a prd
		// bool found = false;
		// PRDTableCache prdCache;
		// for(PRDTableCache& cache : cachedPRDTables)
		// {
		// 	if(!cache.used)
		// 	{
		// 		prdCache = cache;
		// 		cache.used = true;
		// 		found = 1;
		// 		break;
		// 	}
		// }

		// if(!found)
		// {
		// 	prdCache.address = Physical::AllocateDMA(1);
		// 	prdCache.length = 0x1000;
		// 	prdCache.used = 1;

		// 	cachedPRDTables.push_back(prdCache);
		// }

		// PRDEntry* prd = (PRDEntry*) prdCache.address.virt;

		// // allocate a buffer that we know is a good deal
		// DMAAddr paddr = Physical::AllocateDMA((Bytes + 0xFFF) / 0x1000);

		// prd->bufferPhysAddr = (uint32_t) paddr.phys;
		// prd->byteCount = (uint16_t) Bytes;
		// prd->lastEntry = 0x8000;

		// // write the bytes of address into register
		// IOPort::Write32((uint16_t)(mmio + (dev->GetBus() ? 8 : 0) + 4), (uint32_t) prdCache.address.phys);

		// PreviousDevice = dev;

		// // Log("sending cmd data: sector %d, %d sectors", Sector, (uint8_t) (Bytes / dev->GetSectorSize()));
		// if(Bytes / dev->GetSectorSize() > UINT16_MAX + 1)
		// 	HALT("Too many bytes!");

		// PIO::SendCommandData(dev, Sector, (uint32_t) (Bytes / dev->GetSectorSize()), true);

		// IOPort::WriteByte(dev->GetBaseIO() + 7, Sector > 0x0FFFFFFF ? ATA_WriteSectors48DMA : ATA_WriteSectors28DMA);
		// IOPort::WriteByte((uint16_t)(mmio + (dev->GetBus() ? 8 : 0) + 0), DMA::DMACommandWrite | DMA::DMACommandStart);

		// _WaitingDMA14 = !dev->GetBus();
		// _WaitingDMA15 = dev->GetBus();

		// uint64_t no = Time::Now();
		// uint64_t t1 = 100000000;
		// while((_WaitingDMA14 || _WaitingDMA15) && Time::Now() < no + 100 && t1 > 0)
		// 	t1--;


		// _WaitingDMA14 = false;
		// _WaitingDMA15 = false;

		// // stop
		// IOPort::WriteByte((uint16_t)(mmio + (dev->GetBus() ? 8 : 0) + 0), DMA::DMACommandWrite | DMA::DMACommandStop);

		// // copy over
		// Memory::Copy((void*) Buffer, (void*) paddr.virt, Bytes);
		// Physical::FreeDMA(paddr, (Bytes + 0xFFF) / 0x1000);
	}



	static void HandleIRQ()
	{
		ATADrive* dev = PreviousDevice;
		if(!dev)
			return;

		if(dev->ParentPCI->IsBARIOPort(4))
		{
			uint32_t mmio = (uint32_t) dev->ParentPCI->GetBAR(4);
			uint8_t status = IOPort::ReadByte((uint16_t) (mmio + (dev->GetBus() ? 8 : 0) + 2));

			if(status & 0x2)
				Log(3, "DMA Transfer failed???");

			if(!(status & 0x4))
			{
				if(dev->GetBus()) _WaitingDMA15 = true;
				else _WaitingDMA14 = true;

				return;
			}

			IOPort::WriteByte((uint16_t) (mmio + (dev->GetBus() ? 8 : 0) + 2), status | 0x4);
		}
		else
		{
			// wtf are we supposed to do?
		}
	}

	void HandleIRQ14()
	{
		if(_WaitingDMA14)
		{
			_WaitingDMA14 = false;
			HandleIRQ();
		}
	}

	void HandleIRQ15()
	{
		if(_WaitingDMA15)
		{
			_WaitingDMA15 = false;
			HandleIRQ();
		}
	}
}
}
}
}
}
}







