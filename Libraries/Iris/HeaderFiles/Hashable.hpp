// Hashable.hpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0


#include <stdint.h>
#pragma once

namespace Library
{
	class Hashable
	{
		public:
			// returns a hash of the contents of the object.
			// should be unique.
			virtual uint64_t Hash() = 0;

		protected:
			virtual ~Hashable();
	};
}
