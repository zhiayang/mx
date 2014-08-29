// main.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

void putpixel(int x, int y)
{
	uint64_t pos = y * 1024 + x;
	pos *= 4;	// 4 bytes per pixel

	*((uint32_t*) (0xFD000000 + pos)) = 0xFFFFFFFF;
}

void printchar(stbtt_fontinfo* font, int x, int y, int size, char c)
{
	unsigned char *bitmap;
	int w = 0;
	int h = 0;

	bitmap = stbtt_GetCodepointBitmap(font, 0, stbtt_ScaleForPixelHeight(font, size), c, &w, &h, 0,0);

	for(int i = 0; i < w; i++)
	{
		for(int j = 0; j < h; j++)
		{
			if(bitmap[w * j + i])
				putpixel(x + i, y + j);
		}
	}

	stbtt_FreeBitmap(bitmap, NULL);
}

int main()
{
	stbtt_fontinfo font;
	void* buffer = malloc(500000);
	fread(buffer, 1, 475160, fopen("/menlo.ttf", "rb"));
	stbtt_InitFont(&font, (const uint8_t*) buffer, stbtt_GetFontOffsetForIndex((const uint8_t*) buffer, 0));

	printchar(&font, 100, 300, 100, 'F');
	printchar(&font, 150, 300, 100, 'U');
	printchar(&font, 200, 300, 100, 'C');
	printchar(&font, 250, 300, 100, 'K');
	printchar(&font, 300, 300, 100, 'T');
	printchar(&font, 350, 300, 100, 'H');
	printchar(&font, 400, 300, 100, 'I');
	printchar(&font, 450, 300, 100, 'S');
	printchar(&font, 500, 300, 100, 'S');
	printchar(&font, 550, 300, 100, 'H');
	printchar(&font, 600, 300, 100, 'I');
	printchar(&font, 650, 300, 100, 'T');


	return 0;
}

















