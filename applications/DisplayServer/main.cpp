// main.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <mqueue.h>
#include <sys/syscall.h>
#include <pthread.h>

static uint64_t framebuffer = 0;
static uint64_t width = 0;
static uint64_t height = 0;
static uint64_t bpp = 0;

static __thread int m = 410;
volatile static bool flag = false;

void* thr(void*)
{
	flag = true;
	m = 200;
	printf("m in thr: %d\n\n", m);
	printf("sleeping for 1000ms\n");
	Library::SystemCall::Sleep(1000);
	printf("awake, hello!\n");

	return (void*) 0xFAD;
}

int main(int argc, char** argv)
{
	assert(argc == 5);
	/*
		1. framebuffer address
		2. framebuffer width
		3. framebuffer height
		4. framebuffer bpp
	*/

	m = 512;
	printf("m in main: %d\n", m);

	pthread_t thrid = 0;
	pthread_create(&thrid, NULL, thr, NULL);

	printf("created thread with id %ld\n", thrid);

	void* retval = 0;
	// while(!flag);
	pthread_join(thrid, &retval);
	printf("m in main: %d\n", m);
	printf("thread retval: %p\n", retval);

	framebuffer	= (uint64_t) argv[1];
	width		= (uint64_t) argv[2];
	height		= (uint64_t) argv[3];
	bpp		= (uint64_t) argv[4] / 4;		// kernel gives us BITS per pixel, but we really only care about BYTES per pixel.

	printf("Display server online\n");
	// auto fd = mq_open("/random/path", O_CREATE);

	// that's really all we need to do, except watch for messages and flush the screen on occasion.
	return 0;
}
