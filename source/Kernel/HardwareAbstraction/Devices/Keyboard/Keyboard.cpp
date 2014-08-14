// Keyboard.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>

namespace Kernel {
namespace HardwareAbstraction {
namespace Devices
{
	Keyboard::Keyboard(Keyboard* dev)
	{
		this->ActualDevice = dev;
		this->Enabled = false;
	}

	Keyboard::~Keyboard()
	{
	}

	void Keyboard::HandleKeypress()
	{
		this->ActualDevice->HandleKeypress();
	}

	uint8_t Keyboard::ReadBuffer()
	{
		return this->ActualDevice->ReadBuffer();
	}

	void Keyboard::Enable()
	{
		this->Enabled = true;
	}

	void Keyboard::Disable()
	{
		this->Enabled = false;
	}

	bool Keyboard::ItemsInBuffer()
	{
		return this->ActualDevice->ItemsInBuffer();
	}
}
}
}
