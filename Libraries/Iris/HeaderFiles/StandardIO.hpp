// userspace/StandardIO.hpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#pragma once

#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>

namespace Library
{
	namespace StandardIO
	{
		uint64_t PrintString(const char* string, size_t length = (size_t) -1, void (*pf)(uint8_t) = 0);
		void PrintGUID(uint64_t High64, uint64_t Low64);


		void PrintFormatted(void (*pf)(uint8_t), const char* str, ...);
		void PrintFormatted(const char* string, ...);

		void PrintFormatted(void (*pf)(uint8_t), const char* str, va_list);
		void PrintFormatted(const char* string, va_list);

	}
}



