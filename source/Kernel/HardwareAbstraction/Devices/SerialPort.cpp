// SerialPort.c
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#include <Kernel.hpp>
#include <HardwareAbstraction/Devices/SerialPort.hpp>
#include <HardwareAbstraction/Devices/IOPort.hpp>
#include <StandardIO.hpp>
using namespace Kernel::HardwareAbstraction::Devices;



namespace Kernel {
namespace HardwareAbstraction {
namespace Devices {
namespace SerialPort
{
	#define Port		0x3F8		// Serial port 1

	// static bool Stage2Initialised = false;
	extern "C" void SerialPort_HandleIRQ4();


	void WriteString(const char* String)
	{
		for(uint64_t i = 0; String[i] != '\0'; i++)
		{
			WriteChar((uint8_t) String[i]);
		}
	}

	void Initialise()
	{
		IOPort::WriteByte(Port + 1, 0x00);    // Disable all interrupts
		IOPort::WriteByte(Port + 3, 0x80);    // Enable DLAB (set baud rate divisor)
		IOPort::WriteByte(Port + 0, 0x01);    // Set divisor to 1 (lo byte) max baud
		IOPort::WriteByte(Port + 1, 0x00);    //			(hi byte)
		IOPort::WriteByte(Port + 3, 0x03);    // 8 bits, no parity, one stop bit
		IOPort::WriteByte(Port + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
		IOPort::WriteByte(Port + 4, 0x0B);    // IRQs enabled, RTS/DSR set
		// IOPort::WriteByte(Port + 3, 0x00);    // Disable DLAB
	}

	void InitialiseStage2()
	{
		// Interrupts::SetGate(32 + 4, (uint64_t) SerialPort_HandleIRQ4, 0x8, 0xEE);
		// IOPort::WriteByte(Port + 1, 0x02);    // Enable buffer empty interrupt.

		// Stage2Initialised = true;
	}

	extern "C" void IRQ_BufferEmpty()
	{

	}

	void WriteChar(uint8_t Ch)
	{
		while(!(IOPort::ReadByte(Port + 5) & 0x20));
		IOPort::WriteByte(Port, Ch);
	}
}
}
}
}




