// Fat32VFS.hpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#pragma once
#include "FSUtil.hpp"

namespace Kernel {
namespace HardwareAbstraction {
namespace Filesystems
{
	class FSDriverFat32 : public FSDriver
	{
		public:
			FSDriverFat32(Devices::Storage::Partition* part);
			virtual ~FSDriverFat32() override;
			virtual bool Create(VFS::vnode* node, const char* path, uint64_t flags, uint64_t perms) override;
			virtual bool Delete(VFS::vnode* node, const char* path) override;
			virtual bool Traverse(VFS::vnode* node, const char* path, char** symlink) override;
			virtual size_t Read(VFS::vnode* node, void* buf, off_t offset, size_t length) override;
			virtual size_t Write(VFS::vnode* node, const void* buf, off_t offset, size_t length) override;
			virtual void Stat(VFS::vnode* node, struct stat* stat, bool statlink) override;
			virtual void Flush(VFS::vnode* node) override;
			virtual void Close(VFS::vnode* node) override;

			virtual rde::vector<VFS::vnode*> ReadDir(VFS::vnode* node) override;

		private:
			rde::string ReadLFN(uint64_t addr, int* nument);
			uint64_t ClusterToLBA(uint32_t clus);
			rde::vector<uint32_t> GetClusterChain(VFS::vnode* node, uint64_t* numclus);

			uint16_t BytesPerSector;
			uint8_t SectorsPerCluster;
			uint16_t ReservedSectors;
			uint8_t NumberOfFATs;
			uint16_t NumberOfDirectories;

			uint32_t TotalSectors;
			uint32_t HiddenSectors;

			uint32_t FATSectorSize;
			uint32_t RootDirectoryCluster;
			uint16_t FSInfoCluster;
			uint16_t backupBootCluster;

			uint64_t FirstUsableCluster;
	};
}
}
}
