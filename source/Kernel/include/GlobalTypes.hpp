// GlobalTypes.hpp
// Copyright (c) 2014 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <stdint.h>
#include <stddef.h>
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

	struct IOResult
	{
		IOResult() : bytesTransferred(0), allocatedBuffer(0, 0), bufferSizeInPages(0) { }
		IOResult(size_t bytes, DMAAddr buf, size_t bufSize) : bytesTransferred(bytes), allocatedBuffer(buf), bufferSizeInPages(bufSize) { }

		size_t bytesTransferred;

		DMAAddr allocatedBuffer;
		size_t bufferSizeInPages;
	};
}
