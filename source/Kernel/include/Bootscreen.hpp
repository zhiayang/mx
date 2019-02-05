// Bootscreen.hpp
// Copyright (c) 2013 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#pragma once
#include <stdint.h>

namespace Kernel
{
	namespace Bootscreen
	{
		void Initialise();
		void StartProgressBar();
		void PrintMessage(const char* msg);
	}
}
