// DMA.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <HardwareAbstraction/Devices/StorageDevice.hpp>
#include <HardwareAbstraction/Interrupts.hpp>
#include <HardwareAbstraction/Devices/IOPort.hpp>
#include <Memory.hpp>
#include <StandardIO.hpp>

using namespace Library::StandardIO;
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


	static volatile bool _WaitingDMA14 = false;
	static volatile bool _WaitingDMA15 = false;

	static ATADrive* PreviousDevice = 0;

	static __attribute__((aligned(8))) uint8_t PRDT[8][256] = { { 0 } };

	void Initialise()
	{
		using namespace Kernel::HardwareAbstraction::Devices::PCI;
		PCIDevice* ide = PCI::GetDeviceByClassSubclass(01, 01);

		assert(ide);

		// enable bus mastering
		uint32_t f = ide->GetRegisterData(0x4, 0, 2);
		ide->WriteRegisterData(0x4, 0, 2, f | 0x4);

		uint32_t mmio = (uint32_t) ide->GetBAR(4);
		assert(ide->IsBARIOPort(4));
		Log("Initialised Busmastering DMA with BaseIO %x", mmio);

		uint64_t i = 0;

		for(auto d : *ATADrive::ATADrives)
			d->PRDTable = (uint64_t) (PRDT[i]), i++;


		// for(uint64_t i = 0; i < ATADrive::ATADrives->size(); i++)
		// 	ATADrive::ATADrives->get(i)->PRDTable = (uint64_t)(PRDT[i]);

		IOPort::WriteByte((uint16_t) mmio + 2, 0x4);
		IOPort::WriteByte((uint16_t) mmio + 10, 0x4);

		IOPort::WriteByte(0x3F6, 0);
		IOPort::WriteByte(0x376, 0);
	}

	void ReadBytes(ATADrive* dev, uint64_t Buffer, uint64_t Sector, uint64_t Bytes)
	{
		uint32_t mmio = (uint32_t) dev->ParentPCI->GetBAR(4);

		// create a prd
		uintptr_t* prd = (uintptr_t*) dev->PRDTable;
		uint32_t b = (uint32_t) Buffer;


		*((uint32_t*) prd) = b;
		*((uint16_t*)((uintptr_t) prd + 4)) = (uint16_t) Bytes;
		*((uint16_t*)((uintptr_t) prd + 6)) = 0x8000;

		// write the bytes of address into register
		IOPort::Write32((uint16_t)(mmio + (dev->GetBus() ? 8 : 0) + 4), (uint32_t)((uint64_t) prd));

		PreviousDevice = dev;
		PIO::SendCommandData(dev, Sector, (uint8_t)(Bytes / dev->GetSectorSize()));

		IOPort::WriteByte(dev->GetBaseIO() + 7, Sector > 0x0FFFFFFF ? ATA_ReadSectors48DMA : ATA_ReadSectors28DMA);
		IOPort::WriteByte((uint16_t)(mmio + (dev->GetBus() ? 8 : 0) + 0), DMA::DMACommandRead | DMA::DMACommandStart);

		_WaitingDMA14 = !dev->GetBus();
		_WaitingDMA15 = dev->GetBus();

		uint64_t no = Time::Now();
		uint64_t t1 = 100000;
		while((_WaitingDMA14 || _WaitingDMA15) && Time::Now() < no + 100 && t1 > 0)
			t1--;

		_WaitingDMA14 = false;
		_WaitingDMA15 = false;

		// stop
		IOPort::WriteByte((uint16_t)(mmio + (dev->GetBus() ? 8 : 0) + 0), DMA::DMACommandRead | DMA::DMACommandStop);
	}






	void WriteBytes(ATADrive* dev, uint64_t Buffer, uint64_t Sector, uint64_t Bytes)
	{
		uint32_t mmio = (uint32_t) dev->ParentPCI->GetBAR(4);

		// create a prd
		uintptr_t* prd = (uintptr_t*) dev->PRDTable;
		uint32_t b = (uint32_t) Buffer;

		*((uint32_t*) prd) = b;
		*((uint16_t*)((uintptr_t) prd + 4)) = (uint16_t) Bytes;
		*((uint16_t*)((uintptr_t) prd + 6)) = 0x8000;

		// write the bytes of address into register
		IOPort::Write32((uint16_t)(mmio + (dev->GetBus() ? 8 : 0) + 4), (uint32_t)((uint64_t) prd));

		PreviousDevice = dev;
		PIO::SendCommandData(dev, Sector, (uint8_t)(Bytes / dev->GetSectorSize()));

		IOPort::WriteByte(dev->GetBaseIO() + 7, Sector > 0x0FFFFFFF ? ATA_WriteSectors48DMA : ATA_WriteSectors28DMA);
		IOPort::WriteByte((uint16_t)(mmio + (dev->GetBus() ? 8 : 0) + 0), DMA::DMACommandWrite | DMA::DMACommandStart);

		_WaitingDMA14 = !dev->GetBus();
		_WaitingDMA15 = dev->GetBus();

		uint64_t no = Time::Now();
		uint64_t t1 = 100000;
		while((_WaitingDMA14 || _WaitingDMA15) && Time::Now() < no + 100 && t1 > 0)
			t1--;


		_WaitingDMA14 = false;
		_WaitingDMA15 = false;

		// stop
		IOPort::WriteByte((uint16_t)(mmio + (dev->GetBus() ? 8 : 0) + 0), DMA::DMACommandWrite | DMA::DMACommandStop);
	}



	static void HandleIRQ()
	{
		if(!PreviousDevice)
			return;

		if(PreviousDevice->ParentPCI->IsBARIOPort(4))
		{
			IOPort::ReadByte((uint16_t)(PreviousDevice->ParentPCI->GetBAR(4) + (PreviousDevice->GetBus() ? 8 : 0) + 2));
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







