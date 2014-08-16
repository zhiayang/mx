// VFS.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <string.h>
#include <Kernel.hpp>
#include <rdestl/hash_map.h>

using namespace Kernel::HardwareAbstraction::Devices::Storage;
namespace Kernel {
namespace HardwareAbstraction {
namespace Filesystems
{
	namespace VFS
	{
		static id_t curid = 0;
		const static long FirstFreeFD = 3;

		struct Filesystem
		{
			FSDriver* driver;
			Partition* partition;
			std::string* mountpoint;
			bool ismounted;
		};

		static IOContext* getctx()
		{
			auto proc = Multitasking::GetCurrentProcess();
			assert(proc);
			assert(proc->iocontext);

			return proc->iocontext;
		}

		static rde::vector<Filesystem*>* mountedfses;
		static rde::hash_map<id_t, vnode*>* vnodepool;

		void Initialise()
		{
			mountedfses = new rde::vector<Filesystem*>();
			vnodepool = new rde::hash_map<id_t, vnode*>();
		}

		// this fetches from the pool. used mainly by fsdrivers to avoid creating duplicate vnodes.
		vnode* NodeFromID(id_t id)
		{
			auto v = vnodepool->find(id);
			if(v == vnodepool->end())
				return nullptr;

			return v->second;
		}

		vnode* NodeFromFD(IOContext* ioctx, fd_t fd)
		{
			assert(ioctx);
			assert(ioctx->fdarray);
			assert(ioctx->fdarray->fds);

			for(auto v : *ioctx->fdarray->fds)
			{
				if(v->fd == fd)
					return v->node;
			}

			return nullptr;
		}

		vnode* CreateNode(FSDriver* fs)
		{
			vnode* node = new vnode;
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
			assert(part);
			assert(fs);
			assert(path);

			auto _fs = new Filesystem;
			_fs->driver = fs;
			_fs->ismounted = true;
			_fs->mountpoint = new std::string(path);
			_fs->partition = part;

			mountedfses->push_back(_fs);
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
			fe->fd		= FirstFreeFD + ioctx->fdarray->fds->size();

			ioctx->fdarray->fds->reserve(4);
			ioctx->fdarray->fds->push_back(fe);

			return fe;
		}

		fileentry* OpenFile(IOContext* ioctx, const char* path, int flags)
		{
			assert(ioctx);
			assert(path);

			if(!mountedfses)
				return nullptr;

			Filesystem* fs = nullptr;
			std::string pth;
			pth += path;

			for(auto v : *mountedfses)
			{
				auto tmp = pth.substr(0, 1);
				if(pth.substr(0, v->mountpoint->size()) == *v->mountpoint)
				{
					fs = v;
					break;
				}
			}

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
			else return nullptr;
		}

		size_t Read(IOContext* ioctx, vnode* node, void* buf, off_t off, size_t len)
		{
			assert(ioctx);
			assert(node);
			assert(node->info);
			assert(node->refcount > 0);
			assert(node->info->driver);

			auto fs = node->info->driver;
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
	}


	// namespace VFS ends here
	using namespace VFS;
	fd_t OpenFile(const char* path, int flags)
	{
		assert(path);
		auto ctx = getctx();

			// MemoryManager::KernelHeap::Print();
			// UHALT();

		auto fe = VFS::OpenFile(ctx, path, flags);
		return fe ? fe->fd : 0;
	}

	size_t Read(fd_t fd, void* buf, off_t off, size_t len)
	{
		auto ctx = getctx();
		auto node = VFS::NodeFromFD(ctx, fd);
		if(node == nullptr)
			return 0;

		return VFS::Read(ctx, node, buf, off, len);
	}

	size_t Write(fd_t fd, void* buf, off_t off, size_t len)
	{
		auto ctx = getctx();

		auto node = VFS::NodeFromFD(ctx, fd);
		if(node == nullptr)
			return 0;

		return VFS::Write(ctx, node, buf, off, len);
	}
}
}
}
