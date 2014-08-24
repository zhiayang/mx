// main.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>

int main()
{
	// asm volatile("xor %%r10, %%r10; int $0xF8" ::: );
	Library::SystemCall::Write(1, (uint8_t*) "Hello, World!!", 13);
	while(true);
}
