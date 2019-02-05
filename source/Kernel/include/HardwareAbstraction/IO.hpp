// IOScheduler.hpp
// Copyright (c) 2014 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>

namespace Kernel {
namespace HardwareAbstraction {
namespace IO
{
	void Initialise();
	void Read(Devices::IODevice* dev, uint64_t pos, uint64_t buf, uint64_t bytes);
	void Write(Devices::IODevice* dev, uint64_t pos, uint64_t buf, uint64_t bytes);


	// non blocking.
	// returns void pointer (opaque type essentially) to check status.
	void* ScheduleRead(Devices::IODevice* dev, uint64_t pos, uint64_t buf, uint64_t bytes);
	void* ScheduleWrite(Devices::IODevice* dev, uint64_t pos, uint64_t buf, uint64_t bytes);
	bool CheckStatus(void* request);
}
}
}
