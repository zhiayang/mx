// itoa.cpp
// Copyright (c) 2014 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "../../include/stdio.h"
#include "../../include/defs/_printf.h"

extern "C" char* itoa(int num, char* dest, int base)
{
	convert_integer(dest, num, base, "0123456789abcdefghijklmnopqrstuvwxyz");
	return dest;
}

extern "C" char* ltoa(long num, char* dest, int base)
{
	convert_integer(dest, num, base, "0123456789abcdefghijklmnopqrstuvwxyz");
	return dest;
}

extern "C" char* lltoa(long long num, char* dest, int base)
{
	convert_integer(dest, num, base, "0123456789abcdefghijklmnopqrstuvwxyz");
	return dest;
}

