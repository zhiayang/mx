// fcntl.h
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "stdint.h"
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// TODO: actually use these flags.
#define O_RDONLY 		(1 << 0)
#define O_WRONLY		(1 << 1)
#define O_RDWR		(O_RDONLY | O_WRONLY)

int open(const char* path, int flags);

#ifdef __cplusplus
}
#endif
