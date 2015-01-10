// GenericNIC.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under Creative Commons 3.0 Unported.

#include <Kernel.hpp>

namespace Kernel {
namespace HardwareAbstraction {
namespace Devices {
namespace NIC
{
	GenericNIC::GenericNIC(GenericNIC* dev)
	{
		this->device = dev;
	}

	GenericNIC::~GenericNIC()
	{
	}

	void GenericNIC::Reset()
	{
		this->device->Reset();
	}

	void GenericNIC::SendData(uint8_t *data, uint64_t bytes)
	{
		this->device->SendData(data, bytes);
	}

	uint64_t GenericNIC::GetHardwareType()
	{
		return this->device->GetHardwareType();
	}

	uint8_t* GenericNIC::ReceiveData(uint8_t *data, uint64_t bytes)
	{
		return this->device->ReceiveData(data, bytes);
	}

	void GenericNIC::HandleInterrupt()
	{
		this->device->HandleInterrupt();
	}

	uint8_t* GenericNIC::GetMAC()
	{
		return this->device->MAC;
	}
}
}
}
}
