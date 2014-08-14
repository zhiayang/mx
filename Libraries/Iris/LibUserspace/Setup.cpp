// userspace/Setup.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <stdint.h>
#include "../HeaderFiles/SystemCall.hpp"
#include "../HeaderFiles/Heap.hpp"

extern "C" void Entry();

extern "C" void entry_main()
{
	// setup heap.
	Library::Heap::Initialise();

	// setup fds.
	Entry();
}
