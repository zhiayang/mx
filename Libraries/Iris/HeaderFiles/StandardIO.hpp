// userspace/StandardIO.hpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#pragma once

#include <stdint.h>
#include "Defines.hpp"
#include "SystemCall.hpp"
#include "String.hpp"

namespace Library
{
	namespace StandardIO
	{
		uint64_t PrintString(const char* string, int64_t length = -1, void (*pf)(uint8_t) = 0);
		void PrintGUID(uint64_t High64, uint64_t Low64);


		void PrintFormatted(void (*pf)(uint8_t), const char* str, ...);
		void PrintFormatted(const char* string, ...);

		void PrintFormatted(void (*pf)(uint8_t), const char* str, va_list);
		void PrintFormatted(const char* string, va_list);

		void PrintToString(Library::string*, const char* str, ...);
	}
}

void operator delete(void* p);
void operator delete[](void* p);
void* operator new(unsigned long size);
void* operator new[](unsigned long size);
void* operator new(unsigned long, void* addr);


