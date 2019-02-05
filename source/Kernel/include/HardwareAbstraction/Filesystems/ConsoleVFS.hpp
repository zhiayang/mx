// ConsoleVFS.hpp
// Copyright (c) 2014 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "FSUtil.hpp"

namespace Kernel
{
	namespace HardwareAbstraction
	{
		namespace Filesystems
		{
			class FSDriverConsole : public FSDriver
			{
				public:
					FSDriverConsole();
					virtual ~FSDriverConsole() override;
					virtual bool Create(VFS::vnode* node, const char* path, uint64_t flags, uint64_t perms) override;
					virtual bool Delete(VFS::vnode* node, const char* path) override;
					virtual bool Traverse(VFS::vnode* node, const char* path, char** symlink) override;
					virtual size_t Read(VFS::vnode* node, void* buf, off_t offset, size_t length) override;
					virtual size_t Write(VFS::vnode* node, const void* buf, off_t offset, size_t length) override;
					virtual void Stat(VFS::vnode* node, struct stat* stat, bool statlink) override;
					virtual void Flush(VFS::vnode* node) override;
					virtual void Close(VFS::vnode* node) override;

					virtual rde::vector<VFS::vnode*> ReadDir(VFS::vnode* node) override;
			};

			class FSDriverStdin : public FSDriver
			{
				public:
					FSDriverStdin();
					virtual ~FSDriverStdin() override;
					virtual bool Create(VFS::vnode* node, const char* path, uint64_t flags, uint64_t perms) override;
					virtual bool Delete(VFS::vnode* node, const char* path) override;
					virtual bool Traverse(VFS::vnode* node, const char* path, char** symlink) override;
					virtual size_t Read(VFS::vnode* node, void* buf, off_t offset, size_t length) override;
					virtual size_t Write(VFS::vnode* node, const void* buf, off_t offset, size_t length) override;
					virtual void Stat(VFS::vnode* node, struct stat* stat, bool statlink) override;
					virtual void Flush(VFS::vnode* node) override;
					virtual void Close(VFS::vnode* node) override;

					virtual rde::vector<VFS::vnode*> ReadDir(VFS::vnode* node) override;
			};

			class FSDriverStdout : public FSDriver
			{
				public:
					FSDriverStdout();
					virtual ~FSDriverStdout() override;
					virtual bool Create(VFS::vnode* node, const char* path, uint64_t flags, uint64_t perms) override;
					virtual bool Delete(VFS::vnode* node, const char* path) override;
					virtual bool Traverse(VFS::vnode* node, const char* path, char** symlink) override;
					virtual size_t Read(VFS::vnode* node, void* buf, off_t offset, size_t length) override;
					virtual size_t Write(VFS::vnode* node, const void* buf, off_t offset, size_t length) override;
					virtual void Stat(VFS::vnode* node, struct stat* stat, bool statlink) override;
					virtual void Flush(VFS::vnode* node) override;
					virtual void Close(VFS::vnode* node) override;

					virtual rde::vector<VFS::vnode*> ReadDir(VFS::vnode* node) override;
			};

			class FSDriverStdlog : public FSDriver
			{
				public:
					FSDriverStdlog();
					virtual ~FSDriverStdlog() override;
					virtual bool Create(VFS::vnode* node, const char* path, uint64_t flags, uint64_t perms) override;
					virtual bool Delete(VFS::vnode* node, const char* path) override;
					virtual bool Traverse(VFS::vnode* node, const char* path, char** symlink) override;
					virtual size_t Read(VFS::vnode* node, void* buf, off_t offset, size_t length) override;
					virtual size_t Write(VFS::vnode* node, const void* buf, off_t offset, size_t length) override;
					virtual void Stat(VFS::vnode* node, struct stat* stat, bool statlink) override;
					virtual void Flush(VFS::vnode* node) override;
					virtual void Close(VFS::vnode* node) override;

					virtual rde::vector<VFS::vnode*> ReadDir(VFS::vnode* node) override;
			};
		}
	}
}




