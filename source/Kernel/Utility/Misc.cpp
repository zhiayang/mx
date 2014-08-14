// Misc.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <HardwareAbstraction/Devices/SerialPort.hpp>
#include <StandardIO.hpp>
#include <string>

using namespace Library::StandardIO;
using namespace Library;

namespace Kernel
{
	namespace Utilities
	{
		void DumpBytes(uint64_t address, uint64_t length)
		{
			Log("Dumping %d bytes at address %x:", length, address);
			std::string* str = new std::string();
			for(uint64_t i = 0; i < length; i++)
			{
				if(!(i % 16))
					HardwareAbstraction::Devices::SerialPort::WriteString("\n");
				PrintToString(str, "%#02x ", *((uint8_t*)(address + i)));
				HardwareAbstraction::Devices::SerialPort::WriteString(str->c_str());
				str->clear();

			}
			Log("Dump complete");
		}
	}
}
