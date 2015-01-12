// userspace/Memory.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "HeaderFiles/Memory.hpp"
#include <stddef.h>

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

	inline static void* memcpy_slow(void* __restrict__ dstptr, const void* __restrict__ srcptr, size_t length)
	{
		uint8_t* __restrict__ dst = (uint8_t* __restrict__) dstptr;
		const uint8_t* __restrict__ src = (const uint8_t* __restrict__) srcptr;
		for(size_t i = 0; i < length; i += sizeof(uint8_t))
			dst[i] = src[i];

		return dstptr;
	}

	void* Copy(void* __restrict__ dstptr, const void* __restrict__ srcptr, size_t length)
	{
		const unsigned long unalignmask = sizeof(unsigned long) - 1;
		const unsigned long srcunalign = (unsigned long) srcptr & unalignmask;
		const unsigned long dstunalign = (unsigned long) dstptr & unalignmask;

		if(srcunalign != dstunalign)
			return memcpy_slow(dstptr, srcptr, length);

		union
		{
			unsigned long srcval;
			const uint8_t* __restrict__ src8;
			const uint16_t* __restrict__ src16;
			const uint32_t* __restrict__ src32;
			const uint64_t* __restrict__ src64;
			const unsigned long* __restrict__ srcul;
		};
		srcval = (unsigned long) srcptr;

		union
		{
			unsigned long dstval;
			uint8_t* __restrict__ dst8;
			uint16_t* __restrict__ dst16;
			uint32_t* __restrict__ dst32;
			uint64_t* __restrict__ dst64;
			unsigned long* __restrict__ dstul;
		};
		dstval = (unsigned long) dstptr;

		if(dstunalign)
		{
			if(1 <= length && !(dstval & (1 - 1)) && (dstval & (2 - 1)))
				*dst8++ = *src8++,
				length -= 1;

			if(2 <= length && !(dstval & (2 - 1)) && (dstval & (4 - 1)))
				*dst16++ = *src16++,
				length -= 2;

			if(4 <= length && !(dstval & (4 - 1)) && (dstval & (8 - 1)))
				*dst32++ = *src32++,
				length -= 4;
		}

		size_t numcopies = length / sizeof(unsigned long);

		for(size_t i = 0; i < numcopies; i++)
			*dstul++ = *srcul++;

		length -= numcopies * sizeof(unsigned long);

		if(length)
		{
			if(4 <= length)
				*dst32++ = *src32++,
				length -= 4;

			if(2 <= length)
				*dst16++ = *src16++,
				length -= 2;

			if(1 <= length)
				*dst8++ = *src8++,
				length -= 1;
		}

		return dstptr;
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

