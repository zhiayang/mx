// GlobalTypes.hpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <stdint.h>
#pragma once

namespace Kernel
{
	struct DMAAddr
	{
		DMAAddr() { phys = 0; virt = 0; }
		DMAAddr(uint64_t p, uint64_t v) : phys(p), virt(v) { }
		uint64_t phys;
		uint64_t virt;
	};
}
