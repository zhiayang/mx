// main.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <pthread.h>

static uint64_t framebuffer = 0;
static uint64_t width = 0;
static uint64_t height = 0;
static uint64_t bpp = 0;

int main(int argc, char** argv)
{
	assert(argc == 5);
	/*
		1. framebuffer address
		2. framebuffer width
		3. framebuffer height
		4. framebuffer bpp
	*/

	framebuffer	= (uint64_t) argv[1];
	width		= (uint64_t) argv[2];
	height		= (uint64_t) argv[3];
	bpp		= (uint64_t) argv[4] / 4;		// kernel gives us BITS per pixel, but we really only care about BYTES per pixel.

	printf("Display server online\n");
	// auto fd = mq_open("/random/path", O_CREATE);

	// int c = 0;
	// puts("Enter text. Include a dot ('.') in a sentence to exit:");
	// do
	// {
	// 	c = getchar();
	// 	putchar(c);

	// } while(c != '.');
	// printf("\ndone\n");


	// uint64_t* ptr = (uint64_t*) 0x00231000;
	// *ptr = 5912344;
	// printf("value = %lx\n", *ptr);




	// FILE* f = fopen("/test/Main.class", "r");
	// struct stat s;

	// fstat((int) f->__fd, &s);
	// printf("file is %ld bytes long\n", s.st_size);

	// uint8_t* x = (uint8_t*) malloc(s.st_size + 1);

	// fread(x, 1, s.st_size, f);
	// fclose(f);

	// for (off_t i = 0; i < s.st_size; i++)
	// 	printf("%02x ", x[i]);

	// fflush(stdout);





	// that's really all we need to do, except watch for messages and flush the screen on occasion.
	return 0;
}
