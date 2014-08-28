// main.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

// #include <algorithm>


// void operator delete(void* p) noexcept
// {
// 	free(p);
// }

// void operator delete[](void* p) noexcept
// {
// 	free(p);
// }

// void* operator new(unsigned long size)
// {
// 	printf("[%d]", size);
// 	return (void*) malloc(size);
// }

// void* operator new[](unsigned long size)
// {
// 	printf("\narr: [%d]\n", size);
// 	return (void*) malloc(size);
// }



// int main()
// {
// 	printf("hello, world\n");
// 	// printf("reading file\n");


// 	// png_structp png_ptr;
// 	// png_infop info_ptr;

// 	// uint8_t sig[8];
// 	// FILE* infile = fopen("/logo.png", "");
// 	// fread(&sig, 1, 8, infile);
// 	// printf("file read\n");

// 	// if(!png_check_sig(sig, 8))
// 	// {
// 	// 	printf("invalid png file\n");
// 	// 	return 1;
// 	// }

// 	// printf("png file verified\n");

// 	// png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
// 	// if(!png_ptr)
// 	// {
// 	// 	printf("out of memory\n");
// 	// 	return 1;
// 	// }

// 	// info_ptr = png_create_info_struct(png_ptr);
// 	// if(!info_ptr)
// 	// {
// 	// 	png_destroy_read_struct(&png_ptr, NULL, NULL);
// 	// 	printf("out of memory");
// 	// 	return 1;
// 	// }

// 	// printf("structs created\n");

// 	// png_init_io(png_ptr, infile);
// 	// png_set_sig_bytes(png_ptr, 8);
// 	// png_read_info(png_ptr, info_ptr);
// 	// printf("info read\n");

// 	// uint32_t width = 0;
// 	// uint32_t height = 0;
// 	// auto depth = 0;
// 	// auto ctype = 0;
// 	// png_get_IHDR(png_ptr, info_ptr, &width, &height, &depth, &ctype, NULL, NULL, NULL);

// 	// printf("info header got. dimensions: %dx%d:%d", width, height, depth);
// 	// size_t  i, rowbytes;
// 	// // uint8_t* row_pointers[height];

// 	// uint8_t** row_pointers = (uint8_t**) malloc(height * sizeof(uint8_t*));

// 	// uint8_t* image_data = nullptr;

// 	// png_read_update_info(png_ptr, info_ptr);

// 	// rowbytes = png_get_rowbytes(png_ptr, info_ptr);
// 	// // *pChannels = (int)png_get_channels(png_ptr, info_ptr);

// 	// if((image_data = (uint8_t*) malloc(rowbytes * height)) == NULL)
// 	// 	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

// 	// for(i = 0; i < height; i++)
// 	// 	row_pointers[i] = image_data + i * rowbytes;


// 	// if(ctype == PNG_COLOR_TYPE_PALETTE)
// 	// {
// 	// 	png_set_palette_to_rgb(png_ptr);
// 	// }
// 	// if(ctype == PNG_COLOR_TYPE_GRAY && depth < 8)
// 	// {
// 	// 	printf("fail");
// 	// 	return 0;
// 	// }
// 	// if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
// 	// {
// 	// 	png_set_tRNS_to_alpha(png_ptr);
// 	// }

// 	// if(depth == 16)
// 	// 	png_set_strip_16(png_ptr);

// 	// if(ctype == PNG_COLOR_TYPE_GRAY || ctype == PNG_COLOR_TYPE_GRAY_ALPHA)
// 	// 	png_set_gray_to_rgb(png_ptr);


// 	// // flip the colours
// 	// png_set_bgr(png_ptr);
// 	// assert("ENOSUP" && ctype == PNG_COLOR_TYPE_RGBA);
// 	// png_read_image(png_ptr, row_pointers);
// 	// printf("done\n\n");



// 	// size_t x = 0;
// 	// size_t y = 0;
// 	// size_t fw = 1024;
// 	// uint64_t fb = 0xFD000000;
// 	// uint8_t* ptr = image_data;


// 	// for(size_t d = 0; d < height; d++)
// 	// {
// 	// 	uint64_t pos = (y * fw + x) * 4;
// 	// 	memcpy((void*)((uint64_t) fb + pos), (void*) ptr, width * 4);

// 	// 	ptr += 4 * width;

// 	// 	y++;
// 	// }

// 	// printf("\n\n\n\n\n\n\n\n\n\n\n\n");
// 	printf("hi");

// 	// uint8_t* ptr = (uint8_t*) malloc(1024);
// 	uint8_t* ptr = new uint8_t[1024];
// 	memset(ptr, 0xFF, 1024);

// 	// std::vector<int>* vec = new std::vector<int>();
// 	// for(int v = 0; v < 4000; v += 100)
// 	// 	vec->push_back(v);

// 	// for(auto v : *vec)
// 	// 	printf("[%d]", v);

// 	printf("done");

// 	return 0;
// }



#include <vector>

int main()
{
	// uint8_t* ptr = new uint8_t[1024];
	// memset(ptr, 0xAB, 1024);

	// printf("%x", ptr);

	std::vector<int> vec;
	for(int v = 0; v < 4000; v += 100)
		vec.push_back(v);

	for(auto v : vec)
		printf("[%d]", v);

	printf("done");
	return 0;
}

















