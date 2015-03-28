// Console.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.



// Flax rewrite candidate
// required features:
// declaration of function without body
// global variables



#include <Kernel.hpp>
#include <Console.hpp>
#include <HardwareAbstraction/VideoOutput.hpp>
#include <HardwareAbstraction/Devices/IOPort.hpp>
#include <HardwareAbstraction/Devices/SerialPort.hpp>
#include <Memory.hpp>

using namespace Kernel;
using namespace Library;
using namespace Kernel::HardwareAbstraction;
using namespace Kernel::HardwareAbstraction::VideoOutput::LinearFramebuffer;

namespace Kernel {
namespace Console
{
	#define CharWidth		9
	#define CharHeight		16
	#define BitsPerPixel	32

	static uint16_t VT_CursorX = 0;
	static uint16_t VT_CursorY = 0;

	static uint16_t CharsPerLine;
	static uint16_t CharsPerPage;
	static uint16_t CharsPerColumn;

	static uint32_t VT_Colour = 0xFFFFFFFF;
	static bool VT_DidInit = false;
	static bool HasFramebuffer = false;
	static uint16_t OffsetLeft = 4;

	static Mutex* mtx;

	void Initialise()
	{
		HasFramebuffer = (VideoOutput::LinearFramebuffer::GetResX() > 0 && VideoOutput::LinearFramebuffer::GetResY() > 0);

		if(HasFramebuffer)
		{
			CharsPerLine = (GetResX() - OffsetLeft) / CharWidth - 1;
			CharsPerPage = CharsPerLine * (GetResY() / CharHeight) - 1;
			CharsPerColumn = CharsPerPage / CharsPerLine;
		}
		else
		{
			CharsPerLine = 80 - 1;
			CharsPerPage = 80 * 25;
			CharsPerColumn = 25;
		}

		VT_DidInit = true;

		mtx = new Mutex();

		Log("Console Initialised");
	}

	bool IsInitialised()
	{
		return VT_DidInit;
	}

	void PrintChar(uint8_t c)
	{
		if(c == 0)
			return;

		if(SERIALMIRROR){ HardwareAbstraction::Devices::SerialPort::WriteChar(c); }

		if(!IsInitialised() || !HasFramebuffer)
		{
			Kernel::HardwareAbstraction::VideoOutput::Console80x25::PrintChar(c);
			return;
		}

		// AutoMutex lk = AutoMutex(mtx);

		if(c == '\r')
		{
			VT_CursorX = 0;
			return;
		}
		if(c == '\b')
		{
			if(VT_CursorX > 0)
			{
				VT_CursorX--;
				DrawChar(' ', (VT_CursorX * CharWidth) + OffsetLeft, (VT_CursorY * CharHeight), VT_Colour);
			}
			else if(VT_CursorY > 0)
			{
				VT_CursorX = CharsPerLine - 1;
				VT_CursorY--;

				DrawChar(' ', (VT_CursorX * CharWidth) + OffsetLeft, (VT_CursorY * CharHeight), VT_Colour);
			}

			return;
		}

		if(VT_CursorY == CharsPerColumn && VT_CursorX == CharsPerLine)
		{
			// Reached end of line and no more space below
			VT_CursorX = 0;
			ScrollDown(1);



			if(c == '\n' || c == '\t')
			{
				ScrollDown(1);
				VT_CursorX = 0;

				return;
			}
			DrawChar(c, (VT_CursorX * CharWidth) + OffsetLeft, (VT_CursorY * CharHeight), VT_Colour);
			VT_CursorX++;

		}
		else if((VT_CursorX * CharWidth) >= ((GetResX()) - 10))
		{
			// Reached end of line
			VT_CursorX = 0;
			VT_CursorY++;

			DrawChar(c, (VT_CursorX * CharWidth) + OffsetLeft, (VT_CursorY * CharHeight), VT_Colour);
			VT_CursorX = 1;
		}
		else
		{
			if(c == '\n')
			{
				if(VT_CursorY < CharsPerColumn)
				{
					VT_CursorX = 0;
					VT_CursorY++;
				}
				else
				{
					VT_CursorX = 0;
					ScrollDown(1);
				}

				return;
			}
			else if(c == '\t')
			{
				if(((VT_CursorX + 4) & ~(4 - 1)) < CharsPerLine)
				{
					VT_CursorX = (VT_CursorX + 4) & ~(4 - 1);
				}
				else
				{
					VT_CursorX = 0;
					if(VT_CursorY < CharsPerColumn)
						VT_CursorY++;

					else
						ScrollDown(1);
				}

				return;
			}

			// Normal printing
			DrawChar(c, (VT_CursorX * CharWidth) + OffsetLeft, (VT_CursorY * CharHeight), VT_Colour);
			VT_CursorX++;
		}

		return;
	}

	void ClearScreen()
	{
		if(!HasFramebuffer)
		{
			VideoOutput::Console80x25::ClearScreen();
		}
		else
		{
			Memory::Set((uint64_t*) Kernel::GetFramebufferAddress(), 0x00, GetResX() * GetResY() * 4);
			Memory::Set((uint64_t*) Kernel::GetTrueLFBAddress(), 0x00, GetResX() * GetResY() * 4);
			VT_CursorX = 0;
			VT_CursorY = 0;
		}
	}

	void Scroll()
	{
		if(!HasFramebuffer)
		{
			VideoOutput::Console80x25::Scroll();
		}
		else
		{
			uint64_t x = GetResX(), y = GetResY();

			// copy up.
			Memory::Copy((void*) Kernel::GetFramebufferAddress(), (void*)(Kernel::GetFramebufferAddress()
				+ (x * CharHeight * (BitsPerPixel / 8))), (x * y * (BitsPerPixel / 8)) - (x * CharHeight * (BitsPerPixel / 8)));

			// delete the last line.
			Memory::Set((void*)(Kernel::GetFramebufferAddress() + ((x * y * (BitsPerPixel / 8)) - (x * CharHeight * (BitsPerPixel / 8)))),
				0x00, x * CharHeight * (BitsPerPixel / 8));
		}
	}

	void ScrollDown(uint64_t lines)
	{
		while(lines > 0)
		{
			Scroll();
			lines--;
		}
		return;
	}


	void Backspace(uint64_t characters)
	{
		while(characters > 0)
		{
			PrintChar('\b');
			characters--;
		}
	}

	void SetColour(uint32_t Colour)
	{
		VT_Colour = Colour;
	}

	uint32_t GetColour()
	{
		return VT_Colour;
	}


	void MoveCursor(uint16_t x, uint16_t y)
	{
		if(!HasFramebuffer)
		{
			VideoOutput::Console80x25::MoveCursor((uint8_t) x, (uint8_t) y);
		}
		else
		{
			if(x <= CharsPerLine && y <= CharsPerColumn)
			{
				AutoMutex lk = AutoMutex(mtx);
				VT_CursorX = x;
				VT_CursorY = y;
			}
		}
	}

	uint16_t GetCharsPerLine()
	{
		return CharsPerLine;
	}

	uint16_t GetCharsPerColumn()
	{
		return CharsPerColumn;
	}

	uint16_t GetCharsPerPage()
	{
		return CharsPerPage;
	}

	uint16_t GetCursorX()
	{
		if(!HasFramebuffer)
			return VideoOutput::Console80x25::GetCursorX();

		else
			return VT_CursorX;
	}
	uint16_t GetCursorY()
	{
		if(!HasFramebuffer)
			return VideoOutput::Console80x25::GetCursorY();

		else
			return VT_CursorY;
	}
}
}

