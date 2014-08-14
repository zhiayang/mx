// Filesystems.hpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#pragma once
#include <stdint.h>
#include <List.hpp>
#include "Devices/StorageDevice.hpp"

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

			namespace VFS
			{
				enum class FSObjectTypes
				{
					Invalid,
					File,
					Folder,
					Filesystem,
					Socket,
					IPCSocket
				};

				extern const uint8_t Attr_ReadOnly;
				extern const uint8_t Attr_Hidden;

				class Filesystem;
				class FSObject
				{
					public:
						FSObject(FSObjectTypes type);
						FSObject(const FSObject& other);
						virtual ~FSObject();

						virtual const char* Name();
						virtual const char* Path();
						virtual FSObject* Parent();
						virtual Filesystem* RootFS();
						virtual uint8_t Attributes();
						virtual bool Exists();
						FSObjectTypes Type;

					protected:
						FSObject* parent;
						char* _name;
						char* _path;
						Filesystem* rootfs;
						uint8_t _attributes;
						bool _exists;
				};

				class Folder : public FSObject
				{
					public:
						Folder(const char* path, uint64_t clus, Filesystem* rfs, uint8_t attr);
						Folder(const char* path);

						virtual const char* Name() override;
						virtual const char* Path() override;
						virtual Folder* Parent() override;
						virtual uint8_t Attributes() override;
						virtual Filesystem* RootFS() override;
						virtual bool Exists() override;

						uint64_t Cluster();

					private:
						uint64_t _cluster;
				};

				class File : public FSObject
				{
					public:
						File(const char* nm, uint64_t sz, uint64_t cluster, Filesystem* rfs, uint8_t attr);
						File(const char* path);

						virtual const char* Name() override;
						virtual const char* Path() override;
						virtual Folder* Parent() override;
						virtual uint8_t Attributes() override;
						virtual Filesystem* RootFS() override;
						virtual bool Exists() override;

						uint64_t FileSize();
						uint64_t Cluster();
						uint64_t Read(uint8_t* buffer, uint64_t length = 0);
						uint64_t Write(uint8_t* buffer, uint64_t length = 0);
						uint64_t Append(uint8_t* buffer, uint64_t length = 0, uint64_t offset = 0);

					private:
						uint64_t _FileSize;
						uint64_t _Cluster;
				};


				class Filesystem : public FSObject
				{
					public:
						Filesystem(const char* name, FSDriver* fsdriver);

						virtual const char* Name() override;
						virtual FSObject* Parent() override;
						virtual const char* Path() override;
						virtual Filesystem* RootFS() override;
						virtual uint8_t Attributes() override;
						virtual bool Exists() override;

						FSObject* GetFSObject(const char* path);
						FSDriver* GetFSDriver();
						static Library::LinkedList<Filesystem>* Filesystems;

					private:
						FSDriver* fsdriver;
				};

				Filesystem* GetFilesystem(const char* path);

				// userspace fd things.
				extern const uint64_t MaxDescriptors;
				extern const uint64_t ReservedStreams;
				struct FileDescriptor
				{
					FSObject* Pointer;
				};

				uint64_t GetAndIncrementDescriptor(Kernel::HardwareAbstraction::Multitasking::Process* proc);
				uint64_t OpenFile(const char* path, uint8_t mode);
				uint64_t FileSize(uint64_t fd);
				void CloseFile(uint64_t fd);
				uint64_t ReadFile(uint64_t fd, uint8_t* buffer, uint64_t length);
				uint64_t WriteFile(uint64_t fd, uint8_t* buffer, uint64_t length);

				uint64_t OpenFolder(const char* path);
				void CloseFolder(uint64_t fd);
				Library::LinkedList<char>* ListObjects(uint64_t fd, Library::LinkedList<char>* output, uint64_t* items);
			}






			class FSDriver
			{
				public:
					FSDriver(FSTypes type);
					virtual ~FSDriver();

					virtual Devices::Storage::Partition* GetPartition();
					virtual VFS::Filesystem* RootFS();

					virtual void PrintInfo();
					virtual VFS::Folder* GetRootFolder() = 0;
					virtual Library::LinkedList<VFS::FSObject>* GetFSObjects(VFS::Folder* start) = 0;

					virtual void ReadFile(VFS::File* File, uint64_t Address, uint64_t length) = 0;
					virtual void WriteFile(VFS::File* File, uint64_t Address, uint64_t length) = 0;
					virtual void AppendFile(VFS::File* File, uint64_t Address, uint64_t length, uint64_t offset) = 0;

					FSTypes Type;

				protected:
					Devices::Storage::Partition* ParentPartition;
					VFS::Filesystem* rootfs;
			};

			class FAT32 : public FSDriver
			{
				public:
					FAT32(Devices::Storage::Partition* Parent);


					virtual VFS::Folder* GetRootFolder() override;
					virtual void ReadFile(VFS::File* File, uint64_t Address, uint64_t length) override;
					virtual void WriteFile(VFS::File* File, uint64_t Address, uint64_t length) override;
					virtual void AppendFile(VFS::File* File, uint64_t Address, uint64_t length, uint64_t offset) override;
					virtual Library::LinkedList<VFS::FSObject>* GetFSObjects(VFS::Folder* start) override;
					virtual void PrintInfo() override;


					uint16_t		GetBytesPerSector();
					uint8_t			GetSectorsPerCluster();
					uint16_t		GetReservedSectors();
					uint8_t			GetNumberOfFATS();
					uint16_t		GetNumberOfDirectories();

					uint32_t		GetTotalSectors();
					uint32_t		GetHiddenSectors();
					uint32_t		GetFATSectorSize();
					uint32_t		GetRootDirectoryCluster();

					uint16_t		GetFSInfoCluster();
					uint16_t		GetBackupBootCluster();
					uint32_t		GetFirstUsableCluster();


				private:
					char*			GetFileName(const char* filename);
					char*			GetFolderName(const char* foldername);
					uint32_t		AllocateCluster(uint32_t PreviousCluster = 0);

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



			class HFSPlus : public FSDriver
			{
				struct VolumeHeader_type
				{
					uint8_t signature[2];
					uint16_t version;
					uint32_t attributes;
					uint32_t lastmountedversion;
					uint32_t journalinfoblock;

					uint32_t createdate;
					uint32_t modifydate;
					uint32_t backupdate;
					uint32_t checkeddate;

					uint32_t filecount;
					uint32_t foldercount;

					uint32_t blocksize;
					uint32_t totalblocks;
					uint32_t freeblocks;

					uint32_t nextalloc;
					uint32_t resourceclumpsize;
					uint32_t dataclumpsize;



					uint32_t writecount;
					uint64_t encodingbitmap;
					uint32_t finderinfo[8];
				} __attribute__ ((packed));


				public:
					HFSPlus(Devices::Storage::Partition* parent);

					virtual VFS::Folder* GetRootFolder() override;
					virtual void ReadFile(VFS::File* File, uint64_t Address, uint64_t length) override;
					virtual void WriteFile(VFS::File* File, uint64_t Address, uint64_t length) override;
					virtual void AppendFile(VFS::File* File, uint64_t Address, uint64_t length, uint64_t offset) override;
					virtual Library::LinkedList<VFS::FSObject>* GetFSObjects(VFS::Folder* start) override;
					virtual void PrintInfo() override;

				private:
					VolumeHeader_type* volumeheader;
			};

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
