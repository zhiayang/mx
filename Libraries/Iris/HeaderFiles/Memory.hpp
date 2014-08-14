// userspace/Memory.hpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#pragma once
#include <stdint.h>


namespace Library
{
	namespace Memory
	{
		void* Set(void* Dest, uint8_t Value, uint64_t Length);
		void* Set32(void* Dest, uint32_t Value, uint64_t Length);
		void* Set64(void* Dest, uint64_t Value, uint64_t Length);

		void* Copy(void* Dest, const void* Source, uint64_t Length);
		void* Copy32(void* Dest, const void* Source, uint64_t Length);
		void* Copy64(void* Dest, const void* Source, uint64_t Length);

		void* CopyOverlap(void* Dest, const void* Source, uint64_t Length);
		void* CopyOverlap32(void* Dest, const void* Source, uint64_t Length);
		void* CopyOverlap64(void* Dest, const void* Source, uint64_t Length);

		bool  Compare(const void* ptr1, const void* ptr2, uint64_t num);

	}
}
