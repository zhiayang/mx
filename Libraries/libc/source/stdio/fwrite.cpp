// fwrite.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "../../include/unistd.h"

extern "C" size_t fwrite(const void* ptr, size_t size, size_t count, FILE* stream)
{
	write((int) stream->fd, ptr, count * size);
	return count;
}
