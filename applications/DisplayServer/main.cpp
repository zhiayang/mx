// main.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <netinet/in.h>
#include <sys/socket.h>
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
	bpp			= (uint64_t) argv[4] / 8;		// kernel gives us BITS per pixel, but we really only care about BYTES per pixel.

	printf("Display server online\n");
	printf("Forking process...\n");
	// int64_t res = Library::SystemCall::ForkProcess();

	// if(res == 0)
	// {
	// 	printf("in child, exiting\n");
	// 	exit(0);
	// }
	// else
	// {
	// 	printf("parent: child proc has pid %ld, continuing\n", res);
	// }

	// Library::SystemCall::SpawnProcess("/test.mxa", "test");

	printf("stuff is going on\n");





	{
		FILE* f = fopen("/boot/grub/menu.lst", "r");
		struct stat s;

		fstat((int) f->__fd, &s);
		printf("file is %ld bytes long\n", s.st_size);

		uint8_t* x = (uint8_t*) malloc(s.st_size + 1);

		fread(x, 1, s.st_size, f);
		fclose(f);

		printf("%s", x);

		fflush(stdout);
	}

	{
		// int s = socket(AF_INET, SOCK_STREAM, 0);
		// struct sockaddr_in addr;
		// addr.sin_family = AF_INET;
		// addr.sin_addr.s_addr = 1572395042;
		// addr.sin_port = 80;

		// connect(s, (struct sockaddr*) &addr, sizeof(addr));
		// close(s);
	}

















	// that's really all we need to do, except watch for messages and flush the screen on occasion.
	return 0;
}




















