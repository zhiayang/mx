// GenericNIC.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under Creative Commons 3.0 Unported.

#include <Kernel.hpp>

namespace Kernel {
namespace HardwareAbstraction {
namespace Devices {
namespace NIC
{
	GenericNIC::GenericNIC()
	{

	}

	GenericNIC::~GenericNIC()
	{
	}

	void GenericNIC::Reset()
	{

	}

	void GenericNIC::SendData(uint8_t* data, uint64_t bytes)
	{
		(void) data;
		(void) bytes;
	}

	uint64_t GenericNIC::GetHardwareType()
	{
		return 0;
	}

	uint8_t* GenericNIC::GetMAC()
	{
		return 0;
	}

	void GenericNIC::HandleInterrupt()
	{
	}
}
}
}
}
