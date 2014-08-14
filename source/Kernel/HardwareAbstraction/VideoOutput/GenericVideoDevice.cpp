// GenericVideoDevice.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under Creative Commons 3.0 Unported.

#include <Kernel.hpp>

namespace Kernel {
namespace HardwareAbstraction {
namespace VideoOutput
{
	GenericVideoDevice::GenericVideoDevice(GenericVideoDevice* dev)
	{
		this->Device = dev;
	}

	GenericVideoDevice::~GenericVideoDevice()
	{
	}

	void GenericVideoDevice::SetMode(uint16_t x, uint16_t y, uint16_t d)
	{
		this->Device->SetMode(x, y, d);
	}

	uint16_t GenericVideoDevice::GetResX()
	{
		return this->Device->GetResX();
	}

	uint16_t GenericVideoDevice::GetResY()
	{
		return this->Device->GetResY();
	}

	uint16_t GenericVideoDevice::GetBitDepth()
	{
		return this->Device->GetBitDepth();
	}

	uint64_t GenericVideoDevice::GetFramebufferAddress()
	{
		return this->Device->GetFramebufferAddress();
	}
}
}
}














