// PIO.cpp
// Copyright (c) 2013 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#include <Kernel.hpp>
#include <HardwareAbstraction/Devices/StorageDevice.hpp>
#include <HardwareAbstraction/Interrupts.hpp>
#include <HardwareAbstraction/Devices/IOPort.hpp>
#include <StandardIO.hpp>

using namespace Kernel;
using namespace Kernel::HardwareAbstraction::Devices;
using namespace Library;

namespace Kernel {
namespace HardwareAbstraction {
namespace Devices {
namespace Storage {
namespace ATA {
namespace PIO
{
	const uint8_t ATA_ReadSectors28		= 0x20;
	const uint8_t ATA_ReadSectors48		= 0x24;

	const uint8_t ATA_WriteSectors28	= 0x30;
	const uint8_t ATA_WriteSectors48	= 0x34;


	volatile static bool IsWaiting14 = false;
	volatile static bool IsWaiting15 = false;

	void ReadSector(ATADrive* dev, uint64_t Sector)
	{
		SendCommandData(dev, Sector, 1);

		IsWaiting14 = !dev->GetBus();
		IsWaiting15 = dev->GetBus();

		IOPort::WriteByte(dev->GetBaseIO() + 7, (Sector > 0x0FFFFFFF ? ATA_ReadSectors48 : ATA_ReadSectors28));


		if(IsWaiting14)
		{
			while(IsWaiting14);
		}
		else
		{
			while(IsWaiting15);
		}

		// read data
		for(uint64_t i = 0; i < 256; i++)
		{
			dev->Data[i] = IOPort::Read16(dev->GetBaseIO() + 0);
		}
	}

	void WriteSector(ATADrive* dev, uint64_t Sector)
	{
		SendCommandData(dev, Sector, 1);

		IOPort::WriteByte(dev->GetBaseIO() + 7, (Sector > 0x0FFFFFFF ? ATA_WriteSectors48 : ATA_WriteSectors28));

		// write data
		for(uint64_t i = 0; i < 256; i++)
		{
			IOPort::Write16(dev->GetBaseIO() + 0, dev->Data[i]);
		}

	}

	void SendCommandData(ATADrive* dev, uint64_t Sector, uint32_t SecCount, bool isDMA)
	{
		{
			uint16_t timeout = 0;
			// read the alt. status port to check if the drive is ready
			uint8_t stat = IOPort::ReadByte(dev->GetBaseIO() + 7);

			// check for error
			if((stat & (1 << 0)) || (stat & (1 << 5)))
			{
				Log(1, "Error: ATA Drive ERR or DF bit set. (cmd: sector %d, num = %d) Return addresses:", Sector, SecCount);

				Log(1, "(0): %x", __builtin_return_address(0));
				Log(1, "(1): %x", __builtin_return_address(1));

				// abort.
				Log(1, "ABORT CMD");
				return;
			}

			// wait until we're ready
			while(!(IOPort::ReadByte(dev->GetBaseIO() + 7) & (1 << 6)) && (IOPort::ReadByte(dev->GetBaseIO() + 7) & (1 << 7)) && (IOPort::ReadByte(dev->GetBaseIO() + 7) & (1 << 3)) && (stat & (1 << 0) || stat & (1 << 5)) && timeout < 65535)
			{
				timeout++;
			}

			if(timeout == 65535)
			{
				Log(1, "Disk reading sector [%d] failed, BSY bit stuck or RDY bit not set.\n\n", Sector);
				return;
			}
		}

		if((isDMA && SecCount == 256) || (!isDMA && SecCount == 65536))
			SecCount = 0;

		else if((isDMA && SecCount > 256) || (!isDMA && SecCount > 65536))
			HALT("Too many sectors.");

		// don't use LBA48 unless required
		if(Sector > 0x0FFFFFFF || SecCount > UINT8_MAX)
		{
			// init
			IOPort::WriteByte(dev->GetBaseIO() + 6, dev->IsSlave() ? 0x50 : 0x40);

			// high sector count
			// must write 0 for DMA, because stupid.
			IOPort::WriteByte(dev->GetBaseIO() + 2, isDMA ? 0 : ((SecCount & 0xFF00) >> 8));

			// send high bytes of LBA
			IOPort::WriteByte(dev->GetBaseIO() + 3, (uint8_t) (Sector >> 24));
			IOPort::WriteByte(dev->GetBaseIO() + 4, (uint8_t) (Sector >> 32));
			IOPort::WriteByte(dev->GetBaseIO() + 5, (uint8_t) (Sector >> 40));
		}
		else
		{
			// init
			IOPort::WriteByte(dev->GetBaseIO() + 6, (dev->IsSlave() ? 0xF0 : 0xE0) | ((Sector >> 24) & 0x0F));
		}

		// low sector count
		IOPort::WriteByte(dev->GetBaseIO() + 2, SecCount & 0xFF);

		// send low bytes of LBA
		IOPort::WriteByte(dev->GetBaseIO() + 3, (uint8_t) Sector);
		IOPort::WriteByte(dev->GetBaseIO() + 4, (uint8_t) (Sector >> 8));
		IOPort::WriteByte(dev->GetBaseIO() + 5, (uint8_t) (Sector >> 16));
	}


	extern "C" void IRQHandler14()
	{
		DMA::HandleIRQ14();
		IsWaiting14 = false;
	}

	extern "C" void IRQHandler15()
	{
		DMA::HandleIRQ15();
		IsWaiting15 = false;
	}
}
}
}
}
}
}



















