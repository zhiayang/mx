// Iterator.hpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.



#pragma once

#include <stdint.h>

namespace Library
{

	template <typename T>
	class Iterator
	{
		public:
			typedef Iterator self_type;
			typedef T value_type;
			typedef T& reference;
			typedef T* pointer;
			typedef int difference_type;

			Iterator(pointer ptr) : ptr_(ptr)
			{

			}

			self_type operator++()
			{
				self_type i = *this;
				ptr_++;
				return i;
			}

			self_type operator++(int junk)
			{
				(void) junk;
				ptr_++;
				return *this;
			}

			reference operator*()
			{
				return *ptr_;
			}
			pointer operator->()
			{
				return ptr_;
			}
			bool operator==(const self_type& rhs)
			{
				return ptr_ == rhs.ptr_;
			}
			bool operator!=(const self_type& rhs)
			{
				return ptr_ != rhs.ptr_;
			}

		private:
			pointer ptr_;
	};

	template<class T>
	class Iterable
	{
		public:
			T& operator[](uint64_t index);
			const T& operator[](uint64_t index) const;
			Iterator<T> begin();
			Iterator<T> end();
	};
}




