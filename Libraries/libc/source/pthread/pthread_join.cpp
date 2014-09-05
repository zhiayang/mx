// pthread_join.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <pthread.h>
#include <sys/syscall.h>
#include <string.h>

extern "C" int pthread_join(pthread_t thread, void** retval)
{
	// implicit join with return value.
	void* ret = Library::SystemCall::JoinThread(thread);
	if(retval)
		*retval = ret;

	return 0;
}
