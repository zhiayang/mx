// Multitasking.hpp
// Copyright (c) 2014 - 2016, zhiayang@gmail.com
// Licensed under Creative Commons Attribution ShareAlike 3.0 Unported.


#include <stdint.h>
#pragma once

namespace Kernel {
namespace HardwareAbstraction {
namespace VideoOutput
{
	namespace Console80x25
	{
		void Initialise();
		void ClearScreen(uint8_t Colour = 0x00);
		void PrintChar(uint8_t Char, uint32_t Colour);
		void PrintChar(uint8_t Char);
		void Scroll();

		void MoveCursor(uint16_t x, uint16_t y);
		uint16_t GetCursorX();
		uint16_t GetCursorY();
	}
}
}
}
