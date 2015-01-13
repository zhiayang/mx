// HFSPlusVFS.hpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#pragma once
#include "FSUtil.hpp"
#include <endian.h>

namespace Kernel {
namespace HardwareAbstraction {
namespace Filesystems
{
	struct HFSPlusExtentDescriptor
	{
	    uint32_t				startBlock;
	    uint32_t				blockCount;
	} __attribute__((packed));

	typedef HFSPlusExtentDescriptor HFSPlusExtentRecord[8];
	typedef uint32_t HFSCatalogNodeID;

	struct HFSPlusForkData
	{
	    uint64_t				logicalSize;
	    uint32_t				clumpSize;
	    uint32_t				totalBlocks;
	    HFSPlusExtentRecord		extents;
	} __attribute__((packed));


	struct HFSPlusVolumeHeader
	{
		uint8_t				signature[2];
		uint16_t			version;
		uint32_t			attributes;
		uint32_t			lastMountedVersion;
		uint32_t			journalInfoBlock;

		uint32_t			createDate;
		uint32_t			modifyDate;
		uint32_t			backupDate;
		uint32_t			checkedDate;

		uint32_t			fileCount;
		uint32_t			folderCount;

		uint32_t			blockSize;
		uint32_t			totalBlocks;
		uint32_t			freeBlocks;

		uint32_t			nextAllocation;
		uint32_t			rsrcClumpSize;
		uint32_t			dataClumpSize;
		HFSCatalogNodeID	nextCatalogID;

		uint32_t			writeCount;
		uint64_t			encodingsBitmap;

		uint32_t			finderInfo[8];

		HFSPlusForkData		allocationFile;
		HFSPlusForkData		extentsFile;
		HFSPlusForkData		catalogFile;
		HFSPlusForkData		attributesFile;
		HFSPlusForkData		startupFile;

	} __attribute__((packed));








































	class FSDriverHFSPlus : public FSDriver
	{
		public:
			FSDriverHFSPlus(Devices::Storage::Partition* part);
			virtual ~FSDriverHFSPlus() override;
			virtual bool Create(VFS::vnode* node, const char* path, uint64_t flags, uint64_t perms) override;
			virtual bool Delete(VFS::vnode* node, const char* path) override;
			virtual bool Traverse(VFS::vnode* node, const char* path, char** symlink) override;
			virtual size_t Read(VFS::vnode* node, void* buf, off_t offset, size_t length) override;
			virtual size_t Write(VFS::vnode* node, const void* buf, off_t offset, size_t length) override;
			virtual void Flush(VFS::vnode* node) override;
			virtual void Stat(VFS::vnode* node, struct stat* stat, bool statlink) override;

			// returns a list of items inside the directory, as vnodes.
			virtual rde::vector<VFS::vnode*>* ReadDir(VFS::vnode* node) override;


		private:
			HFSPlusVolumeHeader		volumeHeader;
	};
}
}
}






