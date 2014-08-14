// exit.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#include "../../include/stdlib.h"

func_void_void_t __exitfunc = 0;

extern "C" void _fini();
extern "C" void exit(int ret)
{
	if(__exitfunc)
		__exitfunc();

	(void) ret;
	_fini();

	// do a system call
	asm volatile("xor %%r10, %%r10; int $0xF8" ::: "%r10");
	while(true);
}

extern "C" void abort()
{
	exit(-69);
}
