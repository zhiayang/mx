// Comparable.hpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0

#include <stdint.h>
#pragma once

namespace Library
{
	template<class T>
	class Comparable
	{
		public:
			// returns 1 if a > b, 0 if a == b, -1 if a < b
			virtual int Compare(T a, T b) = 0;

			// returns 1 if a is > self, 0 if a == self, -1 if a < self
			virtual int Compare(T a) = 0;

		protected:
			virtual ~Comparable()
			{
			}
	};
}
