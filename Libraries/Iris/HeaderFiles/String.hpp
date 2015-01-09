// String.hpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#pragma once
#include <stdint.h>

namespace String
{
	uint64_t Length(const char* str);
	char* Copy(char* destination, const char* source);
	int Compare(const char* str1, const char* str2, unsigned long length = 0);
	char* TrimWhitespace(char *str);
}
