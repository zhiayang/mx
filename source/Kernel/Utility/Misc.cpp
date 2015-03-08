// Misc.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <HardwareAbstraction/Devices/SerialPort.hpp>
#include <StandardIO.hpp>

using namespace Library::StandardIO;
using namespace Library;

namespace Kernel
{
	namespace Utilities
	{
		static rde::string* str;
		void appendToString(uint8_t ch)
		{
			assert(str);
			str->append(ch);
		}

		void DumpBytes(uint64_t address, uint64_t length)
		{
			Log("Dumping %d bytes at address %x:", length, address);
			str = new rde::string();
			for(uint64_t i = 0; i < length; i++)
			{
				if(!(i % 16))
					HardwareAbstraction::Devices::SerialPort::WriteString("\n");

				PrintFormatted(appendToString, "%#02x ", *((uint8_t*)(address + i)));

				HardwareAbstraction::Devices::SerialPort::WriteString(str->c_str());
				str->clear();
			}

			delete str;
			Log("Dump complete");
		}

		void StackDump(uint64_t* ptr, int num, bool fromTop)
		{
			Log("Stack dump of %x%s:", ptr, fromTop ? " (reverse)" : "");

			for(int i = 0; i < num; i++)
				Log("%d: %x", i, fromTop ? *--ptr : *ptr++);
		}
	}
}
