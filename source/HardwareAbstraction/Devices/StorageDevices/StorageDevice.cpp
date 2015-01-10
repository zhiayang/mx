// StorageDevice.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <HardwareAbstraction/Devices/StorageDevice.hpp>


namespace Kernel {
namespace HardwareAbstraction {
namespace Devices {
namespace Storage
{
	StorageDevice::~StorageDevice()
	{
	}

	void StorageDevice::Read(uint64_t position, uint64_t outbuf, uint64_t bytes)
	{
		UNUSED(position);
		UNUSED(outbuf);
		UNUSED(bytes);
	}

	void StorageDevice::Write(uint64_t position, uint64_t outbuf, uint64_t bytes)
	{
		UNUSED(position);
		UNUSED(outbuf);
		UNUSED(bytes);
	}
}
}
}
}
