// VFS.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <string.h>
#include <Kernel.hpp>
#include <rdestl/hash_map.h>
#include <unistd.h>

using namespace Kernel::HardwareAbstraction::Devices::Storage;
namespace Kernel {
namespace HardwareAbstraction {
namespace Filesystems
{
	namespace VFS
	{
		static id_t curid = 0;
		static id_t curfeid = 0;
		static fd_t FirstFreeFD = 0;

		struct Filesystem
		{
			FSDriver* driver;
			Partition* partition;
			rde::string* mountpoint;
			bool ismounted;
		};

		static Library::Vector<Filesystem*>* mountedfses;
		static rde::hash_map<id_t, vnode*>* vnodepool;

		FSDriver* driver_stdin;
		FSDriver* driver_stdout;

		static IOContext* getctx()
		{
			auto proc = Multitasking::GetCurrentProcess();
			assert(proc);
			assert(proc->iocontext);

			return proc->iocontext;
		}

		static Filesystem* getfs(rde::string& path)
		{
			for(auto v : *mountedfses)
			{
				if(strcmp(path.substr(0, math::min(path.length(), v->mountpoint->length())).c_str(), v->mountpoint->c_str()) == 0)
					return v;
			}

			return nullptr;
		}


		void Initialise()
		{
			mountedfses = new Library::Vector<Filesystem*>();
			vnodepool = new rde::hash_map<id_t, vnode*>();
		}

		void InitIO()
		{
			auto ConsoleFSD = new FSDriverConsole();
			driver_stdin = new FSDriverStdin();
			driver_stdout = new FSDriverStdout();

			Mount(nullptr, ConsoleFSD, "/dev/console");
			Mount(nullptr, driver_stdin, "/dev/stdin");
			Mount(nullptr, driver_stdout, "/dev/stdout");

			auto ctx = getctx();
			OpenFile(ctx, "/dev/stdin", 0);
			OpenFile(ctx, "/dev/stdout", 0);
		}

		// this fetches from the pool. used mainly by fsdrivers to avoid creating duplicate vnodes.
		vnode* NodeFromID(id_t id)
		{
			auto v = vnodepool->find(id);
			if(v == vnodepool->end())
				return nullptr;

			return v->second;
		}

		fileentry* FileEntryFromFD(IOContext* ioctx, fd_t fd)
		{
			assert(ioctx);
			assert(ioctx->fdarray);
			assert(ioctx->fdarray->fds);

			for(auto v : *ioctx->fdarray->fds)
			{
				if(v->fd == fd)
					return v;
			}

			return nullptr;
		}

		vnode* NodeFromFD(IOContext* ioctx, fd_t fd)
		{
			auto ret = FileEntryFromFD(ioctx, fd);
			if(ret)
				return ret->node;

			return nullptr;
		}

		vnode* CreateNode(FSDriver* fs)
		{
			vnode* node = new vnode;

			assert(node);
			memset(node, 0, sizeof(vnode));

			node->data = nullptr;
			node->info = new fsref;
			node->refcount = 1;
			node->type = VNodeType::None;
			node->attrib = 0;

			memset(node->info, 0, sizeof(fsref));
			node->info->data = nullptr;
			node->info->driver = fs;
			node->info->id = fs->GetID();

			// add the node to the pool.
			node->id = curid++;
			(*vnodepool)[node->id] = node;
			return node;
		}

		void DeleteNode(vnode* node)
		{
			(void) node;
		}

		vnode* Reference(vnode* node)
		{
			assert(node);
			node->refcount++;

			return node;
		}

		vnode* Dereference(vnode* node)
		{
			assert(node);

			node->refcount--;
			if(node->refcount == 0)
			{
				DeleteNode(node);
				return nullptr;
			}

			return node;
		}

		void Mount(Partition* part, FSDriver* fs, const char* path)
		{
			assert(fs);
			if(fs->GetType() == FSDriverType::Physical)
				assert(part);

			assert(path);

			auto _fs = new Filesystem;
			_fs->driver = fs;
			_fs->ismounted = true;
			_fs->mountpoint = new rde::string(path);
			_fs->partition = part;

			mountedfses->InsertBack(_fs);
		}

		void Unmount(const char* path)
		{
			(void) path;
		}

		fileentry* Open(IOContext* ioctx, vnode* node, int flags)
		{
			assert(ioctx);
			assert(ioctx->fdarray);
			assert(ioctx->fdarray->fds);
			assert(node);

			auto fe		= new fileentry;
			fe->node	= node;
			fe->offset	= 0;
			fe->flags	= flags;
			fe->fd		= FirstFreeFD + ioctx->fdarray->fds->Size();
			fe->id		= curfeid++;

			ioctx->fdarray->fds->InsertBack(fe);

			return fe;
		}

		fileentry* OpenFile(IOContext* ioctx, const char* path, int flags)
		{
			assert(ioctx);
			assert(path);

			if(!mountedfses)
				return nullptr;

			rde::string pth = rde::string(path);
			Filesystem* fs = getfs(pth);

			if(fs == nullptr || fs->ismounted == false)
				return nullptr;

			auto node = VFS::CreateNode(fs->driver);
			assert(node);
			node->type = VNodeType::File;

			// this ought to fill in the information in node.
			assert(fs);
			assert(fs->driver);

			bool res = fs->driver->Traverse(node, path, nullptr);
			if(res)
			{
				auto ret = VFS::Open(ioctx, node, flags);
				return ret;
			}
			else
				return nullptr;
		}

		size_t Read(IOContext* ioctx, vnode* node, void* buf, off_t off, size_t len)
		{
			assert(ioctx);
			assert(node);
			assert(node->info);
			assert(node->refcount > 0);
			assert(node->info->driver);
			assert(buf);

			auto fs = node->info->driver;
			assert(fs);

			return fs->Read(node, buf, off, len);
		}

		size_t Write(IOContext* ioctx, vnode* node, void* buf, off_t off, size_t len)
		{
			assert(ioctx);
			assert(node);
			assert(node->info);
			assert(node->refcount > 0);
			assert(node->info->driver);

			auto fs = node->info->driver;
			return fs->Write(node, buf, off, len);
		}

		void Stat(IOContext* ioctx, vnode* node, struct stat* st)
		{
			assert(ioctx);
			assert(node);
			assert(node->info);
			assert(node->refcount > 0);
			assert(node->info->driver);

			auto fs = node->info->driver;
			fs->Stat(node, st);
		}

		void Seek(fileentry* fe, off_t offset, int origin)
		{
			assert(fe);
			assert(fe->node);

			if(origin == SEEK_SET)
				fe->offset = 0;

			fe->offset += offset;
		}
	}


	// namespace VFS ends here
	using namespace VFS;
	fd_t OpenFile(const char* path, int flags)
	{
		assert(path);
		auto ctx = getctx();

		auto fe = VFS::OpenFile(ctx, path, flags);
		return fe ? fe->fd : -1;
	}

	size_t Read(fd_t fd, void* buf, size_t len)
	{
		if(len == 0)
			return 0;

		auto ctx = getctx();
		auto fe = VFS::FileEntryFromFD(ctx, fd);
		if(fe == nullptr)
			return 0;

		assert(fe->node);
		auto read = VFS::Read(ctx, fe->node, buf, fe->offset, len);
		fe->offset += read;
		return read;
	}

	size_t Write(fd_t fd, void* buf, size_t len)
	{
		if(len == 0)
			return 0;

		auto ctx = getctx();
		auto fe = VFS::FileEntryFromFD(ctx, fd);
		if(fe == nullptr)
			return 0;

		assert(fe->node);
		auto written = VFS::Write(ctx, fe->node, buf, fe->offset, len);

		fe->offset += written;
		return written;
	}

	VFSError Stat(fd_t fd, struct stat* out)
	{
		auto ctx = getctx();
		if(fd < 0)
			return VFSError::NOT_FOUND;

		auto node = VFS::NodeFromFD(ctx, fd);
		if(node == nullptr)
			return VFSError::NOT_FOUND;

		VFS::Stat(ctx, node, out);
		return VFSError::NO_ERROR;
	}

	void Seek(fd_t fd, off_t offset, int origin)
	{
		auto ctx = getctx();
		if(fd < 0)
			// todo: set errno
			return;

		auto fe = VFS::FileEntryFromFD(ctx, fd);
		if(fe == nullptr)
			return;

		VFS::Seek(fe, offset, origin);
	}
}
}
}




