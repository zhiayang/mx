// RTC.hpp
// Copyright (c) 2014 - 2016, zhiayang@gmail.com
// Licensed under Creative Commons Attribution ShareAlike 3.0 Unported.


#include <stdint.h>
#pragma once

namespace Kernel {
namespace HardwareAbstraction {
namespace Devices
{
	namespace RTC
	{
		void Initialise(int8_t UTCOffset);
		void InitialiseTimer();
		void ReadTime();
		bool DidInitialise();
	}
}
}
}
