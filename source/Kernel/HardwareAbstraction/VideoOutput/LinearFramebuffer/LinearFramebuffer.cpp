// LinearFrameBuffer.cpp
// Copyright (c) 2013 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <HardwareAbstraction/VideoOutput.hpp>
#include <Console.hpp>
#include <Memory.hpp>

using namespace Library;
using namespace Kernel::HardwareAbstraction::Devices;

namespace Kernel {
namespace HardwareAbstraction {
namespace VideoOutput {
namespace LinearFramebuffer
{
	#define _RED(color) ((color & 0x00FF0000) / 0x10000)
	#define _GRE(color) ((color & 0x0000FF00) / 0x100)
	#define _BLU(color) ((color & 0x000000FF) / 0x1)
	#define _ALP(color) ((color & 0xFF000000) / 0x1000000)

	static uint16_t ResX = 0;
	static uint16_t ResY = 0;

	uint32_t backColour = 0;
	uint32_t frontColour = 0xFFFFFFFF;

	// #define BytesPerPixel		4
	#define CharHeight			16
	#define CharWidth			9

	#define CurrentFont			Font8x16_Thick


	static void* memset32(void* dst, uint32_t val, uint64_t len)
	{
		uintptr_t d0 = 0;
		asm volatile(
			"rep stosl"
			:"=&D" (d0), "+&c" (len)
			:"0" (dst), "a" (val)
			:"memory");

		return dst;
	}

	void Initialise()
	{
		GenericVideoDevice* vid = (GenericVideoDevice*) DeviceManager::GetDevice(DeviceType::FramebufferVideoCard);
		assert(vid);

		ResX = vid->GetResX();
		ResY = vid->GetResY();
	}

	void PutPixelAtAddr(uint32_t pos, uint32_t Colour)
	{
		// For safety's sake, we'd better keep this internal
		*((uint32_t*) (Kernel::GetFramebufferAddress() + pos)) = Colour;
	}


	void PutPixel(uint16_t x, uint16_t y, uint32_t Colour)
	{
		if(x < GetResX() && y < GetResY())
		{
			// Index = (x * width) + y
			uint32_t pos = y * GetResX() + x;
			pos *= 4;	// 4 bytes per pixel

			PutPixelAtAddr(pos, Colour);
		}
	}

	uint32_t BlendPixels(uint32_t bottom, uint32_t top)
	{
		uint8_t a = _ALP(top);
		uint8_t b = (uint8_t)(((uint32_t)_ALP(bottom) * (255 - a)) / 255);
		uint8_t alp = a + b;
		uint8_t red = alp ? (uint8_t)(uint32_t)((_RED(bottom) * b + _RED(top) * a) / alp) : 0;
		uint8_t gre = alp ? (uint8_t)(uint32_t)((_GRE(bottom) * b + _GRE(top) * a) / alp) : 0;
		uint8_t blu = alp ? (uint8_t)(uint32_t)((_BLU(bottom) * b + _BLU(top) * a) / alp) : 0;
		return GetRGBA(red, gre, blu, alp);
	}

	uint32_t GetRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	{
		return (a * 0x1000000) + (r * 0x10000) + (g * 0x100) + (b * 0x1);
	}

	void DrawChar(uint8_t c, uint16_t x, uint16_t y, uint32_t Colour)
	{
		if(!Console::IsInitialised())
			return;

		if(!c)
			return;

		if(c == ' ')
		{
			for(uint64_t i = y; i < (uint64_t) (y + CharHeight); i++)
				memset32((void*) (Kernel::GetFramebufferAddress() + (i * GetResX() + x) * 4), backColour, 4);
		}

		uint32_t* rowAddress = 0;

		rowAddress = (uint32_t *) Kernel::GetFramebufferAddress() + y * GetResX() + x;
		for(int row = 0; row < CharHeight; row++)
		{
			uint8_t data = CurrentFont[c][row];
			uint32_t* columnAddress = rowAddress;

			int col = 0;
			for(data = CurrentFont[c][row]; col < CharWidth; data <<= 1, col++)
			{
				if((data & 0x80) != 0)
				{
					*columnAddress = Colour;
				}
				else
				{
					*columnAddress = 0;
				}
				columnAddress++;
			}
			rowAddress += GetResX();
		}
	}

	void RefreshBuffer()
	{

	}

	uint16_t GetResX()
	{
		return ResX;
	}

	uint16_t GetResY()
	{
		return ResY;
	}
}
}
}
}
