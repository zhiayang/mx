// open.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "../../include/fcntl.h"
#include <sys/syscall.h>

extern "C" int open(const char* path, int flags)
{
	// TODO: flags ignored for now
	return (int) Library::SystemCall::Open(path, flags);
}
