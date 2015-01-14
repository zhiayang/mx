// GenericVideoDevice.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <HardwareAbstraction/VideoOutput/VideoDevice.hpp>

using namespace Kernel::HardwareAbstraction::Devices;

namespace Kernel {
namespace HardwareAbstraction {
namespace VideoOutput
{
	GenericVideoDevice::~GenericVideoDevice()
	{
	}

	void GenericVideoDevice::SetMode(uint16_t, uint16_t, uint16_t)
	{

	}

	uint16_t GenericVideoDevice::GetResX()
	{
		return 0;
	}

	uint16_t GenericVideoDevice::GetResY()
	{
		return 0;
	}

	uint16_t GenericVideoDevice::GetBitDepth()
	{
		return 0;
	}

	uint64_t GenericVideoDevice::GetFramebufferAddress()
	{
		return 0;
	}

}
}
}
