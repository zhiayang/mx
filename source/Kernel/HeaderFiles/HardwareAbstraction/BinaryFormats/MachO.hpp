// MachO.hpp
// Copyright (c) 2011-2013, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.



#pragma once
#include <stdint.h>

// struct mach_header_64
// {
// 	uint32_t	magic;		/* mach magic number identifier */
// 	cpu_type_t	cputype;	/* cpu specifier */
// 	cpu_subtype_t	cpusubtype;	/* machine specifier */
// 	uint32_t	filetype;	 type of file
// 	uint32_t	ncmds;		/* number of load commands */
// 	uint32_t	sizeofcmds;	/* the size of all the load commands */
// 	uint32_t	flags;		/* flags */
// 	uint32_t	reserved;	/* reserved */
// };
