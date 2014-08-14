// GenericVESA.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under Creative Commons 3.0 Unported.

#include <Kernel.hpp>
#include <Memory.hpp>

using namespace Kernel::HardwareAbstraction::Devices;

namespace Kernel {
namespace HardwareAbstraction {
namespace VideoOutput
{
	GenericVESA::GenericVESA(PCI::PCIDevice* thisdev) : GenericVideoDevice(this)
	{
		this->PCIDev = thisdev;


	}

	void GenericVESA::SetMode(uint16_t w, uint16_t h, uint16_t bd)
	{
		(void) w;
		(void) h;
		(void) bd;
	}

	uint16_t GenericVESA::GetResX()
	{
		return 0;
	}

	uint16_t GenericVESA::GetResY()
	{
		return 0;
	}

	uint16_t GenericVESA::GetBitDepth()
	{
		return 0;
	}

	uint64_t GenericVESA::GetFramebufferAddress()
	{
		return 0;
	}
}
}
}














