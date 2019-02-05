// PIT.cpp
// Copyright (c) 2013 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <HardwareAbstraction/Devices/IOPort.hpp>

using namespace Kernel::HardwareAbstraction::Devices;

namespace Kernel {
namespace HardwareAbstraction {
namespace Devices {
namespace PIT
{
	void SetTimerHertz(uint16_t hz)
	{
		uint16_t divisor = (uint16_t)((uint32_t)1193180 / hz);		// Calculate our divisor
		IOPort::WriteByte(0x43, 0x36);					// Set our command byte 0x36
		IOPort::WriteByte(0x40, divisor & 0xFF);			// Set low byte of divisor
		IOPort::WriteByte(0x40, (divisor >> 8) & 0xFF);			// Set high byte of divisor
	}

	void Initialise()
	{
		SetTimerHertz(GlobalTickRate);
	}
}
}
}
}
