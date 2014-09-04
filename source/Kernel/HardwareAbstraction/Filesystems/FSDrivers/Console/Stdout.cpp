// Stdin.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#include <Kernel.hpp>
#include <StandardIO.hpp>
#include <HardwareAbstraction/Filesystems.hpp>

namespace Kernel {
namespace HardwareAbstraction {
namespace Filesystems
{
	/*
		class FSDriverConsole : public FSDriver
		{
			public:
				FSDriverConsole();
				~FSDriverConsole() override;
				bool Traverse(VFS::vnode* node, const char* path, char** symlink) override;
				size_t Read(VFS::vnode* node, void* buf, off_t offset, size_t length) override;
				size_t Write(VFS::vnode* node, const void* buf, off_t offset, size_t length) override;
				void Stat(VFS::vnode* node, struct stat* stat) override;

				rde::vector<VFS::vnode*>* ReadDir(VFS::vnode* node) override;
		};
	*/

	// initialise with null partition, because we don't have one.
	FSDriverStdout::FSDriverStdout() : FSDriver(nullptr, FSDriverType::Virtual)
	{
		// nothing to do here.
	}

	FSDriverStdout::~FSDriverStdout()
	{
		// same here
	}

	bool FSDriverStdout::Create(VFS::vnode*, const char*, uint64_t, uint64_t)
	{
		return false;
	}
	bool FSDriverStdout::Delete(VFS::vnode*, const char*)
	{
		return false;
	}

	bool FSDriverStdout::Traverse(VFS::vnode* node, const char* path, char** symlink)
	{
		(void) node;
		(void) path;
		(void) symlink;

		return true;
	}

	size_t FSDriverStdout::Read(VFS::vnode* node, void* buf, off_t offset, size_t length)
	{
		(void) node;
		(void) buf;
		(void) offset;
		(void) length;
		return 0;
	}

	size_t FSDriverStdout::Write(VFS::vnode* node, const void* buf, off_t offset, size_t length)
	{
		(void) node;
		(void) buf;
		(void) offset;
		(void) length;

		assert(buf);
		// offset is ignored.
		if(length == 0)
			return 0;

		Library::StandardIO::PrintString((const char*) buf, length);
		return length;
	}

	void FSDriverStdout::Stat(VFS::vnode* node, struct stat* st)
	{
		(void) node;
		(void) st;
	}

	rde::vector<VFS::vnode*>* FSDriverStdout::ReadDir(VFS::vnode* node)
	{
		(void) node;
		return nullptr;
	}
}
}
}
