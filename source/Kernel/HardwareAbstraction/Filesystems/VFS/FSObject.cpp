// FSObject.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <String.hpp>

namespace Kernel {
namespace HardwareAbstraction {
namespace Filesystems {
namespace VFS
{
	FSObject::FSObject(FSObjectTypes type)
	{
		this->Type = type;
	}

	FSObject::FSObject(const FSObject& other)
	{
		// Log(3, "WHAT: %x", __builtin_return_address(0));
		// HALT("");
		this->Type = other.Type;
		this->rootfs = other.rootfs;
		this->_attributes = other._attributes;
	}

	FSObject::~FSObject()
	{
	}

	const char* FSObject::Name()
	{
		// Log(3, "WHAT: %x", __builtin_return_address(0));
		// HALT("");
		return (new Library::string(""))->CString();
	}

	const char* FSObject::Path()
	{
		// Log(3, "WHAT: %x", __builtin_return_address(0));
		// HALT("");
		return (new Library::string(""))->CString();
	}

	FSObject* FSObject::Parent()
	{
		// Log(3, "WHAT: %x", __builtin_return_address(0));
		// HALT("");
		return nullptr;
	}

	Filesystem* FSObject::RootFS()
	{
		// Log(3, "WHAT: %x", __builtin_return_address(0));
		// HALT("");
		return nullptr;
	}

	uint8_t FSObject::Attributes()
	{
		// Log(3, "WHAT: %x", __builtin_return_address(0));
		// HALT("");
		return 0;
	}

	bool FSObject::Exists()
	{
		// Log(3, "WHAT: %x", __builtin_return_address(0));
		// HALT("");
		return false;
	}
}
}
}
}
