// HFS+.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <StandardIO.hpp>
using namespace Kernel::HardwareAbstraction::MemoryManager;

#define bswap16(x)						__builtin_bswap16(x)
#define bswap32(x)						__builtin_bswap32(x)
#define bswap64(x)						__builtin_bswap64(x)

#define VHAttr_VolumeUnmounted			(1 << 8)
#define VHAttr_SparedBlocks				(1 << 9)
#define VHAttr_NoCacheRequired			(1 << 10)
#define VHAttr_BootVolInconsistent		(1 << 11)
#define VHAttr_CNIDsReused				(1 << 12)
#define VHAttr_VolumeJournaled			(1 << 13)
#define VHAttr_SoftwareLock				(1 << 15)


namespace Kernel {
namespace HardwareAbstraction {
namespace Filesystems
{
	FSDriverHFSPlus::FSDriverHFSPlus(Devices::Storage::Partition* _part) : FSDriver(_part, FSDriverType::Physical)
	{
		// // read the fields from LBA 0
		// auto atadev = this->partition->GetStorageDevice();

		// uint64_t buf = MemoryManager::Physical::AllocateDMA(1);
		// IO::Read(atadev, this->partition->GetStartLBA() + 2, buf, 512);		// volume header is 512 bytes, starts 1024 bytes in

		// // todo: handle sector sizes not 512
		// assert(((Devices::Storage::ATADrive*) atadev)->GetSectorSize() == 512);

		// HFSPlusVolumeHeader* volheader = (HFSPlusVolumeHeader*) buf;

		// if(volheader->signature[0] != 'H')
		// 	Log(1, "Invalid signature on HFS+ filesystem (disk%ds%d), expected 'H', got '%c'", atadev->diskid, this->partition->GetPartitionNumber(), volheader->signature[0]);

		// if(volheader->signature[1] != '+' && volheader->signature[1] != 'X')
		// 	Log(1, "Invalid signature on HFS+ filesystem (disk%ds%d), expected '+' (for HFS+) or 'X' (for case-sensitive HFS+), got '%c'", atadev->diskid, this->partition->GetPartitionNumber(), volheader->signature[1]);


		// if(volheader->signature[1] == 'X')
		// {
		// 	assert(bswap16(volheader->version) == 5);
		// 	this->caseSensitive = true;
		// }
		// else
		// {
		// 	assert(bswap16(volheader->version) == 4);
		// 	this->caseSensitive = false;
		// }



		// // fucking warnings
		// (void) this->volumeHeader;









		// Log("HFS+ Driver on disk%ds%d has been initialised, FS appears to conform to specifications.", atadev->diskid, this->partition->GetPartitionNumber());
	}

	FSDriverHFSPlus::~FSDriverHFSPlus()
	{

	}

	bool FSDriverHFSPlus::Create(VFS::vnode* node, const char* path, uint64_t flags, uint64_t perms)
	{
		(void) node;
		(void) path;
		(void) flags;
		(void) perms;
		return 0;
	}

	bool FSDriverHFSPlus::Delete(VFS::vnode* node, const char* path)
	{
		(void) node;
		(void) path;
		return 0;
	}

	bool FSDriverHFSPlus::Traverse(VFS::vnode* node, const char* path, char** symlink)
	{
		(void) node;
		(void) path;
		(void) symlink;
		return 0;
	}

	size_t FSDriverHFSPlus::Read(VFS::vnode* node, void* buf, off_t offset, size_t length)
	{
		(void) node;
		(void) buf;
		(void) offset;
		(void) length;
		return 0;
	}

	size_t FSDriverHFSPlus::Write(VFS::vnode* node, const void* buf, off_t offset, size_t length)
	{
		(void) node;
		(void) buf;
		(void) offset;
		(void) length;
		return 0;
	}

	void FSDriverHFSPlus::Flush(VFS::vnode* node)
	{
		(void) node;
	}

	void FSDriverHFSPlus::Stat(VFS::vnode* node, struct stat* stat, bool statlink)
	{
		(void) node;
		(void) stat;
		(void) statlink;
	}

	void FSDriverHFSPlus::Close(VFS::vnode *node)
	{
	}

	// returns a list of items inside the directory, as vnodes.
	iris::vector<VFS::vnode*> FSDriverHFSPlus::ReadDir(VFS::vnode* node)
	{
		(void) node;
		return iris::vector<VFS::vnode*>();
	}

}
}
}
