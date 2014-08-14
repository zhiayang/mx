// userspace/PrintCharU.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <stdint.h>
#include "../HeaderFiles/SystemCall.hpp"

namespace Library {
namespace StandardIO
{
	void PrintChar(uint8_t c, void (*pf)(uint8_t))
	{
		if(pf)
			pf(c);

		else
			Library::SystemCall::WriteIPCSocket(1, &c, 1);
	}
}
}
