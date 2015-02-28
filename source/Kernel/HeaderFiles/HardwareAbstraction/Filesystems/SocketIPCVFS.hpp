// SocketIPCVFS.hpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "FSUtil.hpp"

namespace Kernel
{
	namespace HardwareAbstraction
	{
		namespace Filesystems
		{
			class FSDriverSocketIPC : public FSDriver
			{
				public:
					FSDriverSocketIPC();
					~FSDriverSocketIPC() override;
					bool Create(VFS::vnode* node, const char* path, uint64_t flags, uint64_t perms) override;
					bool Delete(VFS::vnode* node, const char* path) override;
					bool Traverse(VFS::vnode* node, const char* path, char** symlink) override;
					size_t Read(VFS::vnode* node, void* buf, off_t offset, size_t length) override;
					size_t Write(VFS::vnode* node, const void* buf, off_t offset, size_t length) override;
					void Stat(VFS::vnode* node, struct stat* stat, bool statlink) override;
					void Flush(VFS::vnode* node) override;

					rde::vector<VFS::vnode*> ReadDir(VFS::vnode* node) override;
			};
		}
	}
}




