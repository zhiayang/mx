// userspace/Memory.hpp
// Copyright (c) 2013 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#pragma once
#include <stdint.h>

namespace Memory
{
	void* Set(void* Dest, uint8_t Value, uint64_t Length);
	void* Copy(void* Dest, const void* Source, uint64_t Length);
	void* CopyOverlap(void* Dest, const void* Source, uint64_t Length);

	int  Compare(const void* ptr1, const void* ptr2, uint64_t num);

}
