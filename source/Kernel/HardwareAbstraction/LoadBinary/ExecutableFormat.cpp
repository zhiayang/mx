// ExecutableFormat.cpp
// Copyright (c) 2014 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <HardwareAbstraction/LoadBinary.hpp>

namespace Kernel {
namespace HardwareAbstraction {
namespace LoadBinary
{
	ExecutableFormat::~ExecutableFormat()
	{
	}

	uint64_t ExecutableFormat::GetEntryPoint()
	{
		return 0;
	}

	uint64_t ExecutableFormat::GetTLSSize()
	{
		return 0;
	}

	void ExecutableFormat::Load(Multitasking::Process*)
	{
	}
}
}
}
