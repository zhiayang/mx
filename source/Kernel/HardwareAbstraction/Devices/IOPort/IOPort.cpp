// IOPort.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>

namespace Kernel {
namespace HardwareAbstraction {
namespace Devices {
namespace IOPort
{
	uint8_t ReadByte(uint16_t Port)
	{
		uint8_t ret;
		asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(Port));
		return ret;
	}

	uint16_t Read16(uint16_t Port)
	{
		uint16_t ret;
		asm volatile("inw %1, %0" : "=a"(ret) : "Nd"(Port));
		return ret;
	}

	uint32_t Read32(uint16_t Port)
	{
		uint32_t ret;
		asm volatile("inl %1, %0" : "=a"(ret) : "Nd"(Port));
		return ret;
	}

	void WriteByte(uint16_t Port, uint8_t Value)
	{
		asm volatile("outb %0, %1" :: "a"(Value), "Nd"(Port));
	}

	void Write16(uint16_t Port, uint16_t Value)
	{
		asm volatile("outw %0, %1" :: "a"(Value), "Nd"(Port));
	}

	void Write32(uint16_t Port, uint32_t Value)
	{
		asm volatile("outl %0, %1" :: "a"(Value), "Nd"(Port));
	}
}

}
}
}
