// userspace/Utility.hpp
// Copyright (c) 2013 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#pragma once
#include <stdint.h>

namespace Library
{
	namespace Utility
	{
		int64_t ParseInteger(const char* str, char** endptr, int base);
		char* ConvertToString(int64_t num, char* out = 0);
		int64_t ConvertToInt(char* str, uint8_t base = 10);

		uint64_t ReduceBinaryUnits(uint64_t bytes);
		uint64_t GetReducedMemory(uint64_t bytes);

		uint8_t* GetMD5Hash(uint8_t* Input, uint64_t Size);
		bool HashEqual(uint8_t* one, uint8_t* two);

		void SetBitmap(uint8_t* bitmap, uint64_t offset, uint64_t length = 1);
		bool CheckBitmap(uint8_t* bitmap, uint64_t offset, uint64_t length = 1);
		void ClearBitmap(uint8_t* bitmap, uint64_t offset, uint64_t length = 1);
	}
}































