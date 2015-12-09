// PIT.hpp
// Copyright (c) 2013 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#pragma once
#include <stdint.h>

namespace Kernel {
namespace HardwareAbstraction {
namespace Devices
{
	namespace PIT
	{
		void SetTimerHertz(uint16_t hz);
		void Initialise();
	}
}
}
}
