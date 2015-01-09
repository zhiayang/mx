// APIC.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <HardwareAbstraction/ACPI.hpp>
#include <HardwareAbstraction/Devices/APIC.hpp>

namespace Kernel {
namespace HardwareAbstraction {
namespace Devices
{
}


namespace ACPI
{
	APICTable::APICTable(uint64_t address)
	{
		APICTable* apic = (APICTable*) address;

		uint32_t	physLocalControllerAddr;
		uint32_t	flags;
	}
}


}
}
