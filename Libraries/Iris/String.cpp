// userspace/String.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "HeaderFiles/String.hpp"
#include "HeaderFiles/StandardIO.hpp"
#include <string.h>

using namespace Library::String;
using namespace Library::StandardIO;



namespace Library
{
	namespace String
	{
		char* TrimWhitespace(char *str)
		{
			char *end;

			// Trim leading space
			while(*str == ' ')
				str++;

			if(*str == 0)  // All spaces?
				return str;

			// Trim trailing space
			end = str + strlen(str) - 1;
			while(end > str && *end == ' ')
				end--;

			// Write new null terminator
			*(end + 1) = 0;

			return str;
		}
	}
}
