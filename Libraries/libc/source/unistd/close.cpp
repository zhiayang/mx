// close.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "../../include/unistd.h"
#include <sys/syscall.h>

extern "C" int close(int fd)
{
	Library::SystemCall::CloseAny(fd);
	return 0;
}
