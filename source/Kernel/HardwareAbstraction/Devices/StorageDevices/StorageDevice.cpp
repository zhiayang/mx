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


	static rde::vector<StorageDevice*>* storageDevices;
	void AddDevice(StorageDevice* dev)
	{
		if(!storageDevices)	 storageDevices = new rde::vector<StorageDevice*>();
		if(storageDevices->contains(dev))
		{
			Log(1, "Ignoring duplicate device");
			return;
		}

		dev->diskid = storageDevices->size();
		storageDevices->push_back(dev);
	}
}
}
}
}












