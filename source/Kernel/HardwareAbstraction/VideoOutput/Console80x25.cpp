// Console80x25.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <HardwareAbstraction/Devices/IOPort.hpp>
#include <HardwareAbstraction/VideoOutput/Console80x25.hpp>
#include <Memory.hpp>


namespace Kernel {
namespace HardwareAbstraction {
namespace VideoOutput {
namespace Console80x25
{
	static uint16_t CursorX = 0;
	static uint16_t CursorY = 0;

	const uint32_t Space = ((uint16_t)(' ') | (0x0F << 8) | (((uint32_t)(uint16_t)(' ') | (0x0F << 8)) << 16));
	const uint8_t TabWidth = 4;

	void Initialise()
	{
		// Hide the stupid cursor
		using namespace Kernel::HardwareAbstraction::Devices::IOPort;
		Write16(0x3D4, 0x200A);
		Write16(0x3D4, 0xB);

		// ClearScreen(0x00);
	}

	static void* memset32(void* dst, uint32_t val, uint64_t len)
	{
		uintptr_t d0 = 0;
		uint64_t uval = ((uint64_t) val << 32) + val;
		asm volatile(
			"rep stosq"
			:"=&D" (d0), "+&c" (len)
			:"0" (dst), "a" (uval)
			:"memory");

		return dst;
	}

	void ClearScreen(uint8_t Colour)
	{
		uint32_t s = (uint16_t)((' ') | (Colour << 8));

		for(int i = 0; i < 25; i++)
			memset32((uint16_t*)0xB8000 + i * 80, s, 80);

		CursorX = 0;
		CursorY = 0;
	}

	void PrintChar(uint8_t Char)
	{
		PrintChar(Char, 0x0F);
	}

	void PrintChar(uint8_t Char, uint32_t Colour)
	{
		uint16_t* Location;

		if(Char == '\b')
		{
			if(CursorX != 0)
			{
				CursorX--;
				PrintChar(' ');
				CursorX--;
			}
			else if(CursorY > 0)
			{
				CursorX = 79;
				PrintChar(' ');
				CursorX = 79;
			}
		}

		else if(Char == '\t')
		{
			CursorX = (CursorX + TabWidth) & ~(TabWidth - 1);
		}

		else if(Char == '\r')
		{
			CursorX = 0;
		}

		else if(Char == '\n')
		{
			CursorX = 0;
			CursorY++;
		}

		else if(Char >= ' ')
		{
			Location = (uint16_t*) 0xB8000 + (CursorY * 80 + CursorX);
			*Location = (uint16_t)(Char | (Colour << 8));
			CursorX++;
		}

		if(CursorX >= 80)
		{
			CursorX = 0;
			CursorY++;
		}

		Scroll();
	}

	void Scroll()
	{
		if(CursorY >= 25)
		{
			Memory::Copy((uint16_t*) 0xB8000, (uint16_t*) 0xB8000 + (CursorY - 24) * 80, (49 - CursorY) * 80 * 2);
			memset32((uint16_t*) 0xB8000 + (49 - CursorY) * 80, Space, 40);
			CursorY = 24;
		}
	}

	void MoveCursor(uint16_t x, uint16_t y)
	{
		CursorX = x;
		CursorY = y;
	}

	uint16_t GetCursorX()
	{
		return CursorX;
	}

	uint16_t GetCursorY()
	{
		return CursorY;
	}
}
}
}
}













