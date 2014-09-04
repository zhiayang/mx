// Filesystems.hpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#pragma once
#include <stdint.h>
#include <CircularBuffer.hpp>
#include "Devices/StorageDevice.hpp"
#include <Vector.hpp>
#include <rdestl/rdestl.h>
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

			enum Attributes
			{
				ReadOnly	= 0x1,
				Hidden		= 0x2,
			};

			namespace VFS
			{
				enum VFSError
				{
					NO_ERROR = 0,
					NOT_FOUND,
				};

				enum class VNodeType
				{
					None = 0,
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
					id_t id;
					fd_t fd;
				};

				struct FDArray
				{
					FDArray()
					{
						this->fds = new Library::Vector<fileentry*>();
					}

					Library::Vector<fileentry*>* fds;
				};

				extern FSDriver* driver_stdin;
				extern FSDriver* driver_stdout;
				extern FSDriver* driver_ipcmsg;

				void Initialise();
				void InitIO();

				vnode* NodeFromID(id_t id);
				fd_t FDFromNode(IOContext* ioctx, vnode* node);
				vnode* NodeFromFD(IOContext* ioctx, fd_t fd);

				vnode* CreateNode(FSDriver* fs);
				vnode* DuplicateNode(vnode* orig);
				void DeleteNode(vnode* node);

				vnode* Reference(vnode* node);
				vnode* Dereference(vnode* node);

				void Mount(Devices::Storage::Partition* partition, FSDriver* fs, const char* path);
				void Unmount(const char* path);

				fileentry* Open(IOContext* ioctx, vnode* node, int flags);
				fileentry* OpenFile(IOContext* ioctx, const char* path, int flags);

				size_t Read(IOContext* ioctx, vnode* node, void* buf, off_t off, size_t len);
				size_t Write(IOContext* ioctx, vnode* node, void* buf, off_t off, size_t len);
				void Stat(IOContext* ioctx, vnode* node, struct stat* st);
				void Seek(fileentry* fe, off_t offset, int origin);
			}

			fd_t OpenFile(const char* path, int flags);
			size_t Read(fd_t fd, void* buf, size_t len);
			size_t Write(fd_t fd, void* buf, size_t len);
			void Seek(fd_t, off_t offset, int origin);
			VFS::VFSError Stat(fd_t fd, struct stat* out);
			fd_t Duplicate(fd_t old);

			enum class FSDriverType
			{
				Invalid = 0,
				Physical,
				Virtual
			};

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
					FSDriver(Devices::Storage::Partition* part, FSDriverType type) : partition(part), _type(type) { }
					virtual ~FSDriver();
					virtual bool Create(VFS::vnode* node, const char* path, uint64_t flags, uint64_t perms);
					virtual bool Delete(VFS::vnode* node, const char* path);
					virtual bool Traverse(VFS::vnode* node, const char* path, char** symlink);
					virtual size_t Read(VFS::vnode* node, void* buf, off_t offset, size_t length);
					virtual size_t Write(VFS::vnode* node, const void* buf, off_t offset, size_t length);
					virtual void Stat(VFS::vnode* node, struct stat* stat);

					// returns a list of items inside the directory, as vnodes.
					virtual Library::Vector<VFS::vnode*>* ReadDir(VFS::vnode* node);

					virtual dev_t GetID() final { return this->fsid; }
					virtual FSDriverType GetType() final { return this->_type; }

				protected:
					Devices::Storage::Partition* partition;
					FSDriverType _type;
					dev_t fsid;
			};

			class FSDriverConsole : public FSDriver
			{
				public:
					FSDriverConsole();
					~FSDriverConsole() override;
					bool Create(VFS::vnode* node, const char* path, uint64_t flags, uint64_t perms) override;
					bool Delete(VFS::vnode* node, const char* path) override;
					bool Traverse(VFS::vnode* node, const char* path, char** symlink) override;
					size_t Read(VFS::vnode* node, void* buf, off_t offset, size_t length) override;
					size_t Write(VFS::vnode* node, const void* buf, off_t offset, size_t length) override;
					void Stat(VFS::vnode* node, struct stat* stat) override;

					Library::Vector<VFS::vnode*>* ReadDir(VFS::vnode* node) override;
			};

			class FSDriverStdin : public FSDriver
			{
				public:
					FSDriverStdin();
					~FSDriverStdin() override;
					bool Create(VFS::vnode* node, const char* path, uint64_t flags, uint64_t perms) override;
					bool Delete(VFS::vnode* node, const char* path) override;
					bool Traverse(VFS::vnode* node, const char* path, char** symlink) override;
					size_t Read(VFS::vnode* node, void* buf, off_t offset, size_t length) override;
					size_t Write(VFS::vnode* node, const void* buf, off_t offset, size_t length) override;
					void Stat(VFS::vnode* node, struct stat* stat) override;

					Library::Vector<VFS::vnode*>* ReadDir(VFS::vnode* node) override;
			};

			class FSDriverStdout : public FSDriver
			{
				public:
					FSDriverStdout();
					~FSDriverStdout() override;
					bool Create(VFS::vnode* node, const char* path, uint64_t flags, uint64_t perms) override;
					bool Delete(VFS::vnode* node, const char* path) override;
					bool Traverse(VFS::vnode* node, const char* path, char** symlink) override;
					size_t Read(VFS::vnode* node, void* buf, off_t offset, size_t length) override;
					size_t Write(VFS::vnode* node, const void* buf, off_t offset, size_t length) override;
					void Stat(VFS::vnode* node, struct stat* stat) override;

					Library::Vector<VFS::vnode*>* ReadDir(VFS::vnode* node) override;
			};

			class FSDriverIPCMsg : public FSDriver
			{
				struct pathid
				{
					char* path;
					id_t id;
				};

				public:
					FSDriverIPCMsg();
					~FSDriverIPCMsg() override;
					bool Create(VFS::vnode* node, const char* path, uint64_t flags, uint64_t perms) override;
					bool Delete(VFS::vnode* node, const char* path) override;
					bool Traverse(VFS::vnode* node, const char* path, char** symlink) override;
					size_t Read(VFS::vnode* node, void* buf, off_t offset, size_t length) override;
					size_t Write(VFS::vnode* node, const void* buf, off_t offset, size_t length) override;
					void Stat(VFS::vnode* node, struct stat* stat) override;

					Library::Vector<VFS::vnode*>* ReadDir(VFS::vnode* node) override;

					rde::hash_map<pathid*, Library::CircularMemoryBuffer*>* messagequeue = nullptr;
			};





			class FSDriverFat32 : public FSDriver
			{
				public:
					FSDriverFat32(Devices::Storage::Partition* part);
					~FSDriverFat32() override;
					bool Create(VFS::vnode* node, const char* path, uint64_t flags, uint64_t perms) override;
					bool Delete(VFS::vnode* node, const char* path) override;
					bool Traverse(VFS::vnode* node, const char* path, char** symlink) override;
					size_t Read(VFS::vnode* node, void* buf, off_t offset, size_t length) override;
					size_t Write(VFS::vnode* node, const void* buf, off_t offset, size_t length) override;
					void Stat(VFS::vnode* node, struct stat* stat) override;

					Library::Vector<VFS::vnode*>* ReadDir(VFS::vnode* node) override;

				private:
					rde::string* ReadLFN(uint64_t addr, int& nument);
					uint64_t ClusterToLBA(uint32_t clus);
					Library::Vector<uint32_t>* GetClusterChain(VFS::vnode* node, uint64_t* numclus);

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
