// _file.h
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#ifndef __file_h
#define __file_h
#pragma once

#include "../stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

#define	BUFSIZ			1


struct _FILE
{
	uint64_t __fd;
	uint8_t __PermFlags;
};

typedef struct _FILE FILE;

#ifdef __cplusplus
}
#endif

#endif
