// DeviceManager.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <HardwareAbstraction/Devices.hpp>
#include <HardwareAbstraction/DeviceManager.hpp>
#include <HardwareAbstraction/Devices/StorageDevice.hpp>

namespace Kernel {
namespace HardwareAbstraction {
namespace Devices {
namespace DeviceManager
{
	static dev_t curid = 0;
	static rde::hash_map<DeviceType, rde::vector<Device*>>* deviceMap;
	static rde::deque<DispatchableDevice*>* deviceJobDispatchQueue;
	static Mutex* queueMtx;

	void EnqueueDriverDispatchJob(DispatchableDevice* device)
	{
		deviceJobDispatchQueue->push_back(device);
	}

	void DispatcherThread()
	{
		while(true)
		{
			LOCK(queueMtx);
			size_t s = deviceJobDispatchQueue->size();
			for(size_t i = 0; i < s; i++)
			{
				deviceJobDispatchQueue->front()->HandleJobDispatch();
				deviceJobDispatchQueue->erase(deviceJobDispatchQueue->begin());
			}

			UNLOCK(queueMtx);
			YieldCPU();
		}
	}

	void Initialise()
	{
		deviceJobDispatchQueue = new rde::deque<DispatchableDevice*>();
		queueMtx = new Mutex();

		Multitasking::Thread* th = Multitasking::CreateKernelThread(DispatcherThread);
		Multitasking::AddToQueue(th);
	}


















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

	rde::vector<Device*> GetDevices(DeviceType type)
	{
		assert(deviceMap);
		return (*deviceMap)[type];
	}

	Device* GetDevice(DeviceType type)
	{
		assert(deviceMap);
		auto vec = (*deviceMap)[type];
		if(vec.size() > 0)
			return vec.front();

		return 0;
	}

	void SetActiveDevice(Device* dev, DeviceType type)
	{
		rde::vector<Device*>& vec = (*deviceMap)[type];

		if(vec.size() > 0 && vec.contains(dev))
		{
			vec.remove(dev);
			vec.insert(vec.begin(), dev);
		}
	}
}

DispatchableDevice::~DispatchableDevice() { }



}
}
}









