// userspace/Memory.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "HeaderFiles/Memory.hpp"

extern "C" void* memset(void* d, char v, unsigned long l)
{
	return Memory::Set(d, (uint8_t) v, l);
}
extern "C" void* memcpy(void* d, void* s, unsigned long l)
{
	return Memory::Copy(d, s, l);
}
extern "C" void* memmove(void* d, void* s, unsigned long l)
{
	return Memory::CopyOverlap(d, s, l);
}


namespace Memory
{
	void* Set(void* destptr, uint8_t value, uint64_t length)
	{
		uint8_t* dest = (uint8_t*) destptr;

		for(uint64_t i = 0; i < length; i++)
			dest[i] = value;

		return destptr;
	}

	void* Copy(void* dest, const void* src, uint64_t len)
	{

		asm volatile("cld; rep movsb"
		: "+c" (len), "+S" (src), "+D" (dest)
		:: "memory");


		return dest;
	}

	void* CopyOverlap(void* dst, const void* src, uint64_t n)
	{
		uint8_t* a = (uint8_t*) dst;
		const uint8_t* b = (uint8_t*) src;
		if(a <= b || b >= (a + n))
		{
			// No overlap, use memcpy logic (copy forward)
			Copy(dst, src, n);
		}
		else
		{
			asm volatile("std");
			Copy(dst, src, n);
			asm volatile("cld");
		}
		return dst;
	}

	int Compare(const void* a, const void* b, uint64_t num)
	{
		const uint8_t* buf1 = (const uint8_t*) a;
		const uint8_t* buf2 = (const uint8_t*) b;
		for(uint64_t i = 0; i < num; i++)
		{
			if(buf1[i] > buf2[i])
				return 1;

			if(buf1[i] < buf2[i])
				return -1;
		}
		return 0;
	}
}

