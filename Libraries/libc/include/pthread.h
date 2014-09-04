// pthread.h
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#ifndef __pthread_h
#define __pthread_h

#include "sys/cdefs.h"

__BEGIN_DECLS
#include "stddef.h"
#include "stdint.h"
#include "sys/types.h"
#include "defs/_pthreadstructs.h"

int pthread_create(pthread_t* restrict thread, const pthread_attr_t* restrict attr, void *(*start_routine)(void*), void *restrict arg);
int pthread_join(pthread_t thread, void** retval);
pthread_t pthread_self();











__END_DECLS
#endif
