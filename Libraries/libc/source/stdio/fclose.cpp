// fclose.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "../../include/stdio.h"
#include "../../include/stdlib.h"
#include "../../include/unistd.h"
#include <sys/syscall.h>

extern "C" int fclose(FILE* file)
{
	if(!file || file->__fd < 4)
		return EOF;

	close((int) file->__fd);
	free(file);
	return 0;
}
