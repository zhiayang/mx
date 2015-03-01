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

	void SocketVFS::Bind(Filesystems::VFS::vnode* node, const char* _path)
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

		// (*ipcSocketMap)[path] = this;
	}

	void SocketVFS::Connect(Filesystems::VFS::vnode* node, const char* _path)
	{
		if(!ipcSocketMap) ipcSocketMap = new rde::hash_map<rde::string, Socket*>();
		rde::string path(_path);

		// connect to the socket.
	}
}






namespace Filesystems
{

}
}
}



















