// Stdin.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#include <Kernel.hpp>
#include <Console.hpp>
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
	FSDriverStdout::FSDriverStdout() : FSDriver(nullptr, FSDriverType::Virtual)
	{
		// nothing to do here.
		this->_seekable = false;
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

	bool FSDriverStdout::Traverse(VFS::vnode*, const char*, char**)
	{
		return true;
	}

	size_t FSDriverStdout::Read(VFS::vnode*, void*, off_t, size_t)
	{
		return 0;
	}

	size_t FSDriverStdout::Write(VFS::vnode*, const void* buf, off_t, size_t length)
	{
		assert(buf);
		// offset is ignored.
		if(length == 0)
			return 0;

		// Library::StandardIO::PrintString((const char*) buf, length);
		TTY::WriteTTY(1, (uint8_t*) buf, length);
		return length;
	}

	void FSDriverStdout::Stat(VFS::vnode*, struct stat*, bool)
	{
	}

	void FSDriverStdout::Close(VFS::vnode*)
	{
	}

	void FSDriverStdout::Flush(VFS::vnode*)
	{
		TTY::FlushTTY(1);
	}

	iris::vector<VFS::vnode*> FSDriverStdout::ReadDir(VFS::vnode*)
	{
		return iris::vector<VFS::vnode*>();
	}
}
}
}
