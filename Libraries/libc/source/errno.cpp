// errno.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "../include/errno.h"
#include "../include/stdlib.h"

extern "C"
{
	int __thread _errno = 0;
}
