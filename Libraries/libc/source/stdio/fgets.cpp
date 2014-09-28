// fgets.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "../../include/stdio.h"
#include "../../include/unistd.h"

extern "C" char* fgets(char* str, int num, FILE* stream)
{
	size_t read = fread(str, 1, num, stream);
	if(read < (size_t) num)
		stream->iseof = -1;

	if(read == 0)
		return NULL;

	return str;
}
