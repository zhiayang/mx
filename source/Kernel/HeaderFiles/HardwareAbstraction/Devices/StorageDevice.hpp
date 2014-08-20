// StorageDevice.hpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under Creative Commons Attribution ShareAlike 3.0 Unported.


#include <stdint.h>
#include <List.hpp>
#include "PCI.hpp"

#pragma once

// class Kernel::HardwareAbstraction::Filesystems::Partition;

namespace Kernel {
namespace HardwareAbstraction
{
	namespace Filesystems
	{
		class FSDriver;

		enum class FSTypes
		{
			// FAT family
			exFAT		= 0xF1EF,
			fat32		= 0xF132,
			fat16		= 0xF116,
			fat12		= 0xF112,

			// Apple
			hfsplus		= 0x11F5,
			hfs		= 0x11F0,

			// Linux
			ext2		= 0xEB12,
			ext3		= 0xEB13,
			ext4		= 0xEB14
		};
	}

	namespace Devices {
	namespace Storage
	{
		enum class StorageDeviceType
		{
			ATAHardDisk,
		};

		enum class PartitionTableType
		{
			MasterBootRecord,
			GuidPartitionTable,
		};

		class Partition;
		class StorageDevice
		{
			public:
				StorageDevice(StorageDeviceType tp) : Type(tp) { }
				virtual ~StorageDevice();
				virtual void Read(uint64_t position, uint64_t outbuf, uint64_t bytes);
				virtual void Write(uint64_t position, uint64_t outbuf, uint64_t bytes);

				StorageDeviceType Type;
				PartitionTableType PartitionTable;
				Library::LinkedList<Partition>* Partitions;
		};

		class Partition
		{
			public:
				Partition(StorageDevice* dev, uint8_t num, uint64_t StartLBA, uint64_t LBALength, Filesystems::FSTypes type, uint64_t PartitionGUID_high, uint64_t PartitionGUID_low, uint64_t TypeGUID_high, uint64_t TypeGUID_low, char* Name, bool Bootable);
				void		PrintInfo();

				uint64_t		GetStartLBA();
				uint64_t		GetLBALength();
				Filesystems::FSTypes	GetType();

				uint64_t	GetGUID_S1();
				uint64_t	GetGUID_S2();
				uint64_t	GetGUID_S3();
				uint64_t	GetGUID_S4();
				uint64_t	GetGUID_S5();


				uint64_t	GetGUID_High();
				uint64_t	GetGUID_Low();


				uint64_t	GetTypeGUID_S1();
				uint64_t	GetTypeGUID_S2();
				uint64_t	GetTypeGUID_S3();
				uint64_t	GetTypeGUID_S4();
				uint64_t	GetTypeGUID_S5();


				uint64_t	GetTypeGUID_High();
				uint64_t	GetTypeGUID_Low();
				char*		GetName();
				bool		IsBootable();
				StorageDevice*	GetStorageDevice();
				uint8_t		GetPartitionNumber();
				Filesystems::FSDriver*	GetFilesystem();

				void		ReadSector(uint64_t LBA);
				void		WriteSector(uint64_t LBA);

			private:
				uint64_t	StartLBA;
				uint64_t	LBALength;
				Filesystems::FSTypes	PartitionType;		// Our internal ones are of 0xZZZZ
				uint64_t	PartitionGUID_high;
				uint64_t	PartitionGUID_low;

				uint64_t	PartitionTypeGUID_high;
				uint64_t	PartitionTypeGUID_low;


				char		Name[37];
				bool		Bootable;
				StorageDevice* Drive;
				uint8_t		PartitionNumber;

				Filesystems::FSDriver*	Filesystem;
		};




		class ATADrive : public StorageDevice
		{
			public:
				ATADrive(uint8_t Bus, uint8_t Drive);
				virtual ~ATADrive() { }

				uint8_t	GetBus();
				uint8_t GetDrive();
				bool IsSlave();

				void SetSectors(uint64_t s);
				uint64_t GetSectors();

				void SetSectorSize(uint16_t SectorSize);
				uint32_t GetSectorSize();

				uint16_t GetBaseIO();
				uint8_t GetDriveNumber();

				bool GetIsGPT();
				void SetIsGPT(bool IsGPT);

				void ReadSector(uint64_t LBA);
				void WriteSector(uint64_t LBA);

				virtual void Read(uint64_t LBA, uint64_t Buffer, uint64_t Bytes) override;
				virtual void Write(uint64_t LBA, uint64_t Buffer, uint64_t Bytes) override;

				uint16_t Data[256];
				uint64_t PRDTable;
				PCI::PCIDevice* ParentPCI;

				static Library::LinkedList<ATADrive>* ATADrives;

			private:
				uint8_t Bus;
				uint8_t Drive;
				uint64_t MaxSectors;
				uint16_t SectorSize;
				uint16_t BaseIO;
				uint8_t DriveNumber;
				bool IsGPT;
		};


























		namespace ATA
		{
			namespace PIO
			{
				// go modify dev->Data before calling this
				// in other words, don't call this directly.
				void ReadSector(ATADrive* dev, uint64_t Sector);
				void WriteSector(ATADrive* dev, uint64_t Sector);

				void SendCommandData(ATADrive* dev, uint64_t Sector, uint8_t SecCount, bool IsDMA = false);

				extern "C" void IRQHandler14();
				extern "C" void IRQHandler15();

				extern "C" void ATA_HandleIRQ14();
				extern "C" void ATA_HandleIRQ15();

				bool GetIsWaiting14();
				bool GetIsWaiting15();
			}

			namespace DMA
			{
				extern const uint8_t DMACommandRead;
				extern const uint8_t DMACommandStart;
				extern const uint8_t DMACommandStop;
				extern const uint8_t DMACommandWrite;

				extern uint64_t DefaultDMATimeout;

				void HandleIRQ14();
				void HandleIRQ15();

				void Initialise();

				void ReadBytes(ATADrive* dev, uint64_t Buffer, uint64_t Sector, uint64_t Bytes);
				void WriteBytes(ATADrive* dev, uint64_t Buffer, uint64_t Sector, uint64_t Bytes);
			}

			void PrintATAInfo(ATADrive* ata);
			void Initialise();
			void IdentifyAll(PCI::PCIDevice* controller);
			ATADrive* IdentifyDevice(uint16_t BaseIO, bool IsMaster);


			extern const uint8_t ATA_Identify;
			extern const uint8_t ATA_ReadSectors28;
			extern const uint8_t ATA_ReadSectors48;
			extern const uint8_t ATA_ReadSectors28DMA;
			extern const uint8_t ATA_ReadSectors48DMA;
			extern const uint8_t ATA_WriteSectors28;
			extern const uint8_t ATA_WriteSectors48;
			extern const uint8_t ATA_WriteSectors28DMA;
			extern const uint8_t ATA_WriteSectors48DMA;
		}
	}
}
}
}