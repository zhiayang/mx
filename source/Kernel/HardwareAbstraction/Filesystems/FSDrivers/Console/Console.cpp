// Console.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
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

				Library::Vector<VFS::vnode*>* ReadDir(VFS::vnode* node) override;
		};
	*/

	// initialise with null partition, because we don't have one.
	FSDriverConsole::FSDriverConsole() : FSDriver(nullptr, FSDriverType::Virtual)
	{
		// nothing to do here.
	}

	FSDriverConsole::~FSDriverConsole()
	{
		// same here
	}

	bool FSDriverConsole::Traverse(VFS::vnode* node, const char* path, char** symlink)
	{
		(void) node;
		(void) path;
		(void) symlink;

		return false;
	}

	size_t FSDriverConsole::Read(VFS::vnode* node, void* buf, off_t offset, size_t length)
	{
		(void) node;
		(void) buf;
		(void) offset;
		(void) length;
		return 0;
	}

	size_t FSDriverConsole::Write(VFS::vnode* node, const void* buf, off_t offset, size_t length)
	{
		(void) node;
		(void) buf;
		(void) offset;
		(void) length;

		return 0;
	}

	void FSDriverConsole::Stat(VFS::vnode* node, struct stat* st)
	{
		(void) node;
		(void) st;
	}

	Library::Vector<VFS::vnode*>* FSDriverConsole::ReadDir(VFS::vnode* node)
	{
		(void) node;
		return nullptr;
	}
}
}
}
