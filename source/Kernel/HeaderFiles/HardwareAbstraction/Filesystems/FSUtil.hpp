// FSUtil.hpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#pragma once
#include <stdint.h>
#include <sys/types.h>
#include <rdestl/rdestl.h>

#include <CircularBuffer.hpp>
#include "../Devices/StorageDevice.hpp"


typedef long fd_t;

namespace Kernel {
namespace HardwareAbstraction {

namespace Multitasking
{
	struct Process;
}

namespace Filesystems
{
	class FSDriver;
	struct IOContext;

	enum Attributes
	{
		ReadOnly	= 0x1,
		Hidden		= 0x2,
	};

	namespace VFS
	{
		enum class VNodeType
		{
			None = 0,
			File,
			Folder,
			Socket,
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
			rde::vector<fileentry*> fds;
		};


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
		err_t Stat(IOContext* ioctx, vnode* node, struct stat* st, bool statlink);
		err_t Seek(fileentry* fe, off_t offset, int origin);
		err_t Flush(IOContext* ioctx, vnode* node);
		err_t Close(IOContext* ioctx, fileentry* node);
		void CloseAll(IOContext* ioctx);
	}

	fd_t OpenFile(const char* path, int flags);
	err_t Close(fd_t fd);
	void CloseAll(Multitasking::Process* p);
	size_t Read(fd_t fd, void* buf, size_t len);
	size_t Write(fd_t fd, void* buf, size_t len);

	err_t Flush(fd_t fd);
	err_t Seek(fd_t, off_t offset, int origin);
	err_t Stat(fd_t fd, struct stat* out, bool statlink = false);
	fd_t Duplicate(fd_t old);
	uint64_t GetSeekPos(fd_t fd);

	enum class FSDriverType
	{
		Invalid = 0,
		Physical,
		Virtual
	};

	struct IOContext
	{
		VFS::FDArray fdarray;
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
			virtual void Flush(VFS::vnode* node);
			virtual void Stat(VFS::vnode* node, struct stat* stat, bool statlink);

			// returns a list of items inside the directory, as vnodes.
			virtual rde::vector<VFS::vnode*> ReadDir(VFS::vnode* node);

			virtual dev_t GetID() final { return this->fsid; }
			virtual FSDriverType GetType() final { return this->_type; }
			virtual bool Seekable() final { return this->_seekable; }

		protected:
			Devices::Storage::Partition* partition;
			FSDriverType _type;
			dev_t fsid;
			bool _seekable;
	};

}

}
}
