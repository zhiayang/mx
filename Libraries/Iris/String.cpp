// userspace/String.cpp
// Copyright (c) 2013 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "HeaderFiles/String.hpp"
#include "HeaderFiles/StandardIO.hpp"
#include "HeaderFiles/Memory.hpp"
#include <string.h>
#include <assert.h>

namespace String
{
	uint64_t Length(const char* str)
	{
		paranoid_assert(str != 0);

		uint64_t len = 0;
		const char* endPtr = str;
		asm("repne scasb" : "+D"(endPtr) : "a"(0), "c"(~0) : "cc");
		len = (endPtr - str) - 1;
		return len;
	}

	char* Copy(char* destination, const char* source)
	{
		paranoid_assert(destination != 0);
		paranoid_assert(source != 0);

		return (char*) Memory::Copy(destination, source, String::Length(source) + 1);
	}

	char* CopyLength(char* destination, const char* source, size_t len)
	{
		// todo: apparently the rest of DEST is supposed to be filled with NULLS???
		// conform to spec.

		// but nobody uses this.

		paranoid_assert(destination != 0);
		paranoid_assert(source != 0);

		if(len == 0) return destination;

		size_t slen = String::Length(source);
		char* ret = (char*) Memory::Copy(destination, source, slen > len ? len : slen);

		ret[(slen > len ? len : slen) - 1] = 0;

		return ret;
	}


	int Compare(const char* str1, const char* str2)
	{
		paranoid_assert(str1 != 0);
		paranoid_assert(str2 != 0);

		size_t len1 = String::Length(str1);
		size_t len2 = String::Length(str2);

		int cmpResult = Memory::Compare(str1, str2, (len1 < len2) ? len1 : len2);
		if(cmpResult != 0)
			return cmpResult;

		if(len1 > len2)
			return 1;

		else if(len1 < len2)
			return -1;

		return 0;
	}

	char* TrimWhitespace(char *str)
	{
		paranoid_assert(str != 0);

		char* end = 0;

		// Trim leading space
		while(*str == ' ')
			str++;

		if(*str == 0)  // All spaces?
			return str;

		// Trim trailing space
		end = str + String::Length(str) - 1;
		while(end > str && *end == ' ')
			end--;

		// Write new null terminator
		*(end + 1) = 0;

		return str;
	}



}

extern "C" uint64_t strlen(const char* str)
{
	return String::Length(str);
}

extern "C" char* strcpy(char* destination, const char* source)
{
	return String::Copy(destination, source);
}
extern "C" char* strncpy(char* destination, const char* source, size_t len)
{
	return String::CopyLength(destination, source, len);
}

extern "C" int strcmp(const char* str1, const char* str2)
{
	return String::Compare(str1, str2);
}

extern "C" int strncmp(const char* s1, const char* s2, size_t n)
{
	return Memory::Compare(s1, s2, n);
}

extern "C" char* strcat(char* dest, const char* src)
{
	strcpy(dest + strlen(dest), src);
	return dest;
}







