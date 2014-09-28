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
		this->_seekable = false;
	}

	FSDriverConsole::~FSDriverConsole()
	{
		// same here
	}

	bool FSDriverConsole::Create(VFS::vnode*, const char*, uint64_t, uint64_t)
	{
		return false;
	}
	bool FSDriverConsole::Delete(VFS::vnode*, const char*)
	{
		return false;
	}

	bool FSDriverConsole::Traverse(VFS::vnode*, const char*, char**)
	{
		return false;
	}

	size_t FSDriverConsole::Read(VFS::vnode*, void*, off_t, size_t)
	{
		return 0;
	}

	size_t FSDriverConsole::Write(VFS::vnode*, const void*, off_t, size_t)
	{
		return 0;
	}

	void FSDriverConsole::Stat(VFS::vnode*, struct stat*, bool)
	{
	}

	void FSDriverConsole::Flush(VFS::vnode*)
	{
	}

	Library::Vector<VFS::vnode*>* FSDriverConsole::ReadDir(VFS::vnode*)
	{
		return nullptr;
	}
}
}
}
