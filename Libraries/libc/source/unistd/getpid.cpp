// getpid.cpp
// Copyright (c) 2014 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "../../include/unistd.h"
#include <sys/syscall.h>

extern "C" pid_t getpid()
{
	return (pid_t) Library::SystemCall::GetPID();
}

extern "C" pid_t getppid()
{
	return (pid_t) Library::SystemCall::GetParentPID();
}
