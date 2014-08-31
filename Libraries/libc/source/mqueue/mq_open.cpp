// mq_open.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "../../include/mqueue.h"
#include "../../include/fcntl.h"
#include "../../include/stdarg.h"
#include "../../include/stdlib.h"
#include "../../include/string.h"
#include <sys/syscall.h>

extern "C" mqd_t mq_open(const char* path, int flags, ...)
{
	mode_t mode = 0;
	(void) mode;
	if(flags & O_CREAT)
	{
		va_list ap;
		va_start(ap, flags);
		mode = va_arg(ap, mode_t);
		va_end(ap);
	}

	// prepend the path with '/dev/_mq/' so the kernel knows how to handle this.
	char* id = (char*) malloc(strlen(path) + __PREFIX_LENGTH);
	memcpy(id, "/dev/_mq/", __PREFIX_LENGTH);
	memcpy(id + __PREFIX_LENGTH, path, strlen(path));

	// TODO: flags ignored for now
	return (int) Library::SystemCall::Open(id, flags);
}
