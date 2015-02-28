// SocketIPC.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <HardwareAbstraction/Filesystems.hpp>
#include <mqueue.h>
#include <String.hpp>

#include <defs/_errnos.h>
#include <HardwareAbstraction/Network.hpp>

namespace Kernel {
namespace HardwareAbstraction {
namespace Network
{
	// note: implemented here since they do similar things
	static rde::hash_map<rde::string, Socket*>* ipcSocketMap = 0;

	void Socket::Bind(Filesystems::VFS::vnode* node, const char* _path)
	{
		// bind/connect is what actually opens the file on the VFS layer, for IPC sockets
		if(!ipcSocketMap) ipcSocketMap = new rde::hash_map<rde::string, Socket*>();
		rde::string path(_path);

		if(ipcSocketMap->find(path) != ipcSocketMap->end())
		{
			Log(1, "Socket with path '%s' already exists, aborting...", _path);
			Multitasking::SetThreadErrno(EADDRINUSE);
			return;
		}

		(*ipcSocketMap)[path] = this;
	}

	void Socket::Connect(Filesystems::VFS::vnode* node, const char* _path)
	{
		if(!ipcSocketMap) ipcSocketMap = new rde::hash_map<rde::string, Socket*>();
		rde::string path(_path);

		// connect to the socket.
	}
}






namespace Filesystems
{
	FSDriverSocketIPC::FSDriverSocketIPC() : FSDriver(nullptr, FSDriverType::Virtual)
	{

	}

	FSDriverSocketIPC::~FSDriverSocketIPC()
	{

	}

	bool FSDriverSocketIPC::Create(VFS::vnode* node, const char* path, uint64_t flags, uint64_t perms)
	{
		return false;
	}

	bool FSDriverSocketIPC::Delete(VFS::vnode* node, const char* path)
	{
		return false;
	}

	bool FSDriverSocketIPC::Traverse(VFS::vnode* node, const char* path, char** symlink)
	{
		return false;
	}

	size_t FSDriverSocketIPC::Read(VFS::vnode* node, void* buf, off_t offset, size_t length)
	{
		return 0;
	}

	size_t FSDriverSocketIPC::Write(VFS::vnode* node, const void* buf, off_t offset, size_t length)
	{
		return 0;
	}

	void FSDriverSocketIPC::Stat(VFS::vnode* node, struct stat* stat, bool statlink)
	{

	}

	void FSDriverSocketIPC::Flush(VFS::vnode* node)
	{

	}


	rde::vector<VFS::vnode*> FSDriverSocketIPC::ReadDir(VFS::vnode* node)
	{
		return rde::vector<VFS::vnode*>();
	}

}
}
}



















