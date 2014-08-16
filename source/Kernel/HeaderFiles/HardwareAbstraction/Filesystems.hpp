// Filesystems.hpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#pragma once
#include <stdint.h>
#include <List.hpp>
#include "Devices/StorageDevice.hpp"
#include <rdestl/vector.h>
#include <rdestl/rde_string.h>
#include <sys/stat.h>

namespace Kernel
{
	namespace HardwareAbstraction
	{
		namespace Multitasking
		{
			struct Process;
		}

		namespace Filesystems
		{
			class FSDriver;
			struct IOContext;
			typedef long fd_t;
			namespace VFS
			{
				enum VFSError
				{
					NO_ERROR = 0,
					NOT_FOUND,
				};

				enum class VNodeType
				{
					None,
					File,
					Folder,
				};

				struct fsref
				{
					dev_t id;
					void* data;
					FSDriver* driver;
				};

				struct vnode
				{
					id_t id;
					fsref* info;
					void* data;
					VNodeType type;
					uint64_t attrib;
					uint64_t refcount;
				};

				struct fileentry
				{
					vnode* node;
					off_t offset;
					uint64_t flags;
					fd_t fd;
				};

				struct FDArray
				{
					FDArray()
					{
						this->fds = new rde::vector<fileentry>();
					}

					rde::vector<fileentry>* fds;
				};

				void Initialise();
				vnode* NodeFromID(id_t id);
				fd_t FDFromNode(IOContext* ioctx, vnode* node);
				vnode* NodeFromFD(IOContext* ioctx, fd_t fd);

				vnode* CreateNode(IOContext* ioctx, FSDriver* fs);
				vnode* DuplicateNode(IOContext* ioctx, vnode* orig);
				void DeleteNode(IOContext* ioctx, vnode* node);

				vnode* Reference(IOContext* ioctx, vnode* node);
				vnode* Dereference(IOContext* ioctx, vnode* node);

				void Mount(Devices::Storage::Partition* partition, FSDriver* fs, const char* path);
				void Unmount(const char* path);

				fileentry* Open(IOContext* ioctx, vnode* node, int flags);
				fileentry* OpenFile(IOContext* ioctx, const char* path, int flags);

				size_t Read(IOContext* ioctx, vnode* node, void* buf, off_t off, size_t len);
				size_t Write(IOContext* ioctx, vnode* node, void* buf, off_t off, size_t len);
			}

			fd_t OpenFile(const char* path, int flags);
			size_t Read(fd_t fd, void* buf, off_t off, size_t len);
			size_t Write(fd_t fd, void* buf, off_t off, size_t len);

			struct IOContext
			{
				IOContext()
				{
					this->fdarray = new VFS::FDArray();
				}

				VFS::FDArray* fdarray;
			};

			class FSDriver
			{
				public:
					FSDriver(Devices::Storage::Partition* part) : partition(part) { }
					virtual ~FSDriver();
					virtual bool Traverse(VFS::vnode* node, const char* path, char** symlink);
					virtual size_t Read(VFS::vnode* node, void* buf, off_t offset, size_t length);
					virtual size_t Write(VFS::vnode* node, const void* buf, off_t offset, size_t length);
					virtual void Stat(VFS::vnode* node, stat* stat);

					// returns a list of items inside the directory, as vnodes.
					virtual rde::vector<VFS::vnode*>* ReadDir(VFS::vnode* node);

					virtual dev_t GetID() final { return this->fsid; }

				protected:
					dev_t fsid;
					Devices::Storage::Partition* partition;
			};

			class FSDriverFat32 : public FSDriver
			{
				public:
					FSDriverFat32(Devices::Storage::Partition* part);
					~FSDriverFat32() override;
					bool Traverse(VFS::vnode* node, const char* path, char** symlink) override;
					size_t Read(VFS::vnode* node, void* buf, off_t offset, size_t length) override;
					size_t Write(VFS::vnode* node, const void* buf, off_t offset, size_t length) override;
					void Stat(VFS::vnode* node, stat* stat) override;

					rde::vector<VFS::vnode*>* ReadDir(VFS::vnode* node) override;

				private:
					rde::string* ReadLFN(uint64_t addr);
					uint64_t ClusterToLBA(uint32_t clus);
					rde::vector<uint32_t>* GetClusterChain(VFS::vnode* node, uint64_t* numclus);

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
					uint16_t BackupBootCluster;

					uint64_t FirstUsableCluster;
			};




			// class FSDriver
			// {
			// 	public:
			// 		FSDriver(FSTypes type);
			// 		virtual ~FSDriver();

			// 		virtual Devices::Storage::Partition* GetPartition();
			// 		virtual VFS::Filesystem* RootFS();

			// 		virtual void PrintInfo();
			// 		virtual VFS::Folder* GetRootFolder() = 0;
			// 		virtual Library::LinkedList<VFS::FSObject>* GetFSObjects(VFS::Folder* start) = 0;

			// 		virtual void ReadFile(VFS::File* File, uint64_t Address, uint64_t length) = 0;
			// 		virtual void WriteFile(VFS::File* File, uint64_t Address, uint64_t length) = 0;
			// 		virtual void AppendFile(VFS::File* File, uint64_t Address, uint64_t length, uint64_t offset) = 0;

			// 		FSTypes Type;

			// 	protected:
			// 		Devices::Storage::Partition* ParentPartition;
			// 		VFS::Filesystem* rootfs;
			// };

			// class FAT32 : public FSDriver
			// {
			// 	public:
			// 		FAT32(Devices::Storage::Partition* Parent);


			// 		virtual VFS::Folder* GetRootFolder() override;
			// 		virtual void ReadFile(VFS::File* File, uint64_t Address, uint64_t length) override;
			// 		virtual void WriteFile(VFS::File* File, uint64_t Address, uint64_t length) override;
			// 		virtual void AppendFile(VFS::File* File, uint64_t Address, uint64_t length, uint64_t offset) override;
			// 		virtual Library::LinkedList<VFS::FSObject>* GetFSObjects(VFS::Folder* start) override;
			// 		virtual void PrintInfo() override;


			// 		uint16_t		GetBytesPerSector();
			// 		uint8_t			GetSectorsPerCluster();
			// 		uint16_t		GetReservedSectors();
			// 		uint8_t			GetNumberOfFATS();
			// 		uint16_t		GetNumberOfDirectories();

			// 		uint32_t		GetTotalSectors();
			// 		uint32_t		GetHiddenSectors();
			// 		uint32_t		GetFATSectorSize();
			// 		uint32_t		GetRootDirectoryCluster();

			// 		uint16_t		GetFSInfoCluster();
			// 		uint16_t		GetBackupBootCluster();
			// 		uint32_t		GetFirstUsableCluster();


			// 	private:
			// 		char*			GetFileName(const char* filename);
			// 		char*			GetFolderName(const char* foldername);
			// 		uint32_t		AllocateCluster(uint32_t PreviousCluster = 0);

			// 		uint16_t BytesPerSector;
			// 		uint8_t SectorsPerCluster;
			// 		uint16_t ReservedSectors;
			// 		uint8_t NumberOfFATs;
			// 		uint16_t NumberOfDirectories;

			// 		uint32_t TotalSectors;
			// 		uint32_t HiddenSectors;

			// 		uint32_t FATSectorSize;
			// 		uint32_t RootDirectoryCluster;
			// 		uint16_t FSInfoCluster;
			// 		uint16_t BackupBootCluster;

			// 		uint64_t FirstUsableCluster;

			// };



			// class HFSPlus : public FSDriver
			// {
			// 	struct VolumeHeader_type
			// 	{
			// 		uint8_t signature[2];
			// 		uint16_t version;
			// 		uint32_t attributes;
			// 		uint32_t lastmountedversion;
			// 		uint32_t journalinfoblock;

			// 		uint32_t createdate;
			// 		uint32_t modifydate;
			// 		uint32_t backupdate;
			// 		uint32_t checkeddate;

			// 		uint32_t filecount;
			// 		uint32_t foldercount;

			// 		uint32_t blocksize;
			// 		uint32_t totalblocks;
			// 		uint32_t freeblocks;

			// 		uint32_t nextalloc;
			// 		uint32_t resourceclumpsize;
			// 		uint32_t dataclumpsize;



			// 		uint32_t writecount;
			// 		uint64_t encodingbitmap;
			// 		uint32_t finderinfo[8];
			// 	} __attribute__ ((packed));


			// 	public:
			// 		HFSPlus(Devices::Storage::Partition* parent);

			// 		virtual VFS::Folder* GetRootFolder() override;
			// 		virtual void ReadFile(VFS::File* File, uint64_t Address, uint64_t length) override;
			// 		virtual void WriteFile(VFS::File* File, uint64_t Address, uint64_t length) override;
			// 		virtual void AppendFile(VFS::File* File, uint64_t Address, uint64_t length, uint64_t offset) override;
			// 		virtual Library::LinkedList<VFS::FSObject>* GetFSObjects(VFS::Folder* start) override;
			// 		virtual void PrintInfo() override;

			// 	private:
			// 		VolumeHeader_type* volumeheader;
			// };

			namespace MBR
			{
				void ReadPartitions(Devices::Storage::StorageDevice* atadev);
			}

			namespace GPT
			{
				void ReadPartitions(Devices::Storage::StorageDevice* atadev);
			}
		}
	}
}
