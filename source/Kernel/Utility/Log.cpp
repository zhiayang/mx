// Log.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#include <Kernel.hpp>
#include <StandardIO.hpp>
#include <Console.hpp>
#include <HardwareAbstraction/Devices/SerialPort.hpp>

using namespace Library;
using namespace Library::StandardIO;

namespace Kernel
{
	// 0 = Info
	// 1 = Warning
	// 2 = Severe
	// 3 = Critical

	void pfso(uint8_t c)
	{
		Kernel::HardwareAbstraction::Devices::SerialPort::WriteChar(c);
	}

	void pfsc(uint8_t c)
	{
		Kernel::HardwareAbstraction::Devices::SerialPort::WriteChar(c);
		Kernel::Console::PrintChar(c);
	}


	void Log(uint8_t level, const char* str, va_list args)
	{
		if(!ENABLELOGGING)
			return;

		// std::string tm;
		// // Time::GetHumanReadableTime(tm);

		// if(tm.length() > 0)
		// 	tm.insert(0, " - ");

		// LOCK(Mutexes::SerialLog);
		switch(level)
		{
			case 0:
			default:
				PrintFormatted(pfso, "[ INFO ]: ");

				PrintFormatted(pfso, str, args);
				PrintString("\n", -1, pfso);
				break;

			case 1:
				PrintFormatted(pfso, "[ WARN ]: ");

				PrintFormatted(pfso, str, args);
				PrintString("\n", -1, pfso);
				break;

			case 2:
				PrintFormatted(pfsc, "[ SEVR ]: ");

				PrintFormatted(pfsc, str, args);
				PrintString("\n", -1, pfsc);
				break;

			case 3:
				PrintFormatted(pfsc, "[ CRIT ]: ");

				PrintFormatted(pfsc, str, args);
				PrintString("\n", -1, pfsc);
				break;
		}

		// UNLOCK(Mutexes::SerialLog);
	}

	void Log(uint8_t level, const char* str, ...)
	{
		va_list args;
		va_start(args, str);
		Log(level, str, args);
		va_end(args);
	}

	void Log(const char* str, ...)
	{
		va_list args;
		va_start(args, str);
		Log(0, str, args);
		va_end(args);
	}
}
