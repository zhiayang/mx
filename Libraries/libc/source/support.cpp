// support.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "../include/stdint.h"
#include "../include/stdio.h"
#include "../include/errno.h"

static FILE _stdin;
static FILE _stdout;
static FILE _stderr;
static FILE _stdlog;

namespace Heap
{
	void Initialise();
}

extern "C" void init_libc()
{
	// init file descriptors
	_stdin.fd	= 0;
	_stdout.fd	= 1;
	_stderr.fd	= 2;
	_stdlog.fd	= 3;

	stdin	= &_stdin;
	stdout	= &_stdout;
	stderr	= &_stderr;
	stdlog	= &_stdlog;

	// init heap
	Heap::Initialise();
}


#ifdef __cplusplus
int* _get_errno()
{
	return &errno;
}
#else
extern "C" int* _get_errno()
{
}
#endif




