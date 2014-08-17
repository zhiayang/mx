// Device.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>

namespace Kernel {
namespace HardwareAbstraction {
namespace Devices
{
	static dev_t curid = 0;

	dev_t GetDevID()
	{
		// skips 0, but who gives a shit
		curid++;
		return curid;
	}

	void FreeDevID(dev_t id)
	{
		(void) id;
	}
}
}
}
