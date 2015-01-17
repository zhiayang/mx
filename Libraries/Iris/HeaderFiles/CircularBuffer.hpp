// CircularBuffer.hpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#pragma once
#include <stdint.h>
#include "Memory.hpp"
#include <math.h>

namespace Library
{
	class CircularMemoryBuffer
	{
		public:
			CircularMemoryBuffer(uint64_t s);
			CircularMemoryBuffer(CircularMemoryBuffer& other);
			~CircularMemoryBuffer();

			void MoveWritePointer(uint64_t p);
			void MoveReadPointer(uint64_t p);

			uint64_t GetReadPointer();
			uint64_t GetWritePointer();
			uint64_t TotalSize();
			uint64_t ByteCount();


			uint64_t PeekRead(uint8_t* data, uint64_t bytes);
			uint64_t PeekWrite(uint8_t* data, uint64_t bytes);

			uint64_t Read(uint8_t* data, uint64_t bytes);
			uint64_t Write(uint8_t* data, uint64_t bytes);


			uint8_t* backingstore;

		private:
			uintptr_t readp;
			uintptr_t writep;
			uint64_t size;
			uint64_t elements;
	};
}


