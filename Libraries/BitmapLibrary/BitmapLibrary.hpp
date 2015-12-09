// BitmapLibrary.hpp
// Copyright (c) 2013 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#pragma once

#include <stdint.h>

namespace BitmapLibrary
{
	enum DIBType
	{
		InfoHeaderV1 = 40,
		CoreHeader = 12
	};


	struct __attribute__((packed)) BitmapHeader
	{
		char Signature[2];		// 'B', 'M'
		uint32_t FileSize;
		uint16_t Reserved1;
		uint16_t Reserved2;
		uint32_t BitmapArrayOffset;
	};

	class BitmapDIB
	{
		public:
			BitmapDIB(BitmapDIB* dib, DIBType tp) : Type(tp), DIB(dib){ }
			virtual ~BitmapDIB();

			virtual int32_t GetBitmapWidth(){ return this->DIB->GetBitmapWidth(); }
			virtual int32_t GetBitmapHeight(){ return this->DIB->GetBitmapHeight(); }
			virtual uint32_t GetBitmapSize(){ return this->DIB->GetBitmapSize(); }
			virtual uint32_t GetBitsPerPixel(){ return this->DIB->GetBitsPerPixel(); }

			DIBType Type;

		private:
			BitmapDIB* DIB;
	};

	class BitmapDIB_BitmapInfoHeader : public BitmapDIB
	{
		public:
			BitmapDIB_BitmapInfoHeader(void* bmpptr);

			int32_t GetBitmapWidth();
			int32_t GetBitmapHeight();
			uint32_t GetBitmapSize();
			uint32_t GetBitsPerPixel();

		private:
			uint32_t SizeOfHeader;
			int32_t BitmapWidth;
			int32_t BitmapHeight;
			uint16_t ColourPlanes;	// 1
			uint16_t BitsPerPixel;
			uint32_t CompressionMethod;
			uint32_t BitmapSize;	// Can be zero for uncompressed images.
			int32_t HorizontalResolution;
			int32_t VerticalResolution;
			uint32_t ColoursInPalette;
			uint32_t ImportantColours;
	};

	class BitmapDIB_BitmapCoreHeader : public BitmapDIB
	{
		public:
			BitmapDIB_BitmapCoreHeader(void* bmpptr);

			int32_t GetBitmapWidth();
			int32_t GetBitmapHeight();
			uint32_t GetBitsPerPixel();
			uint32_t GetBitmapSize();

		private:
			uint32_t SizeOfHeader;
			uint16_t BitmapWidth;
			uint16_t BitmapHeight;
			uint16_t ColourPlanes;
			uint16_t BitsPerPixel;
	};


	class __attribute__((packed)) BitmapImage
	{
		public:
			BitmapImage(void* fp);


			BitmapHeader* Header;
			BitmapDIB* DIB;

			void* ActualBMP;

			void RenderSlow(uint16_t x, uint16_t y, void (*r)(uint16_t, uint16_t, uint32_t));
			void Render(uint16_t x, uint16_t y, uint32_t windowwidth, uint32_t* fb);

		private:
			void Render32Slow(uint16_t x, uint16_t y, void (*r)(uint16_t, uint16_t, uint32_t));
			void Render32(uint16_t x, uint16_t y, uint32_t windowwidth, uint32_t* address);

			void Render24(uint16_t x, uint16_t y, void (*r)(uint16_t, uint16_t, uint32_t));
	};
}

















