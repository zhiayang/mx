// malloc.h
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "sys/cdefs.h"
#include "stddef.h"

__BEGIN_DECLS

// malloc and friends
void*	malloc(size_t size);
void	free(void* ptr);
void*	calloc(size_t num, size_t size);
void*	realloc(void* ptr, size_t newsize);

__END_DECLS
