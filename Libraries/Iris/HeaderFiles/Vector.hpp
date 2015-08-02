// Vector.hpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>

#pragma once

#define INITIAL_CAPACITY		64

namespace iris
{
	template <typename T>
	class vector
	{
		class iterator
		{
			public:
				typedef iterator self_type;
				typedef T value_type;
				typedef T& reference;
				typedef T* pointer;

				// typedef std::forward_iterator_tag iterator_category;
				typedef int difference_type;
				iterator(pointer ptr) : ptr_(ptr) { }

				self_type operator++() { self_type i = *this; ptr_++; return i; } // prefix
				self_type operator++(int) { ptr_++; return *this; } // postfix

				self_type operator--() { self_type i = *this; ptr_--; return i; } // prefix
				self_type operator--(int) { ptr_--; return *this; } // postfix

				size_t operator-(self_type other)
				{
					size_t b = (this->ptr_ > other.ptr_) ? ((uintptr_t) this->ptr_ - (uintptr_t) other.ptr_)
									: ((uintptr_t) other.ptr_ - (uintptr_t) this->ptr_);

					return b / sizeof(T);
				}


				reference operator*() { return *ptr_; }
				pointer operator->() { return ptr_; }
				bool operator==(const self_type& rhs) { return ptr_ == rhs.ptr_; }
				bool operator!=(const self_type& rhs) { return ptr_ != rhs.ptr_; }

			private:
				pointer ptr_;
		};

		class const_iterator
		{
			public:
				typedef const_iterator self_type;
				typedef T value_type;
				typedef T& reference;
				typedef T* pointer;
				typedef int difference_type;

				// typedef std::forward_iterator_tag iterator_category;
				const_iterator(pointer ptr) : ptr_(ptr) { }

				self_type operator++() { self_type i = *this; ptr_++; return i; } // prefix
				self_type operator++(int) { ptr_++; return *this; } // postfix

				self_type operator--() { self_type i = *this; ptr_--; return i; } // prefix
				self_type operator--(int) { ptr_--; return *this; } // postfix


				size_t operator-(self_type other)
				{
					size_t b = (this->ptr_ > other.ptr_) ? ((uintptr_t) this->ptr_ - (uintptr_t) other.ptr_)
									: ((uintptr_t) other.ptr_ - (uintptr_t) this->ptr_);

					return b / sizeof(T);
				}

				const value_type& operator*() const { return *ptr_; }
				const value_type* operator->() const { return ptr_; }
				bool operator==(const self_type& rhs) { return ptr_ == rhs.ptr_; }
				bool operator!=(const self_type& rhs) { return ptr_ != rhs.ptr_; }

			private:
				pointer ptr_;
		};


		private:
			void _quicksort(T* data, uint64_t low, uint64_t high)
			{
				while(true)
				{
					uint64_t i = low;
					uint64_t j = high;
					const T pivot = data[(low + high) >> 1];
					do
					{
						// Jump over elements that are OK (smaller than pivot)
						while(data[i] < pivot /* pred(data[i], pivot) */)
							i++;

						// Jump over elements that are OK (greater than pivot)
						while(pivot < data[j] /* pred(pivot, data[j]) */)
							j--;

						// Anything to swap?
						if(j >= i)
						{
							if(i != j)
							{
								// Swap
								T tmp(data[i]);
								data[i] = data[j];
								data[j] = tmp;
							}

							i++;
							j--;
						}
					}
					while(i <= j);

					if(low < j)
						_quicksort(data, low, j);


					if(i < high)
						low = i;	// that's basically quick_sort(data, i, high, pred), but we avoid recursive call.

					else
						break;

				}
			}



			vector _mergesort(vector& a, vector& b)
			{
				vector result;
				size_t i = 0;
				size_t j = 0;
				while(i < a.size() && j < b.size())
				{
					if(a[i] <= b[j])
						result.push_back(a[i++]);

					else
						result.push_back(b[j++]);
				}

				// Copy tail. Only one of these loops will execute per invocation
				while(i < a.size())
					result.push_back(a[i++]);

				while(j < b.size())
					result.push_back(b[j++]);

				return result;
			}



		private:
			T*				array;
			size_t			_capacity;
			size_t			elements;


			void doCopyConstruct(T* src, T* dest, size_t num)
			{
				// call constructors
				for(size_t i = 0; i < num; i++)
					new (dest + i) T(src[i]);
			}


			void grow(size_t newSize)
			{
				assert(newSize > this->elements);
				// T* newArr = new T[newSize];
				T* newArr = (T*) new uint8_t[newSize * sizeof(T)];

				this->doCopyConstruct(this->array, newArr, this->elements);


				delete[] this->array;
				this->array = newArr;

				this->_capacity = newSize;
			}



		public:
			size_t size()		{ return this->elements; }
			size_t capacity()	{ return this->_capacity; }
			bool empty()		{ return this->elements == 0; }


			~vector()
			{
				delete[] this->array;
			}

			vector(size_t cap)
			{
				this->array		= new T[cap];
				this->_capacity	= cap;
				this->elements	= 0;
			}

			vector(const vector& other)
			{
				if(other._capacity > 0)
				{
					this->_capacity	= other._capacity;
					this->array		= new T[this->_capacity];
					this->elements	= other.elements;

					if(this->elements > 0)
					{
						memcpy(this->array, other.array, this->elements);
					}
				}
				else
				{
					this->_capacity = 0;
					this->array = 0;
					this->elements = 0;
				}
			}

			vector() : vector(INITIAL_CAPACITY)
			{
			}

			vector(vector& other) : vector((const vector&) other)
			{

			}

			vector(iterator _start, iterator _end)
			{
				size_t required_cap = _end - _start;
				this->_capacity = required_cap;
				this->array = (T*) new uint8_t[this->_capacity * sizeof(T)];
				this->elements = 0;


				for(auto it = _start; it != _end; it++, this->elements++)
				{
					this->array[this->elements] = *it;
				}
			}

			void clear()
			{
				this->elements = 0;
				this->_capacity = 0;
				delete[] this->array;
			}

			T& front()
			{
				assert(this->elements > 0);
				return this->array[0];
			}

			const T& front() const
			{
				assert(this->elements > 0);
				return this->array[0];
			}

			T& back()
			{
				assert(this->elements > 0);
				return this->array[this->elements - 1];
			}

			const T& back() const
			{
				assert(this->elements > 0);
				return this->array[this->elements - 1];
			}




			T pop_back()
			{
				assert(this->elements > 0);
				T rear = this->array[this->elements - 1];

				// memset, to be very sure.
				memset(this->array + this->elements - 1, 0, sizeof(T));

				this->elements--;
				return rear;
			}

			T pop_front()
			{
				assert(this->elements > 0);
				T front = this->array[0];

				memmove(this->array, this->array + 1, this->elements - 2);
				this->elements--;
				return front;
			}

			void push_back(T val)
			{
				if(this->elements == this->_capacity)
				{
					this->grow((size_t) ((double) this->_capacity * 1.5));
				}

				assert(this->_capacity > this->elements);
				this->array[this->elements] = val;
				this->elements++;
			}


			// *Much* quicker version of erase, doesnt preserve collection order.
			void erase(iterator it)
			{
				this->erase_unordered(it);
			}

			void erase_unordered(iterator it)
			{
				assert(it != this->end());

				iterator itNewEnd = this->end();
				itNewEnd--;

				if(it != itNewEnd)
					*it = *itNewEnd;

				this->pop_back();
			}

			void remove(const T& value)
			{
				for(iterator it = this->begin(); it != this->end(); it++)
				{
					if(*it == value)
					{
						this->erase(it);
						break;
					}
				}
			}






			void quick_sort()
			{
				this->_quicksort(this->array, 0, this->elements);
			}

			void merge_sort()
			{
				// split in half.
				if(this->elements <= 1)
					return;

				iterator middle = iterator(this->array + (this->elements / 2));
				vector left(this->begin(), middle);
				vector right(middle, this->end());

				assert(left.size() + right.size() == this->size());
				auto result = this->_mergesort(left, right);

				*this = result;
			}











			// operators


			vector& operator=(const vector& other)
			{
				this->_capacity	= other._capacity;
				this->elements	= other.elements;
				this->array		= new T[this->_capacity];

				this->doCopyConstruct(other.array, this->array, this->elements);

				return *this;
			}

			vector& operator=(vector& other)
			{
				return (*this = (const vector&) other);
			}

			T& operator[](size_t i)
			{
				assert(this->elements > i);
				return this->array[i];
			}

			const T& operator[](size_t i) const
			{
				return (*this)[i];
			}











			// iterator stuff

			iterator begin()
			{
				return iterator(this->array);
			}

			iterator end()
			{
				return iterator(this->array + this->elements);
			}

			const_iterator begin() const
			{
				return const_iterator(this->array);
			}

			const_iterator end() const
			{
				return const_iterator(this->array + this->elements);
			}
	};
}














