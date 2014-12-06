// ATA.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <HardwareAbstraction/Devices/StorageDevice.hpp>
#include <HardwareAbstraction/Interrupts.hpp>
#include <HardwareAbstraction/Devices/IOPort.hpp>
#include <StandardIO.hpp>
#include <Utility.hpp>
#include <List.hpp>
#include <Memory.hpp>

using namespace Kernel;
using namespace Kernel::HardwareAbstraction;
using namespace Kernel::HardwareAbstraction::Devices;

namespace Kernel {
namespace HardwareAbstraction {
namespace Devices {
namespace Storage
{
	static uint16_t PrimaryBaseIO			= 0x1F0;
	static uint16_t SecondaryBaseIO		= 0x170;

	// static uint16_t PrimaryControl			= 0x3F6;
	// static uint16_t SecondaryControl		= 0x376;

	static uint16_t PrimaryCommand		= 0;
	static uint16_t SecondaryCommand		= 0;


	ATADrive::ATADrive(uint8_t b, uint8_t d) : StorageDevice(StorageDeviceType::ATAHardDisk)
	{
		this->Bus			= b;
		this->Drive			= d;
		this->SectorSize	= 512;				// TODO: detect sector size
		this->BaseIO		= (Bus == 0 ? PrimaryBaseIO : SecondaryBaseIO);
		this->DriveNumber	= (Bus ? 2 : 0) + (Drive ? 1 : 0);

		PrimaryCommand		= PrimaryBaseIO + 7;
		SecondaryCommand	= SecondaryBaseIO + 7;
	}


	rde::list<ATADrive*>* ATADrive::ATADrives;


	bool ATADrive::GetIsGPT()			{ return this->IsGPT; }
	void ATADrive::SetIsGPT(bool g)		{ this->IsGPT = g; }
	uint8_t ATADrive::GetDriveNumber()		{ return this->DriveNumber; }
	uint8_t ATADrive::GetBus()			{ return this->Bus; }
	uint8_t ATADrive::GetDrive()			{ return this->Drive; }
	bool ATADrive::IsSlave()			{ return this->Drive; }
	void ATADrive::SetSectors(uint64_t s)		{ this->MaxSectors = s; }
	uint64_t ATADrive::GetSectors()		{ return this->MaxSectors; }
	void ATADrive::SetSectorSize(uint16_t s)	{ this->SectorSize = s; }
	uint32_t ATADrive::GetSectorSize()		{ return this->SectorSize; }
	uint16_t ATADrive::GetBaseIO()		{ return this->BaseIO; }

	void ATADrive::Read(uint64_t LBA, uint64_t Buffer, uint64_t Bytes)
	{
		ATA::DMA::ReadBytes(this, Buffer, LBA, Bytes);
	}

	void ATADrive::Write(uint64_t LBA, uint64_t Buffer, uint64_t Bytes)
	{
		ATA::DMA::WriteBytes(this, Buffer, LBA, Bytes);
	}

	namespace ATA
	{
		const uint8_t ATA_Identify			= 0xEC;

		void PrintATAInfo(ATADrive* ata)
		{
			uint64_t index = Library::Utility::ReduceBinaryUnits(ata->GetSectors() * ata->GetSectorSize());
			uint64_t mem = Library::Utility::GetReducedMemory(ata->GetSectors() * ata->GetSectorSize());

			Library::StandardIO::PrintFormatted("\t-> %s Bus, %s Drive: [%d sectors, size: %d %s, %s]\n", !ata->GetBus() ? "Primary" : "Secondary", !ata->GetDrive() ? "Master" : "Slave", ata->GetSectors(), mem, Kernel::K_BinaryUnits[index], ata->GetIsGPT() ? "GPT Drive" : "MBR Drive");
		}

		void Initialise()
		{
			// At this point, PCI devices should be initialised.
			// So let's do some proper probing.
			using Kernel::HardwareAbstraction::Devices::PCI::PCIDevice;

			rde::list<PCIDevice*>* devlist = PCI::SearchByClassSubclass(0x1, 0x1);

			if(devlist->size() == 0)
			{
				Library::StandardIO::PrintFormatted("ERROR: No IDE Controller found on the PCI bus, which is impossible.");
				UHALT();
			}

			ATADrive::ATADrives = new rde::list<ATADrive*>();
			IdentifyAll(devlist->front());

			DMA::Initialise();

			for(auto p : *ATADrive::ATADrives)
				HardwareAbstraction::Filesystems::MBR::ReadPartitions(p);

			// for(uint64_t i = 0; i < ATADrive::ATADrives->size(); i++)
			// 	HardwareAbstraction::Filesystems::MBR::ReadPartitions(ATADrive::ATADrives->get(i));
		}


		void IdentifyAll(PCI::PCIDevice* controller)
		{
			Interrupts::SetGate(32 + 14, (uint64_t) PIO::ATA_HandleIRQ14, 0x08, 0xEE);
			Interrupts::SetGate(32 + 15, (uint64_t) PIO::ATA_HandleIRQ15, 0x08, 0xEE);


			if(ATA::IdentifyDevice(PrimaryBaseIO, true))
			{
				ATADrive* d = ATA::IdentifyDevice(PrimaryBaseIO, true);
				if(d)
				{
					ATADrive::ATADrives->push_back(d);
					ATADrive::ATADrives->back()->ParentPCI = controller;
				}
			}

			if(ATA::IdentifyDevice(PrimaryBaseIO, false))
			{
				ATADrive* d = ATA::IdentifyDevice(PrimaryBaseIO, false);
				if(d)
				{
					ATADrive::ATADrives->push_back(d);
					ATADrive::ATADrives->back()->ParentPCI = controller;
				}
			}

			if(ATA::IdentifyDevice(SecondaryBaseIO, true))
			{
				ATADrive* d = ATA::IdentifyDevice(SecondaryBaseIO, true);
				if(d)
				{
					ATADrive::ATADrives->push_back(d);
					ATADrive::ATADrives->back()->ParentPCI = controller;
				}
			}

			if(ATA::IdentifyDevice(SecondaryBaseIO, false))
			{
				ATADrive* d = ATA::IdentifyDevice(SecondaryBaseIO, false);
				if(d)
				{
					ATADrive::ATADrives->push_back(d);
					ATADrive::ATADrives->back()->ParentPCI = controller;
				}
			}
		}

		ATADrive* IdentifyDevice(uint16_t BaseIO, bool IsMaster)
		{
			bool fl = false;
			if(BaseIO == PrimaryBaseIO && IsMaster == false)
				fl = true;

			// Identify the drives on the primary bus

			// Do master drive on primary
			IOPort::WriteByte(BaseIO + 6, (IsMaster ? 0xA0 : 0xB0));	// drive select
			IOPort::WriteByte(BaseIO + 2, 0);				// sector count
			IOPort::WriteByte(BaseIO + 3, 0);				// lba low
			IOPort::WriteByte(BaseIO + 4, 0);				// lba mid
			IOPort::WriteByte(BaseIO + 5, 0);				// lba high

			IOPort::WriteByte(BaseIO + 7, ATA_Identify);	// send the identify command

			// read status port
			uint16_t exist = IOPort::ReadByte(BaseIO + 7);
			uint8_t pollcount = 0;
			if(exist)
			{
				while(pollcount < (uint8_t)(-1))
				{
					if(IOPort::ReadByte(BaseIO + 7) & (1 << 7))
						pollcount++;

					else
						break;
				}
				if(IOPort::ReadByte(BaseIO + 4) != 0 || IOPort::ReadByte(BaseIO + 5) != 0)
				{
					// non-compliant ATA device, most likely ATAPI
					return 0;
				}

				pollcount = 0;
				while(pollcount < (uint8_t)(-1))
				{
					if(IOPort::ReadByte(BaseIO + 7) & (1 << 3))
						break;

					else if(IOPort::ReadByte(BaseIO + 7) & (1 << 0))
						return 0;
				}

				// init the ATA object
				ATADrive* ata = new ATADrive((BaseIO == PrimaryBaseIO ? 0 : 1), (IsMaster ? 0 : 1));
				for(uint64_t i = 0; i < 256; i++)
				{
					ata->Data[i] = IOPort::Read16(BaseIO + 0);
				}

				// read the identify data -- disk size in sectors.
				if(ata->Data[83] & (1 << 10))
				{
					ata->SetSectors(*((uint64_t*)(ata->Data + 100)));
				}
				else
				{
					ata->SetSectors(*(uint32_t*)(ata->Data + 60));
				}

				return ata;
			}
			return 0;
		}
	}
}
}
}
}

















