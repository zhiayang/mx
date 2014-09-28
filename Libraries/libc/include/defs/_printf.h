// _printf.h
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "../stdint.h"
#include <stddef.h>
#include <stdarg.h>

#ifndef __printf_h
#define __printf_h

size_t convert_integer(char* destination, uint64_t value, uint64_t base, const char* digits);
extern "C" size_t vprintf_callback(size_t (*callback)(void*, const char*, size_t), void* user, const char* format, va_list parameters);


#endif
