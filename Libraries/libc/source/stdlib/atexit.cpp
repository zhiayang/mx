// atexit.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "../../include/stdlib.h"

extern "C" int atexit(func_void_void_t func)
{
	__exitfunc = func;
	return 0;
}
