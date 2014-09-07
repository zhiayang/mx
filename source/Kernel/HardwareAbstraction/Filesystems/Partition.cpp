// MBR.hpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <Utility.hpp>
#include <HardwareAbstraction/Devices/StorageDevice.hpp>
#include <string.h>

using namespace Kernel::HardwareAbstraction::Devices::Storage;

namespace Kernel {
namespace HardwareAbstraction {
namespace Devices {
namespace Storage
{
	Partition::Partition(StorageDevice* d, uint8_t num, uint64_t slba, uint64_t lbal, Filesystems::FSTypes Type, uint64_t pgh, uint64_t pgl, uint64_t th, uint64_t tl, char* n, bool b)
	{
		this->Drive = d;
		this->StartLBA = slba;
		this->LBALength = lbal;
		this->PartitionType = Type;

		this->PartitionGUID_high = pgh;
		this->PartitionGUID_low = pgl;

		this->PartitionTypeGUID_high = th;
		this->PartitionTypeGUID_low = tl;

		// Library::String::Copy(this->Name, n);
		String::Copy(this->Name, n);

		this->Bootable = b;

		// if we find a partition of type 0xEE, the drive is assumed to be a GPT drive.
		if(Type == (Filesystems::FSTypes) 0xEE)
			this->Drive->PartitionTable = PartitionTableType::GuidPartitionTable;


		this->PartitionNumber = num;

		// 48465300-0000-11AA-AA11-00306543ECAC

		if(PartitionType == Filesystems::FSTypes::fat32 ||
			(this->Drive->PartitionTable == PartitionTableType::GuidPartitionTable &&
			this->GetTypeGUID_S1() == 0xEBD0A0A2 &&
			this->GetTypeGUID_S2() == 0xB9E5 &&
			this->GetTypeGUID_S3() == 0x4433 &&
			this->GetTypeGUID_S4() == 0x87C0 &&
			this->GetTypeGUID_S5() == 0x68B6B72699C7))
		{
			// this->Filesystem = new Filesystems::FAT32(this);
		}

		if(PartitionType == Filesystems::FSTypes::hfsplus ||
			(this->Drive->PartitionTable == PartitionTableType::GuidPartitionTable &&
			this->GetTypeGUID_S1() == 0x48465300 &&
			this->GetTypeGUID_S2() == 0x0000 &&
			this->GetTypeGUID_S3() == 0x11AA &&
			this->GetTypeGUID_S4() == 0xAA11 &&
			this->GetTypeGUID_S5() == 0x00306543ECAC))
		{
			// this->Filesystem = new Filesystems::HFSPlus(this);
		}
	}

	void Partition::PrintInfo()
	{
		// uint64_t index = Library::Utility::ReduceBinaryUnits(this->LBALength * this->Drive->GetSectorSize());
		// uint64_t mem = Library::Utility::GetReducedMemory(this->LBALength * this->Drive->GetSectorSize());

		// Library::StandardIO::PrintFormatted("\t-> %wPartition%r%k[%d%k] %won%r %wHDD%r%k[%d%k]: %k{Type: %x, Start: LBA %d, Length: %d Blocks, Size: %w%d %s%k}", Library::Colours::DarkCyan, Library::Colours::Silver, this->PartitionNumber, Library::Colours::Silver, Library::Colours::Silver, Library::Colours::Violet, Library::Colours::Silver, this->Drive->GetDriveNumber(), Library::Colours::Silver, Library::Colours::Silver, (uint64_t) this->PartitionType, this->StartLBA, this->LBALength, Library::Colours::DarkRed, mem, Kernel::K_BinaryUnits[index], Library::Colours::Silver);
	}

	uint64_t				Partition::GetStartLBA(){ return this->StartLBA; }
	uint64_t				Partition::GetLBALength(){ return this->LBALength; }
	Filesystems::FSTypes			Partition::GetType(){ return this->PartitionType; }
	uint64_t				Partition::GetGUID_High(){ return this->PartitionGUID_high; }
	uint64_t				Partition::GetGUID_Low(){ return this->PartitionGUID_low; }
	uint64_t				Partition::GetTypeGUID_High(){ return this->PartitionTypeGUID_high; }
	uint64_t				Partition::GetTypeGUID_Low(){ return this->PartitionTypeGUID_low; }
	char*					Partition::GetName(){ return this->Name; }
	bool					Partition::IsBootable(){ return this->Bootable; }
	StorageDevice*				Partition::GetStorageDevice(){ return this->Drive; }
	uint8_t					Partition::GetPartitionNumber(){ return this->PartitionNumber; }
	Filesystems::FSDriver*			Partition::GetFilesystem(){ return this->Filesystem; }


	uint64_t				Partition::GetGUID_S1(){ return this->PartitionGUID_high & 0x00000000FFFFFFFF; }
	uint64_t				Partition::GetGUID_S2(){ return this->PartitionGUID_high & 0x0000FFFF00000000; }
	uint64_t				Partition::GetGUID_S3(){ return ((this->PartitionGUID_high & 0xFFFF000000000000) >> 48); }
	uint64_t				Partition::GetGUID_S4()
	{
		return (((this->PartitionTypeGUID_low & 0xFFFF) & 0xFF00) >> 8) | (((this->PartitionGUID_low & 0xFFFF) & 0xFF) << 8);
	}
	uint64_t Partition::GetGUID_S5()
	{
		uint64_t s1 = (this->PartitionGUID_low & 0xFF00000000000000) >> 56;
		uint64_t s2 = (this->PartitionGUID_low & 0x00FF000000000000) >> 40;
		uint64_t s3 = (this->PartitionGUID_low & 0x0000FF0000000000) >> 24;
		uint64_t s4 = (this->PartitionGUID_low & 0x000000FF00000000) >> 8;
		uint64_t s5 = (this->PartitionGUID_low & 0x00000000FF000000) << 8;
		uint64_t s6 = (this->PartitionGUID_low & 0x0000000000FF0000) << 24;
		return s1 | s2 | s3 | s4 | s5 | s6;
	}


	uint64_t				Partition::GetTypeGUID_S1(){ return this->PartitionTypeGUID_high & 0x00000000FFFFFFFF; }
	uint64_t				Partition::GetTypeGUID_S2(){ return this->PartitionTypeGUID_high & 0x0000FFFF00000000; }
	uint64_t				Partition::GetTypeGUID_S3(){ return ((this->PartitionTypeGUID_high & 0xFFFF000000000000) >> 48); }
	uint64_t				Partition::GetTypeGUID_S4()
	{
		return (((this->PartitionTypeGUID_low & 0xFFFF) & 0xFF00) >> 8) | (((this->PartitionTypeGUID_low & 0xFFFF) & 0xFF) << 8);
	}


	uint64_t Partition::GetTypeGUID_S5()
	{
		uint64_t s1 = (this->PartitionTypeGUID_low & 0xFF00000000000000) >> 56;
		uint64_t s2 = (this->PartitionTypeGUID_low & 0x00FF000000000000) >> 40;
		uint64_t s3 = (this->PartitionTypeGUID_low & 0x0000FF0000000000) >> 24;
		uint64_t s4 = (this->PartitionTypeGUID_low & 0x000000FF00000000) >> 8;
		uint64_t s5 = (this->PartitionTypeGUID_low & 0x00000000FF000000) << 8;
		uint64_t s6 = (this->PartitionTypeGUID_low & 0x0000000000FF0000) << 24;
		return s1 | s2 | s3 | s4 | s5 | s6;
	}
}
}
}
}
