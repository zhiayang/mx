// Misc.cpp
// Copyright (c) 2013 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <HardwareAbstraction/Devices/SerialPort.hpp>
#include <StandardIO.hpp>

using namespace Library;

namespace Kernel
{
	namespace Utilities
	{
		static rde::string* str;
		void appendToString(uint8_t ch)
		{
			assert(str);
			str->append((char) ch);
		}

		void DumpBytes(uint64_t address, uint64_t length)
		{
			Log("Dumping %d bytes at address %x:", length, address);
			str = new rde::string();
			for(uint64_t i = 0; i < length; i++)
			{
				// if((i % 16) == 0)
				// 	PrintFmt(appendToString, "\n%x:  ", address + i);

				if((i % 16) == 0)
					StdIO::PrintFmt(appendToString, "\n");


				StdIO::PrintFmt(appendToString, "%#02x ", *((uint8_t*)(address + i)));

				HardwareAbstraction::Devices::SerialPort::WriteString(str->c_str());
				str->clear();
			}

			delete str;
			Log("Dump complete");
		}

		static Mutex* dumping_mtx = 0;
		void StackDump(uint64_t* ptr, int num, bool fromTop)
		{
			if(!dumping_mtx) dumping_mtx = new Mutex();

			LOCK(dumping_mtx);

			Log("Stack dump of %x%s:", ptr, fromTop ? " (reverse)" : "");

			for(int i = 0; i < num; i++)
				Log("%d: %x", i, fromTop ? *--ptr : *ptr++);

			UNLOCK(dumping_mtx);
		}

		void GenerateStackTrace(uint64_t stack, int frames)
		{
			// assumptions, everywhere.
			uint64_t* rbp = (uint64_t*) stack;

			Log("Beginning stack trace: (rbp = %x)", rbp);

			for(int i = 0; i < frames && rbp != 0 && *rbp != 0; i++)
			{
				Log("(%2d/%x/%x): %x", i, rbp, *rbp, *(rbp + 1));
				rbp = (uint64_t*) *rbp;
			}
		}
	}
}
















