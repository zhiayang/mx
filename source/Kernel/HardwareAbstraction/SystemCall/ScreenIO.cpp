// ScreenIO.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <Console.hpp>
#include <HardwareAbstraction/VideoOutput.hpp>

namespace Kernel {
namespace HardwareAbstraction {
namespace SystemCalls
{
	extern "C" void Syscall_PrintChar(uint8_t c)
	{
		Kernel::Console::PrintChar(c);
	}

	extern "C" void Syscall_PrintString(const char* s)
	{
		Library::StandardIO::PrintString(s);
	}

	extern "C" uint64_t Syscall_GetFramebufferAddress()
	{
		return Kernel::GetFramebufferAddress();
	}

	extern "C" void Syscall_SetConsoleBackColour(uint64_t c)
	{
		Kernel::HardwareAbstraction::VideoOutput::LinearFramebuffer::BackColour = (uint32_t) c;
	}

	extern "C" uint64_t Syscall_GetFramebufferResX()
	{
		return Kernel::HardwareAbstraction::VideoOutput::LinearFramebuffer::GetResX();
	}

	extern "C" uint64_t Syscall_GetFramebufferResY()
	{
		return Kernel::HardwareAbstraction::VideoOutput::LinearFramebuffer::GetResY();
	}

	extern "C" uint64_t Syscall_GetConsoleBackColour()
	{
		return Kernel::HardwareAbstraction::VideoOutput::LinearFramebuffer::BackColour;
	}

	extern "C" uint64_t Syscall_GetConsoleTextColour()
	{
		return Kernel::Console::GetColour();
	}

	extern "C" void Syscall_SetConsoleTextColour(uint64_t c)
	{
		return Kernel::Console::SetColour((uint32_t) c);
	}

	extern "C" void Syscall_ClearScreen()
	{
		Kernel::Console::ClearScreen();
	}


	extern "C" uint64_t Syscall_GetCursorX()
	{
		return Console::GetCursorX();
	}

	extern "C" uint64_t Syscall_GetCursorY()
	{
		return Console::GetCursorY();
	}

	extern "C" void Syscall_SetCursorPos(uint64_t x, uint64_t y)
	{
		Console::MoveCursor((uint16_t) x, (uint16_t) y);
	}

	extern "C" uint64_t Syscall_GetCharWidth()
	{
		return 8;
	}

	extern "C" uint64_t Syscall_GetCharHeight()
	{
		return 16;
	}

	extern "C" void Syscall_PutPixelAtXY(uint64_t x, uint64_t y, uint64_t Colour)
	{
		HardwareAbstraction::VideoOutput::LinearFramebuffer::PutPixel((uint16_t) x, (uint16_t) y, (uint32_t) Colour);
	}

	extern "C" void Syscall_PutCharNoMove(uint8_t c)
	{
		HardwareAbstraction::VideoOutput::LinearFramebuffer::DrawChar(c, Console::GetCursorX() * 8 + 4, Console::GetCursorY() * 16, Console::GetColour());
	}
}
}
}
