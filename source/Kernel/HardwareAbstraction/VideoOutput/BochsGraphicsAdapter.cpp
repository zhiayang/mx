// BochsGraphicsAdapter.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <HardwareAbstraction/Devices/IOPort.hpp>
#include <Memory.hpp>

using namespace Kernel::HardwareAbstraction::Devices;

namespace Kernel {
namespace HardwareAbstraction {
namespace VideoOutput
{
	static const uint16_t VBE_DISPI_IOPORT_INDEX		= 0x01CE;
	static const uint16_t VBE_DISPI_IOPORT_DATA			= 0x01CF;
	static const uint16_t VBE_DISPI_INDEX_ID			= 0x0;
	static const uint16_t VBE_DISPI_INDEX_XRES			= 0x1;
	static const uint16_t VBE_DISPI_INDEX_YRES			= 0x2;
	static const uint16_t VBE_DISPI_INDEX_BPP			= 0x3;
	static const uint16_t VBE_DISPI_INDEX_ENABLE		= 0x4;

	static const uint16_t VBE_DISPI_DISABLED			= 0x00;
	static const uint16_t VBE_DISPI_LFB_ENABLED			= 0x40;


	BochsGraphicsAdapter::BochsGraphicsAdapter(PCI::PCIDevice* _dev)
	{
		this->pcidev = _dev;

		// check if this version is usable.
		uint16_t ver = (uint16_t) this->ReadRegister(VBE_DISPI_INDEX_ID);
		if(ver < 0xB0C2)
		{
			Log(1, "Bogus BGA Version: expected 0xB0C2 or more, got %4x", ver);
			Log(1, "Continuing anyway...");
		}
	}


	void BochsGraphicsAdapter::WriteRegister(uint64_t i, uint64_t v)
	{
		IOPort::Write16(VBE_DISPI_IOPORT_INDEX, (uint16_t) i);
		IOPort::Write16(VBE_DISPI_IOPORT_DATA, (uint16_t) v);
	}

	uint64_t BochsGraphicsAdapter::ReadRegister(uint64_t i)
	{
		IOPort::Write16(VBE_DISPI_IOPORT_INDEX, (uint16_t) i);
		return IOPort::Read16(VBE_DISPI_IOPORT_DATA);
	}

	void BochsGraphicsAdapter::SetMode(uint16_t w, uint16_t h, uint16_t bd)
	{
		this->WriteRegister(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
		this->WriteRegister(VBE_DISPI_INDEX_XRES, w);
		this->WriteRegister(VBE_DISPI_INDEX_YRES, h);
		this->WriteRegister(VBE_DISPI_INDEX_BPP, bd);
		this->WriteRegister(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_LFB_ENABLED | 1);
	}

	uint16_t BochsGraphicsAdapter::GetResX()
	{
		return (uint16_t) this->ReadRegister(VBE_DISPI_INDEX_XRES);
	}

	uint16_t BochsGraphicsAdapter::GetResY()
	{
		return (uint16_t) this->ReadRegister(VBE_DISPI_INDEX_YRES);
	}

	uint16_t BochsGraphicsAdapter::GetBitDepth()
	{
		return (uint16_t) this->ReadRegister(VBE_DISPI_INDEX_BPP);
	}

	uint64_t BochsGraphicsAdapter::GetFramebufferAddress()
	{
		return this->pcidev->GetBAR(0) & 0xFFFFFFF0;
	}
}
}
}

