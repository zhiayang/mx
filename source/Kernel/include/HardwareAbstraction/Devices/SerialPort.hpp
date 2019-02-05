// SerialPort.hpp
// Copyright (c) 2014 - 2016, zhiayang@gmail.com
// Licensed under Creative Commons Attribution ShareAlike 3.0 Unported.


#include <stdint.h>
#pragma once

namespace Kernel {
namespace HardwareAbstraction {
namespace Devices
{
	namespace SerialPort
	{
		void WriteString(const char* String);
		void Initialise();
		void WriteChar(uint8_t Ch);

		void InitialiseStage2();
	}
}
}
}
