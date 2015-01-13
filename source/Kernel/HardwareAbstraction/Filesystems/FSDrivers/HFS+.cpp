// HFS+.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <StandardIO.hpp>
using namespace Kernel::HardwareAbstraction::MemoryManager;
using namespace Library::StandardIO;

namespace Kernel {
namespace HardwareAbstraction {
namespace Filesystems
{
	FSDriverHFSPlus::FSDriverHFSPlus(Devices::Storage::Partition* _part) : FSDriver(_part, FSDriverType::Physical)
	{
		// read the fields from LBA 0
		auto atadev = this->partition->GetStorageDevice();

		uint64_t buf = MemoryManager::Physical::AllocateDMA(1);
		IO::Read(atadev, this->partition->GetStartLBA() + 2, buf, 512);		// volume header is 512 bytes, starts 1024 bytes in

		// todo: handle sector sizes not 512
		assert(((Devices::Storage::ATADrive*) atadev)->GetSectorSize() == 512);

		HFSPlusVolumeHeader* volheader = (HFSPlusVolumeHeader*) buf;

		if(volheader->signature[0] != 'H')
			Log(1, "Invalid signature on HFS+ filesystem (disk%ds%d), expected 'H', got '%c'", atadev->diskid, this->partition->GetPartitionNumber(), volheader->signature[0]);

		if(volheader->signature[1] != '+' && volheader->signature[1] != 'X')
			Log(1, "Invalid signature on HFS+ filesystem (disk%ds%d), expected '+' (for HFS+) or 'X' (for case-sensitive HFS+), got '%c'", atadev->diskid, this->partition->GetPartitionNumber(), volheader->signature[1]);




		Log("HFS+ Driver on disk%ds%d has been initialised, FS appears to conform to specifications.", atadev->diskid, this->partition->GetPartitionNumber());
	}

	FSDriverHFSPlus::~FSDriverHFSPlus()
	{

	}

	bool FSDriverHFSPlus::Create(VFS::vnode* node, const char* path, uint64_t flags, uint64_t perms)
	{
		return 0;
	}

	bool FSDriverHFSPlus::Delete(VFS::vnode* node, const char* path)
	{
		return 0;
	}

	bool FSDriverHFSPlus::Traverse(VFS::vnode* node, const char* path, char** symlink)
	{
		return 0;
	}

	size_t FSDriverHFSPlus::Read(VFS::vnode* node, void* buf, off_t offset, size_t length)
	{
		return 0;
	}

	size_t FSDriverHFSPlus::Write(VFS::vnode* node, const void* buf, off_t offset, size_t length)
	{
		return 0;
	}

	void FSDriverHFSPlus::Flush(VFS::vnode* node)
	{

	}

	void FSDriverHFSPlus::Stat(VFS::vnode* node, struct stat* stat, bool statlink)
	{

	}


	// returns a list of items inside the directory, as vnodes.
	rde::vector<VFS::vnode*>* FSDriverHFSPlus::ReadDir(VFS::vnode* node)
	{
		return 0;
	}

}
}
}
