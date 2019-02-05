// IOPort.hpp
// Copyright (c) 2013 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#pragma once
#include <stdint.h>

namespace Kernel {
namespace HardwareAbstraction {
namespace Devices
{
	namespace IOPort
	{
		uint8_t ReadByte(uint16_t Port);
		uint16_t Read16(uint16_t Port);
		uint32_t Read32(uint16_t Port);

		void WriteByte(uint16_t Port, uint8_t Value);
		void Write16(uint16_t Port, uint16_t Value);
		void Write32(uint16_t Port, uint32_t Value);

		uint64_t ReadMSR(uint32_t msr);
		void WriteMSR(uint32_t msr, uint64_t val);
	}
}
}
}
