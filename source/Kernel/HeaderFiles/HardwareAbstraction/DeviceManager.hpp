// DeviceManager.hpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <stdint.h>
#include <sys/types.h>
#include <rdestl/vector.h>

#pragma once
namespace Kernel {
namespace HardwareAbstraction {
namespace Devices
{


	enum class DeviceType
	{
		// Timers
		HighPrecisionTimer,


		// Video
		FramebufferVideoCard,


		// Network card
		EthernetNIC,


		// Misc
		AdvancedPIC,
	};

	namespace DeviceManager
	{
		class Device
		{
			protected:
				dev_t devid;
		};

		dev_t AllocateDevID();
		void FreeDevID(dev_t id);

		void AddDevice(Device* dev, DeviceType type);
		rde::vector<Device*>* GetDevices(DeviceType type);
		Device* GetDevice(DeviceType type);
	}
}
}
}
