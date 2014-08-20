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
	static uint8_t CursorX, CursorY;
	const uint32_t Space = ((uint16_t)(' ') | (0x0F << 8) | (((uint32_t)(uint16_t)(' ') | (0x0F << 8)) << 16));
	const uint8_t TabWidth = 4;


	void ScrollUp();

	void Initialise()
	{
		// Hide the stupid cursor
		using namespace Kernel::HardwareAbstraction::Devices::IOPort;
		Write16(0x3D4, 0x200A);
		Write16(0x3D4, 0xB);

		ClearScreen(0x00);
	}

	void ClearScreen(uint8_t Colour)
	{
		using namespace Library::Memory;
		uint32_t s = (uint16_t)((' ') | (Colour << 8));

		for(int i = 0; i < 25; i++)
			Set32((uint16_t*)0xB8000 + i * 80, s, 80);

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
			Location = (uint16_t*)0xB8000 + (CursorY * 80 + CursorX);
			*Location = (uint16_t)(Char | (Colour << 8));
			CursorX++;
		}

		if(CursorX >= 80)
		{
			CursorX = 0;
			CursorY++;
		}

		ScrollUp();
	}

	void ScrollUp()
	{
		if(CursorY >= 25)
		{
			using namespace Library::Memory;
			Copy((uint16_t*)0xB8000, (uint16_t*)0xB8000 + (CursorY - 24) * 80, (49 - CursorY) * 80 * 2);
			Set32((uint16_t*)0xB8000 + (49 - CursorY) * 80, Space, 40);
			CursorY = 24;
		}
	}
}
}
}
}
