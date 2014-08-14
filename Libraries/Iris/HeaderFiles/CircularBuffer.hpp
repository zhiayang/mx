// CircularBuffer.hpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#pragma once
#include <stdint.h>
#include "Memory.hpp"
#include <math.h>

namespace Library
{
	template <class T>
	class CircularBuffer
	{
		public:
			// Creates a buffer with 'slots' slots.
			explicit CircularBuffer(uint64_t slots = 32)
			{
				this->ReadIndex = 0;
				this->WriteIndex = 0;
				this->TotalSize = slots;
				this->size = 0;

				this->Clear();
			}

			// Destructor.
			~CircularBuffer()
			{
				delete[] this->Data;
			}

			// Writes 'value' to the next available slot. It may overwrite
			// values that were not yet read out of the buffer.
			void Write(T value)
			{
				if(this->WriteIndex < this->TotalSize)
				{
					this->Data[this->WriteIndex] = value;
					this->WriteIndex++;

					if(this->size < this->TotalSize)
						this->size++;
				}
				else
				{
					this->WriteIndex = 0;
					this->Data[this->WriteIndex] = value;
				}
			}

			// Returns the next value available for reading, in the order they
			// were written, and marks slot as read. If the buffer is empty returns -1.
			T Read()
			{
				if(this->ReadIndex < this->TotalSize)
				{
					T ret = this->Data[this->ReadIndex];
					this->Data[this->ReadIndex] = 0;

					this->ReadIndex++;
					this->size--;
					return ret;
				}
				else
				{
					this->ReadIndex = 1;
					T ret = this->Data[0];
					this->Data[0] = 0;

					this->size--;
					return ret;
				}
			}

			// Removes all the elements from the buffer.
			void Clear()
			{
				delete[] this->Data;
				this->Data = new T[this->TotalSize];

				this->ReadIndex = 0;
				this->WriteIndex = 0;
				this->size = 0;

				Memory::Set(reinterpret_cast<void*>(this->Data), 0x0, sizeof(T) * this->TotalSize);
			}

			uint64_t Size()
			{
				return this->size;
			}

		private:
			// array of integers
			T* Data;

			// the size of the buffer
			uint64_t TotalSize;

			// index to read the next integer from buffer
			uint64_t ReadIndex;

			// index to write a new integer to buffer
			uint64_t WriteIndex;

			// number of items.
			uint64_t size;

			// Non-copyable, non-assignable.
			CircularBuffer(CircularBuffer&);
			CircularBuffer& operator=(const CircularBuffer&);
	};


	class CircularMemoryBuffer
	{
		public:
			CircularMemoryBuffer(uint64_t s);
			~CircularMemoryBuffer();

			void MoveWritePointer(uint64_t p);
			void MoveReadPointer(uint64_t p);

			uint64_t GetWritePointer();
			uint64_t TotalSize();
			uint64_t ByteCount();

			uint8_t* Read(uint8_t* data, uint64_t bytes);
			uint8_t* Write(uint8_t* data, uint64_t bytes);


			uint8_t* backingstore;

		private:
			uintptr_t readp;
			uintptr_t writep;
			uint64_t size;
			uint64_t elements;
	};
}


