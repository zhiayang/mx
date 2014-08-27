// main.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include <string.h>


static png_structp png_ptr;
static png_infop info_ptr;


int main()
{
	printf("hello, world\n");
	printf("reading file\n");

	uint8_t sig[8];
	FILE* infile = fopen("/logo.png", "");
	fread(&sig, 1, 8, infile);
	printf("file read\n");

	if(!png_check_sig(sig, 8))
	{
		printf("invalid png file\n");
		return 1;
	}

	printf("png file verified\n");

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png_ptr)
	{
		printf("out of memory\n");
		return 1;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr)
	{
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		printf("out of memory");
		return 1;
	}

	printf("structs created\n");

	png_init_io(png_ptr, infile);
	png_set_sig_bytes(png_ptr, 8);
	png_read_info(png_ptr, info_ptr);
	printf("info read\n");

	uint32_t width = 0;
	uint32_t height = 0;
	auto depth = 0;
	auto ctype = 0;
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &depth, &ctype, NULL, NULL, NULL);

	printf("info header got. dimensions: %dx%d:%d", width, height, depth);

	return 0;
}
