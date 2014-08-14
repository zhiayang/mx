// stddef.h
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#ifndef _STDC_STDDEF_H_
#define _STDC_STDDEF_H_

#undef NULL
#ifdef __cplusplus
	#define NULL	0
#else
	#define NULL	((void*) 0)
#endif

#undef __WORDSIZE
#define __WORDSIZE 64

typedef __SIZE_TYPE__		size_t;
typedef __PTRDIFF_TYPE__		ptrdiff_t;
// typedef __WCHAR_TYPE__		wchar_t;

#define offsetof(st, m) \
    ((size_t)((char*)&((st*)(0))->m - (char*)0))


#define TODO		assert(false)


#endif


