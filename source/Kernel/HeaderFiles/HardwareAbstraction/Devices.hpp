// Devices.hpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#pragma once
#include <stdint.h>
#include <sys/types.h>

#include "Devices/StorageDevice.hpp"
#include "Devices/PCI.hpp"
#include "Devices/Keyboard.hpp"
#include "Devices/PS2.hpp"
#include "Devices/RTC.hpp"
#include "Devices/SerialPort.hpp"
#include "Devices/IOPort.hpp"
#include "Devices/PIT.hpp"
#include "Devices/NIC.hpp"

namespace Kernel {
namespace HardwareAbstraction {
namespace Devices
{
	class Device
	{
		protected:
			dev_t devid;
	};

	dev_t GetDevID();
	void FreeDevID(dev_t id);
}
}
}
