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


