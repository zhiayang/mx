// userspace/Memory.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "HeaderFiles/Memory.hpp"

extern "C" void* memset(void* d, char v, unsigned long l)
{
	return Library::Memory::Set(d, (uint8_t) v, l);
}
extern "C" void* memcpy(void* d, void* s, unsigned long l)
{
	return Library::Memory::Copy(d, s, l);
}
extern "C" void* memmove(void* d, void* s, unsigned long l)
{
	return Library::Memory::CopyOverlap(d, s, l);
}

namespace Library
{
	namespace Memory
	{
		void* Set(void* destptr, uint8_t value, uint64_t length)
		{
			uint8_t* dest = (uint8_t*) destptr;

			for(uint64_t i = 0; i < length; i++)
				dest[i] = value;

			return destptr;
		}


		// Fill memory with given 4-byte value val.

		void* Set32(void* dst, uint32_t val, uint64_t len)
		{
			uintptr_t d0 = 0;
			uint64_t uval = ((uint64_t) val << 32) + val;
			asm volatile(
				"rep stosq"
				:"=&D" (d0), "+&c" (len)
				:"0" (dst), "a" (uval)
				:"memory");

			return dst;
		}


		// Fill memory with given 8-byte value val.

		void* Set64(void* dst, uint64_t val, uint64_t len)
		{
			uintptr_t d0 = 0;

			asm volatile(
				"rep stosq"
				:"=&D" (d0), "+&c" (len)
				:"0" (dst), "a" (val)
				:"memory");

			return dst;
		}







		void* Copy(void* dest, const void* src, uint64_t len)
		{

			asm volatile("cld; rep movsb"
			: "+c" (len), "+S" (src), "+D" (dest)
			:: "memory");


			return dest;
		}

		void* Copy32(void* dest, const void* src, uint64_t len)
		{
			asm volatile("cld; rep movsd"
			: "+c" (len), "+S" (src), "+D" (dest)
			:: "memory");


			return dest;
		}

		void* Copy64(void* dest, const void* src, uint64_t len)
		{
			asm volatile("cld; rep movsq"
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

		void* CopyOverlap32(void* dst, const void* src, uint64_t n)
		{
			uint8_t* a = (uint8_t*) dst;
			const uint8_t* b = (uint8_t*) src;
			if(a <= b || b >= (a + n))
			{
				// No overlap, use memcpy logic (copy forward)
				Copy32(dst, src, n);
			}
			else
			{
				asm volatile("std");
				Copy32(dst, src, n);
				asm volatile("cld");
			}
			return dst;
		}

		void* CopyOverlap64(void* dst, const void* src, uint64_t n)
		{
			uint8_t* a = (uint8_t*) dst;
			const uint8_t* b = (uint8_t*) src;
			if(a <= b || b >= (a + n))
			{
				// No overlap, use memcpy logic (copy forward)
				Copy64(dst, src, n);
			}
			else
			{
				asm volatile("std");
				Copy64(dst, src, n);
				asm volatile("cld");
			}
			return dst;
		}

		bool Compare(const void* a, const void* b, uint64_t num)
		{
			const uint8_t* buf1 = (const uint8_t*) a;
			const uint8_t* buf2 = (const uint8_t*) b;
			for(uint64_t i = 0; i < num; i++)
			{
				if(buf1[i] != buf2[i])
				{
					return false;
				}
			}
			return true;
		}
	}
}


