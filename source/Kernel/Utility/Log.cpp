// Log.cpp
// Copyright (c) 2013 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#include <Kernel.hpp>
#include <StandardIO.hpp>
#include <Console.hpp>
#include <HardwareAbstraction/Devices/SerialPort.hpp>

using namespace Library;

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

		if(LOGSPAM)
			level = 3;

		switch(level)
		{
			case 0:
			default:
				StdIO::PrintFmt(pfso, "[INFO]: ");

				StdIO::PrintFmt(pfso, str, args);
				StdIO::PrintFmt(pfso, "\n");
				break;

			case 1:
				StdIO::PrintFmt(pfso, "[WARN]: ");

				StdIO::PrintFmt(pfso, str, args);
				StdIO::PrintFmt(pfso, "\n");
				break;

			case 2:
				StdIO::PrintFmt(pfsc, "[SEVR]: ");

				StdIO::PrintFmt(pfsc, str, args);
				StdIO::PrintFmt(pfsc, "\n");
				break;

			case 3:
				StdIO::PrintFmt(pfsc, "[CRIT]: ");

				StdIO::PrintFmt(pfsc, str, args);
				StdIO::PrintFmt(pfsc, "\n");
				break;
		}
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
