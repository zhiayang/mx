// Vector.hpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0


#include <stdint.h>
#include "Memory.hpp"
#include "Iterator.hpp"

#pragma once

namespace Library
{
	template <class V>
	class Vector : public Iterable<V>
	{
		public:
			Vector(uint64_t StartingSize = 4)
			{
				this->DataArray = new V[StartingSize];
				this->size = 0;
				this->allocatedsize = StartingSize;
			}

			V& Array(const uint64_t i)
			{
				return this->DataArray[i];
			}

			uint64_t Size()
			{
				return this->size;
			}

			uint64_t AllocatedSize()
			{
				return this->allocatedsize;
			}

			void InsertBack(V v)
			{
				if(this->size < this->allocatedsize)
				{
					this->DataArray[this->size] = v;
					this->size++;
				}
				else
				{
					V* newarray = new V[this->allocatedsize * 2];
					this->allocatedsize *= 2;

					// V needs copy constructor or assignmnet operator.
					for(uint64_t i = 0; i < this->size; i++)
						newarray[i] = this->DataArray[i];

					// Memory::Copy(newarray, this->DataArray, this->allocatedsize * sizeof(V));
					newarray[this->size] = v;

					delete[] this->DataArray;
					this->DataArray = newarray;
					this->size++;
				}
			}

			// void InsertFront(V v)
			// {
			// 	V* newarray = new V[this->allocatedsize * (this->size < this->allocatedsize ? 1 : 2)];
			// 	Memory::Copy((V*)((uintptr_t) newarray + sizeof(V)), this->DataArray, this->size * sizeof(V));
			// 	newarray[0] = v;

			// 	delete[] this->DataArray;
			// 	this->DataArray = newarray;
			// }

			V& RemoveBack()
			{
				if(this->size > 0)
				{
					V& v = this->DataArray[this->size - 1];
					this->size--;
					return v;
				}

				V a;
				return a;
			}

			V& RemoveFront()
			{
				if(this->size > 0)
				{
					V& v = this->DataArray[0];
					this->size--;
					return v;
				}

				V* a = 0;
				return *a;
			}

			void Remove(V v)
			{
				Vector<V>* copy = new Vector<V>();
				for(V val : *this)
				{
					if(val == v)
						continue;

					else
						copy->InsertBack(val);
				}

				// TODO: MEMORY LEAK

				delete[] this->DataArray;
				this->DataArray = copy->DataArray;
			}

			V& Front()
			{
				return this->DataArray[0];
			}

			V& Back()
			{
				return this->DataArray[this->size - 1];
			}

			Iterator<V> begin()
			{
				return Iterator<V>(this->DataArray);
			}

			Iterator<V> end()
			{
				return Iterator<V>(this->DataArray + this->size);
			}

			V& operator[](const uint64_t index)
			{
				return this->Array(index);
			}


		private:
			V* DataArray;
			uint64_t size;
			uint64_t allocatedsize;
	};
}








