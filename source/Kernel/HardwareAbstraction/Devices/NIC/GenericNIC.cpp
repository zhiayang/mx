// GenericNIC.cpp
// Copyright (c) 2013 - 2016, zhiayang@gmail.com
// Licensed under Creative Commons 3.0 Unported.

#include <Kernel.hpp>

namespace Kernel {
namespace HardwareAbstraction {
namespace Devices {
namespace NIC
{
	GenericNIC::~GenericNIC()
	{

	}

	GenericNIC::GenericNIC()
	{
		memset(this->MAC, 0, 6);
		this->pcidev = 0;
	}
}
}
}
}
