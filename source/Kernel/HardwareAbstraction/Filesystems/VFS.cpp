// VFS.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <string.h>
#include <Kernel.hpp>
#include <rdestl/hash_map.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <String.hpp>

#include <HardwareAbstraction/Network.hpp>

using namespace Kernel::HardwareAbstraction::Devices::Storage;
namespace Kernel {
namespace HardwareAbstraction {
namespace Filesystems
{
	namespace VFS
	{
		const char* FS_CONSOLE_MOUNTPOINT = "/dev/console";
		const char* FS_STDIN_MOUNTPOINT = "/dev/stdin";
		const char* FS_STDOUT_MOUNTPOINT = "/dev/stdout";
		const char* FS_STDERR_MOUNTPOINT = "/dev/stderr";
		const char* FS_SOCKET_MOUNTPOINT = "/dev/socketfs";

		static id_t curid = 0;
		static id_t curfeid = 0;
		static fd_t FirstFreeFD = 0;

		static rde::vector<Filesystem*> mountedfses;
		static rde::hash_map<id_t, vnode*> vnodepool;

		static FSDriver* driver_console = 0;
		static FSDriver* driver_stdin = 0;
		static FSDriver* driver_stdout = 0;
		static FSDriver* driver_stderr = 0;
		static FSDriver* driver_socketfs = 0;

		static Mutex mtx;

		static IOContext* getctx()
		{
			auto proc = Multitasking::GetCurrentProcess();
			assert(proc);
			// assert(proc->iocontext);

			return &proc->iocontext;
		}

		static Filesystem* getfs(rde::string& path)
		{
			for(auto v : mountedfses)
			{
				if(String::Compare(path.substr(0, __min(path.length(), v->mountpoint->length())).c_str(), v->mountpoint->c_str()) == 0)
					return v;
			}

			return nullptr;
		}

		Filesystem* GetFilesystemAtPath(const char* path)
		{
			rde::string p(path);
			return getfs(p);
		}



		void Initialise()
		{
			// mountedfses = new rde::vector<Filesystem*>();
			// vnodepool = new rde::hash_map<id_t, vnode*>();

			// mtx = new Mutex();
		}

		void InitIO()
		{
			driver_console = new FSDriverConsole();
			driver_stdin = new FSDriverStdin();
			driver_stdout = new FSDriverStdout();
			driver_stderr = new FSDriverStdlog();
			driver_socketfs = new Network::SocketVFS();

			Mount(nullptr, driver_console, FS_CONSOLE_MOUNTPOINT);
			Mount(nullptr, driver_stdin, FS_STDIN_MOUNTPOINT);
			Mount(nullptr, driver_stdout, FS_STDOUT_MOUNTPOINT);
			Mount(nullptr, driver_stdout, FS_STDERR_MOUNTPOINT);
			Mount(nullptr, driver_socketfs, FS_SOCKET_MOUNTPOINT);

			auto ctx = getctx();
			OpenFile(ctx, FS_STDIN_MOUNTPOINT, 0);
			OpenFile(ctx, FS_STDOUT_MOUNTPOINT, 0);
			OpenFile(ctx, FS_STDERR_MOUNTPOINT, 0);
			OpenFile(ctx, FS_CONSOLE_MOUNTPOINT, 0);
		}

		// this fetches from the pool. used mainly by fsdrivers to avoid creating duplicate vnodes.
		vnode* NodeFromID(id_t id)
		{
			auto v = vnodepool.find(id);
			if(v == vnodepool.end())
				return nullptr;

			return v->second;
		}

		fileentry* FileEntryFromFD(IOContext* ioctx, fd_t fd)
		{
			assert(ioctx);

			for(auto v : ioctx->fdarray.fds)
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
			Memory::Set(node, 0, sizeof(vnode));

			node->data = nullptr;
			node->info = new fsref;
			node->refcount = 1;
			node->type = VNodeType::None;
			node->attrib = 0;

			Memory::Set(node->info, 0, sizeof(fsref));
			node->info->data = nullptr;
			node->info->driver = fs;
			node->info->id = fs->GetID();

			// add the node to the pool.
			node->id = curid++;
			vnodepool[node->id] = node;
			return node;
		}

		void DeleteNode(vnode* node)
		{
			(void) node;
			delete node;
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

			mountedfses.push_back(_fs);
		}

		void Unmount(const char* path)
		{
			(void) path;
		}

		fileentry* Open(IOContext* ioctx, vnode* node, int flags)
		{
			assert(ioctx);
			assert(node);

			auto fe		= new fileentry;
			fe->node	= node;
			fe->offset	= 0;
			fe->flags	= (uint64_t) flags;
			fe->fd		= FirstFreeFD + (fd_t) ioctx->fdarray.fds.size();
			fe->id		= curfeid++;

			ioctx->fdarray.fds.push_back(fe);

			return fe;
		}

		err_t Close(IOContext* ioctx, fileentry* fe)
		{
			assert(ioctx);
			assert(fe);
			assert(fe->node);
			assert(fe->node->info);
			assert(fe->node->info->driver);

			fe->node->info->driver->Close(fe->node);

			ioctx->fdarray.fds.remove(fe);
			Dereference(fe->node);
			delete fe;

			return 0;
		}

		void CloseAll(IOContext* ioctx)
		{
			assert(ioctx);

			for(fileentry* fe : ioctx->fdarray.fds)
			{
				Dereference(fe->node);
				delete fe;
			}

			ioctx->fdarray.fds.clear();
		}

		fileentry* OpenFile(IOContext* ioctx, const char* path, int flags)
		{
			assert(ioctx);
			assert(path);

			rde::string pth = rde::string(path);
			Filesystem* fs = getfs(pth);

			if(fs == nullptr || fs->ismounted == false)
			{
				Log(3, "filesystem not mounted");
				return nullptr;
			}

			auto node = VFS::CreateNode(fs->driver);
			assert(node);
			node->type = VNodeType::File;

			// this ought to fill in the information in node.
			assert(fs);
			assert(fs->driver);

			bool res = fs->driver->Traverse(node, path, nullptr);
			if(res || flags & O_CREATE)
			{
				// if O_CREAT, force the issue.
				if(!res && (flags & O_CREATE))
					fs->driver->Create(node, path, (uint64_t) flags, 0);

				auto ret = VFS::Open(ioctx, node, flags);
				return ret;
			}
			else
			{
				Log("returning nullptr: %s", path);
				return nullptr;
			}
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

		err_t Stat(IOContext* ioctx, vnode* node, struct stat* st, bool statlink)
		{
			assert(ioctx);
			assert(node);
			assert(node->info);
			assert(node->refcount > 0);
			assert(node->info->driver);

			auto fs = node->info->driver;
			fs->Stat(node, st, statlink);

			return 0;
		}

		err_t Seek(fileentry* fe, off_t offset, int origin)
		{
			assert(fe);
			assert(fe->node);

			if(fe->node->info->driver->Seekable())
			{
				if(origin == SEEK_SET)
					fe->offset = 0;

				fe->offset += offset;
				return 0;
			}
			else
			{
				Multitasking::SetThreadErrno(EBADF);
				return -1;
			}
		}

		err_t Flush(IOContext* ioctx, vnode* node)
		{
			assert(ioctx);
			assert(node);
			assert(node->info);
			assert(node->refcount > 0);
			assert(node->info->driver);

			auto fs = node->info->driver;
			fs->Flush(node);
			return 0;
		}

		fileentry* Duplicate(IOContext* ctx, fileentry* old)
		{
			assert(ctx);

			fileentry* fe	= new fileentry;
			fe->node	= old->node;
			fe->offset	= 0;
			fe->flags	= old->flags;
			fe->fd		= FirstFreeFD + (fd_t) ctx->fdarray.fds.size();
			fe->id		= curfeid++;

			ctx->fdarray.fds.push_back(fe);
			return fe;
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

	err_t Close(fd_t fd)
	{
		auto ctx = getctx();
		if(fd < 0)
			return -1;

		auto fe = VFS::FileEntryFromFD(ctx, fd);
		if(fe == nullptr)
			return 0;

		return VFS::Close(ctx, fe);
	}

	void CloseAll(Multitasking::Process* p)
	{
		auto ctx = &p->iocontext;
		assert(ctx);

		VFS::CloseAll(ctx);
	}

	size_t Read(fd_t fd, void* buf, size_t len)
	{
		if(len == 0)
			return 0;

		auto ctx = getctx();
		auto fe = VFS::FileEntryFromFD(ctx, fd);
		if(fe == nullptr)
			return (size_t) -1;

		assert(fe->node);
		auto read = VFS::Read(ctx, fe->node, buf, fe->offset, len);

		if(read > 0)
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

	err_t Stat(fd_t fd, struct stat* out, bool statlink)
	{
		auto ctx = getctx();
		if(fd < 0)
			return -1;

		auto node = VFS::NodeFromFD(ctx, fd);
		if(node == nullptr)
			return -1;

		VFS::Stat(ctx, node, out, statlink);
		return 0;
	}

	err_t Seek(fd_t fd, off_t offset, int origin)
	{
		auto ctx = getctx();
		if(fd < 0)
		{
			// todo: set errno
			Multitasking::SetThreadErrno(EBADF);
			return -1;
		}

		auto fe = VFS::FileEntryFromFD(ctx, fd);
		if(fe == nullptr)
		{
			Multitasking::SetThreadErrno(EBADF);
			return -1;
		}

		return VFS::Seek(fe, offset, origin);
	}

	err_t Flush(fd_t fd)
	{
		auto ctx = getctx();
		if(fd < 0)
		{
			// todo: set errno
			Multitasking::SetThreadErrno(EBADF);
			return -1;
		}

		auto node = VFS::NodeFromFD(ctx, fd);
		if(node == nullptr)
		{
			Multitasking::SetThreadErrno(EBADF);
			return -1;
		}

		return VFS::Flush(ctx, node);
	}

	uint64_t GetSeekPos(fd_t fd)
	{
		auto ctx = getctx();
		if(fd < 0)
		{
			// todo: set errno
			Multitasking::SetThreadErrno(EBADF);
			return (uint64_t) -1;
		}

		auto fe = VFS::FileEntryFromFD(ctx, fd);
		if(fe == nullptr)
		{
			Multitasking::SetThreadErrno(EBADF);
			return (uint64_t) -1;
		}

		return fe->offset;
	}

	fd_t Duplicate(fd_t old)
	{
		auto ctx = getctx();
		if(old < 0)
			// todo: set errno
			return -1;

		auto fe = VFS::FileEntryFromFD(ctx, old);
		if(fe == nullptr)
			return -1;

		return VFS::Duplicate(ctx, fe)->fd;
	}
}
}
}




