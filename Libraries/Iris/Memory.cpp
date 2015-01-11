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
	void* Set(void* ptr, uint8_t value, uint64_t num)
	{
		// 'stosl' will use the value in eax but we only want the value in al
		// so we make eax = al << 24 | al << 16 | al << 8 | al
		if((value & 0xff) == 0)
			// this is a little optimization because memset is most often called with value = 0
			value = 0;

		else
		{
			value = (value & 0xff) | ((value & 0xff) << 8);
			value = (value & 0xffff) | ((value & 0xffff) << 16);
		}

		void* temporaryPtr = ptr;
		asm volatile("rep stosl ; mov %3, %2 ; rep stosb" : "+D"(temporaryPtr) : "a"(value), "c"(num / 4), "r"(num % 4) : "cc", "memory");
		return ptr;
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

