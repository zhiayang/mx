// BitmapLibrary.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.



#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "BitmapLibrary.hpp"


extern void operator delete(void* p);
extern void operator delete[](void* p);
extern void* operator new(unsigned long size);
extern void* operator new[](unsigned long size);
extern void* operator new(unsigned long, void* addr);

namespace BitmapLibrary
{
	BitmapImage::BitmapImage(void* Fileptr)
	{

		// uint8_t* s = (uint8_t*) Fileptr;

		this->Header = new BitmapHeader;

		// this->Header->Signature[0] = *((uint8_t*)((uint64_t) Fileptr + 0));
		// this->Header->Signature[1] = *((uint8_t*)((uint64_t) Fileptr + 1));
		// this->Header->FileSize = *((uint32_t*)((uint64_t) Fileptr + 2));

		// this->Header->Reserved1 = *((uint16_t*)((uint64_t) Fileptr + 6));
		// this->Header->Reserved2 = *((uint16_t*)((uint64_t) Fileptr + 8));
		// this->Header->BitmapArrayOffset = *((uint32_t*)((uint64_t) Fileptr + 10));

		memcpy((void*) this->Header, Fileptr, 14);






		this->ActualBMP = new uint8_t[this->Header->FileSize];
		memcpy(this->ActualBMP, (void*) Fileptr, this->Header->FileSize);

		// check the DIBType.
		switch(*((uint32_t*)((uint64_t) Fileptr + 14)))
		{
			case 40:
				this->DIB = new BitmapDIB_BitmapInfoHeader((void*) this->ActualBMP);
				break;

			case 12:
				this->DIB = new BitmapDIB_BitmapCoreHeader((void*) this->ActualBMP);
				break;

			case 108:
				// BITMAPINFOHEADERV4
				break;

			case 124:
				// BITMAPINFOHEADERV5
				break;

			default:
				// not supported.
				break;
		}
	}

	void BitmapImage::RenderSlow(uint16_t x, uint16_t y, void (*r)(uint16_t, uint16_t, uint32_t))
	{
		switch(this->DIB->GetBitsPerPixel())
		{
			case 32:
				this->Render32Slow(x, y, r);
				break;

			case 24:
				this->Render24(x, y, r);
				break;

			default:
				break;
		}
	}

	void BitmapImage::Render(uint16_t x, uint16_t y, uint32_t fw, uint32_t* fb)
	{
		if(this->DIB->GetBitsPerPixel() == 32)
			this->Render32(x, y, fw, fb);
	}

	void BitmapImage::Render32Slow(uint16_t x, uint16_t y, void (*r)(uint16_t, uint16_t, uint32_t))
	{
		uint32_t imght = (uint32_t) __abs(this->DIB->GetBitmapHeight());
		uint64_t ptr = (uint64_t) this->ActualBMP + this->Header->BitmapArrayOffset;

		uint64_t rowsize = ((this->DIB->GetBitsPerPixel() * (uint64_t) this->DIB->GetBitmapWidth() + 31) / 32) * 4;

		uint16_t oposx = x;
		uint16_t posx = oposx;
		uint16_t posy = y + (this->DIB->Type == InfoHeaderV1 && (this->DIB->GetBitmapHeight() < 0 ? 0 : this->DIB->GetBitmapHeight()));

		for(uint32_t d = 0; d < imght; d++)
		{
			for(uint32_t w = 0; w < rowsize / (this->DIB->GetBitsPerPixel() / 8); w++)
			{
				r(posx, posy, *((uint32_t*) ptr));
				ptr += this->DIB->GetBitsPerPixel() / 8;
				posx++;
			}
			posx = oposx;
			posy++;
		}
	}

	void BitmapImage::Render24(uint16_t x, uint16_t y, void (*r)(uint16_t, uint16_t, uint32_t))
	{
		uint32_t imght = (uint32_t) __abs(this->DIB->GetBitmapHeight());
		uint64_t ptr = (uint64_t) this->ActualBMP + this->Header->BitmapArrayOffset;

		uint64_t rowsize = ((this->DIB->GetBitsPerPixel() * (uint64_t) this->DIB->GetBitmapWidth() + 31) / 32) * 4;

		uint16_t oposx = x;
		uint16_t posx = oposx;
		uint16_t posy = y + (this->DIB->Type == InfoHeaderV1 && (this->DIB->GetBitmapHeight() < 0 ? 0 : this->DIB->GetBitmapHeight()));


		for(uint32_t d = 0; d < imght; d++)
		{
			for(uint32_t w = 0; w < rowsize / (this->DIB->GetBitsPerPixel() / 8); w++)
			{
				r(posx, posy, (*((uint32_t*) ptr) & 0xFFFFFF));
				ptr += this->DIB->GetBitsPerPixel() / 8;
				posx++;
			}
			posx = oposx;
			posy--;
		}
	}




	void BitmapImage::Render32(uint16_t x, uint16_t y, uint32_t fw, uint32_t* fb)
	{
		uint32_t imght = (uint32_t) __abs(this->DIB->GetBitmapHeight());
		uint64_t ptr = (uint64_t) this->ActualBMP + this->Header->BitmapArrayOffset;

		uint64_t rowsize = ((this->DIB->GetBitsPerPixel() * (uint64_t) this->DIB->GetBitmapWidth() + 31) / 32) * 4;


		// make sure the image will fit.
		if(rowsize > fw * (32 / 8))
			return;

		for(uint32_t d = 0; d < imght; d++)
		{
			uint64_t pos = (y * fw + x) * 4;
			memcpy((void*)((uint64_t) fb + pos), (void*) ptr, rowsize);

			ptr += (this->DIB->GetBitsPerPixel() / 8) * (rowsize / (this->DIB->GetBitsPerPixel() / 8));

			y++;
		}
	}



	BitmapDIB_BitmapInfoHeader::BitmapDIB_BitmapInfoHeader(void* bmpptr) : BitmapDIB(this, InfoHeaderV1)
	{
		uint64_t pt = (uint64_t) bmpptr;

		// init fields manually.
		this->SizeOfHeader = *((uint32_t*)(pt + 0x0E));
		this->BitmapWidth = *((int32_t*)(pt + 0x12));
		this->BitmapHeight = *((int32_t*)(pt + 0x16));
		this->ColourPlanes = *((uint16_t*)(pt + 0x1A));
		this->BitsPerPixel = *((uint16_t*)(pt + 0x1C));
		this->CompressionMethod = *((uint32_t*)(pt + 0x1E));
		this->BitmapSize = *((uint32_t*)(pt + 0x22));
		this->HorizontalResolution = *((int32_t*)(pt + 0x26));
		this->VerticalResolution = *((int32_t*)(pt + 0x2A));
		this->ColoursInPalette = *((uint32_t*)(pt + 0x2E));
		this->ImportantColours = *((uint32_t*)(pt + 0x32));
	}

	BitmapDIB_BitmapCoreHeader::BitmapDIB_BitmapCoreHeader(void* bmpptr) : BitmapDIB(this, CoreHeader)
	{
		this->SizeOfHeader = *((uint32_t*)((uint64_t) bmpptr + 0x0E));
		this->BitmapWidth = *((uint16_t*)((uint64_t) bmpptr + 0x12));
		this->BitmapHeight = *((uint16_t*)((uint64_t) bmpptr + 0x14));
		this->ColourPlanes = *((uint16_t*)((uint64_t) bmpptr + 0x16));
		this->BitsPerPixel = *((uint16_t*)((uint64_t) bmpptr + 0x18));
	}


	int32_t BitmapDIB_BitmapInfoHeader::GetBitmapWidth(){ return this->BitmapWidth; }
	int32_t BitmapDIB_BitmapInfoHeader::GetBitmapHeight(){ return this->BitmapHeight; }
	uint32_t BitmapDIB_BitmapInfoHeader::GetBitmapSize(){ return this->BitmapSize; }
	uint32_t BitmapDIB_BitmapInfoHeader::GetBitsPerPixel(){ return this->BitsPerPixel; }


	int32_t BitmapDIB_BitmapCoreHeader::GetBitmapWidth(){ return (int32_t) this->BitmapWidth; }
	int32_t BitmapDIB_BitmapCoreHeader::GetBitmapHeight(){ return (int32_t) this->BitmapHeight; }
	uint32_t BitmapDIB_BitmapCoreHeader::GetBitsPerPixel(){ return (uint32_t) this->BitsPerPixel; }
	uint32_t BitmapDIB_BitmapCoreHeader::GetBitmapSize()
	{
		return ((this->BitsPerPixel * this->BitmapWidth + 31) / 32) * 4 * (uint32_t)(this->BitmapHeight < 0 ? this->BitmapHeight * -1 : this->BitmapHeight);
	}

	BitmapDIB::~BitmapDIB()
	{
	}
}







