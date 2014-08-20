// LinearFramebuffer.hpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under Creative Commons Attribution ShareAlike 3.0 Unported.


#include <stdint.h>
#pragma once


namespace Kernel {
namespace HardwareAbstraction {
namespace VideoOutput
{
	namespace LinearFramebuffer
	{
		extern const uint8_t Font8x16_Thin[256][16];
		extern const uint8_t Font8x16_Thick[256][16];
		extern const uint8_t Font8x16_Liberation[256][16];
		extern uint32_t BackColour;
		extern uint32_t FrontColour;

		class DirtyRegion
		{
			public:
				DirtyRegion(uint64_t X, uint64_t Y, uint64_t w, uint64_t h) : x(X), y(Y), Width(w), Height(h){ }

				uint64_t GetX(){ return this->x; }
				uint64_t GetY(){ return this->y; }
				uint64_t GetWidth(){ return this->Width; }
				uint64_t GetHeight(){ return this->Height; }

			private:
				uint64_t x, y, Width, Height;
		};



		void InsertDirtyRegion(DirtyRegion* r);
		void Initialise();
		void PutPixelAtAddr(uint32_t pos, uint32_t Colour);
		void PutPixel(uint16_t x, uint16_t y, uint32_t Colour = 0xFFFFFFFF);
		uint32_t BlendPixels(uint32_t bottom, uint32_t top);
		uint32_t GetRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
		void DrawChar(uint8_t c, uint16_t x, uint16_t y, uint32_t Colour);
		void RefreshBuffer();
		uint16_t GetResX();
		uint16_t GetResY();
	}
}
}
}
