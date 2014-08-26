// main.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

int main()
{
	struct stat st;
	stat("/boot/grub/menu.lst", &st);

	printf("%d bytes\n\n", st.st_size);



	while(true);
}
