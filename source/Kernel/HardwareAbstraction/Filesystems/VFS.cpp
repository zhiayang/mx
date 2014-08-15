// VFS.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <string>
#include <string.h>
#include <Kernel.hpp>

using namespace Kernel::HardwareAbstraction::Devices::Storage;
namespace Kernel {
namespace HardwareAbstraction {
namespace Filesystems
{
	namespace VFS
	{
		// long FDFromNode(IOContext* ioctx, vnode* node)
		// {

		// }

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

		static std::vector<Filesystem*>* mountedfses;

		vnode* NodeFromFD(IOContext* ioctx, fd_t fd)
		{
			assert(ioctx);
			assert(ioctx->fdarray);
			assert(ioctx->fdarray->fds);

			for(auto v : *ioctx->fdarray->fds)
			{
				if(v.fd == fd)
					return v.node;
			}

			return nullptr;
		}

		vnode* CreateNode(IOContext* ioctx, FSDriver* fs)
		{
			assert(ioctx);

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

			return node;
		}

		vnode* DuplicateNode(IOContext* ioctx, vnode* orig)
		{
			assert(ioctx);
			assert(orig);
			assert(orig->info);

			vnode* node = new vnode;
			node->data = nullptr;
			node->info = new fsref;
			node->refcount = 1;
			node->type = orig->type;
			node->attrib = orig->attrib;

			node->info->data = nullptr;
			node->info->driver = orig->info->driver;
			node->info->id = orig->info->driver->GetID();

			return node;
		}

		void DeleteNode(IOContext* ioctx, vnode* node)
		{
			(void) ioctx;
			(void) node;

		}

		vnode* Reference(IOContext* ioctx, vnode* node)
		{
			assert(ioctx);
			assert(node);
			node->refcount++;

			return node;
		}

		vnode* Dereference(IOContext* ioctx, vnode* node)
		{
			assert(ioctx);
			assert(node);

			node->refcount--;
			if(node->refcount == 0)
			{
				DeleteNode(ioctx, node);
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

			if(!mountedfses)
				mountedfses = new std::vector<Filesystem*>();

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
			fe->fd		= ioctx->fdarray->fds->size();

			ioctx->fdarray->fds->push_back(*fe);
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

			auto node = VFS::CreateNode(ioctx, fs->driver);
			assert(node);
			node->type = VNodeType::File;

			// this ought to fill in the information in node.
			assert(fs);
			assert(fs->driver);

			bool res = fs->driver->Traverse(node, path, nullptr);
			Log(3, "created node");
			UHALT();

			return res ? VFS::Open(ioctx, node, flags) : nullptr;
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
