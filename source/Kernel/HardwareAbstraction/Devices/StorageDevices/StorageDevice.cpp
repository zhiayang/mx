// StorageDevice.cpp
// Copyright (c) 2014 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <HardwareAbstraction/Devices/StorageDevice.hpp>


namespace Kernel {
namespace HardwareAbstraction {
namespace Devices
{
	IODevice::~IODevice()
	{
	}

	namespace Storage
	{
		StorageDevice::~StorageDevice()
		{
		}

		IOResult StorageDevice::Read(uint64_t, uint64_t, uint64_t)
		{
			return IOResult();
		}

		IOResult StorageDevice::Write(uint64_t, uint64_t, uint64_t)
		{
			return IOResult();
		}


		static rde::vector<StorageDevice*>* storageDevices;
		void AddStorageDevice(StorageDevice* dev)
		{
			if(!storageDevices)	storageDevices = new rde::vector<StorageDevice*>();

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












