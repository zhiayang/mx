// _tls.h
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "../sys/types.h"
#include "../sys/cdefs.h"

__BEGIN_DECLS

#define TLS_ADDR	0x2610
struct TLSData
{
	int errnum;
};

__END_DECLS
