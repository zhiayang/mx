// fread.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "../../include/unistd.h"

extern "C" size_t fread(void* ptr, size_t size, size_t count, FILE* stream)
{
	read((int) stream->fd, ptr, size * count);
	return count;
}
