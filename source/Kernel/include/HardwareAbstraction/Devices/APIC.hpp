// APIC.hpp
// Copyright (c) 2014 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#pragma once
#include <Kernel.hpp>
#include <HardwareAbstraction/ACPI.hpp>
#include <HardwareAbstraction/DeviceManager.hpp>

namespace Kernel {
namespace HardwareAbstraction {
namespace Devices
{
	class APIC : public DeviceManager::Device
	{
		public:
			APIC(ACPI::APICTable* table);


		private:
	};
}
}
}
