// DeviceManager.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <HardwareAbstraction/DeviceManager.hpp>

namespace Kernel {
namespace HardwareAbstraction {
namespace Devices {
namespace DeviceManager
{
	static dev_t curid = 0;
	static rde::hash_map<DeviceType, rde::vector<Device*>>* deviceMap;

	dev_t AllocateDevID()
	{
		// skips 0, but who gives a shit
		curid++;
		return curid;
	}

	void FreeDevID(dev_t id)
	{
		(void) id;
	}

	// TODO: make PCI enumeration actually fill these in, right now it's pretty useless.
	void AddDevice(Device* dev, DeviceType type)
	{
		if(!deviceMap)
			deviceMap = new rde::hash_map<DeviceType, rde::vector<Device*>>();

		(*deviceMap)[type].push_back(dev);
	}

	rde::vector<Device*>* GetDevices(DeviceType type)
	{
		assert(deviceMap);
		rde::vector<Device*>* ret = new rde::vector<Device*>();
		ret->copy((*deviceMap)[type]);

		return ret;
	}

	Device* GetDevice(DeviceType type)
	{
		assert(deviceMap);
		auto vec = (*deviceMap)[type];
		if(vec.size() > 0)
			return vec.front();

		return 0;
	}
}
}
}
}









