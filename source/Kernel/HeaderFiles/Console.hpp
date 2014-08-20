// Console.hpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#pragma once
#include <stdint.h>

namespace Kernel
{
	namespace Console
	{
		void Initialise();
		bool IsInitialised();
		void PrintChar(uint8_t c);
		void ClearScreen();
		void Scroll();
		void ScrollDown(uint64_t lines);
		void Backspace(uint64_t characters);
		void SetColour(uint32_t Colour);
		uint32_t GetColour();
		void MoveCursor(uint16_t x, uint16_t y);
		uint16_t GetCharsPerLine();
		uint16_t GetCharsPerColumn();
		uint16_t GetCharsPerPage();
		uint16_t GetCursorX();
		uint16_t GetCursorY();
	}

}
