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
				this->length = 0;
				this->allocatedsize = StartingSize;
			}

			V& Array(const uint64_t i)
			{
				return this->DataArray[i];
			}

			uint64_t size()
			{
				return this->length;
			}

			uint64_t AllocatedSize()
			{
				return this->allocatedsize;
			}

			void push_back(V v)
			{
				if(this->length < this->allocatedsize)
				{
					this->DataArray[this->length] = v;
					this->length++;
				}
				else
				{
					V* newarray = new V[this->allocatedsize * 2];
					this->allocatedsize *= 2;

					// V needs copy constructor or assignmnet operator.
					for(uint64_t i = 0; i < this->length; i++)
						newarray[i] = this->DataArray[i];

					// Memory::Copy(newarray, this->DataArray, this->allocatedsize * sizeof(V));
					newarray[this->length] = v;

					delete[] this->DataArray;
					this->DataArray = newarray;
					this->length++;
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

			V& pop_back()
			{
				if(this->size > 0)
				{
					V& v = this->DataArray[this->length - 1];
					this->length--;
					return v;
				}

				V a;
				return a;
			}

			V& pop_front()
			{
				// todo: fix old code here.
				if(this->length > 0)
				{
					V& v = this->DataArray[0];
					this->length--;
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
						copy->push_back(val);
				}

				// TODO: MEMORY LEAK

				delete[] this->DataArray;
				this->DataArray = copy->DataArray;
			}

			V& front()
			{
				return this->DataArray[0];
			}

			V& back()
			{
				return this->DataArray[this->length - 1];
			}

			Iterator<V> begin()
			{
				return Iterator<V>(this->DataArray);
			}

			Iterator<V> end()
			{
				return Iterator<V>(this->DataArray + this->length);
			}

			V& operator[](const uint64_t index)
			{
				return this->Array(index);
			}


		private:
			V* DataArray;
			uint64_t length;
			uint64_t allocatedsize;
	};
}








