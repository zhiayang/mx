// _libcxx_helpers.h
// Copyright (c) thePowersGang (John Hodge)
// See LICENSE file.

#ifndef _LIBCXX__LIBCXX_HELEPRS_H_
#define _LIBCXX__LIBCXX_HELEPRS_H_

#include <assert.h>

#if __cplusplus > 199711L	// C++11 check
# define _CXX11_AVAIL	1
#else
# define _CXX11_AVAIL	0
#endif

#define _libcxx_assert(cnd)	assert(cnd)


#if _CXX11_AVAIL
#define _CXX11_MOVE(val)	::stl::move(val)
#else
#define _CXX11_MOVE(val)	val
#endif

#endif
