// errno.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "../include/defs/_tls.h"
#include "../include/errno.h"
#include "../include/stdlib.h"

extern "C" int* __get_errno()
{
	TLSData* tls = (TLSData*) __TLS_ADDR;
	return &tls->errnum;
}


extern "C" uintptr_t __get_tls_addr()
{
	// fetch value from %fs
	uintptr_t ret = 0;
	// asm volatile("mov %%gs, %[tls]" : [tls]"=r"(ret));
	return ret;
}
