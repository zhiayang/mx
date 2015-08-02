// userspace/StandardIO.hpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#pragma once

#include <ctype.h>
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>

#include "../../acess2_stl/string.h"
#include "Utility.hpp"

#include "VariadicPrintf.hpp"

namespace StdIO
{
	void PrintFmt(void (*pf)(uint8_t), const char* str, ...);
	void PrintFmt(const char* string, ...);

	void PrintFmt(void (*pf)(uint8_t), const char* str, va_list);
	void PrintFmt(const char* string, va_list);

	size_t PrintStr(const char* s, size_t len);
	size_t PrintStr(uint8_t* s, size_t len);
}

int _printf(const char* format, ...);
int _vprintf(const char* format, va_list list);
int _sprintf(char* str, const char* format, ...);
int _vsprintf(char* str, const char* format, va_list ap);
int _snprintf(char* str, size_t size, const char* format, ...);
int _vsnprintf(char* str, size_t size, const char* format, va_list list);
size_t _printf_callback(size_t (*callback)(void*, const char*, size_t), void* user, const char* format, ...);
size_t _vprintf_callback(size_t (*callback)(void*, const char*, size_t), void* user, const char* format, va_list parameters);








