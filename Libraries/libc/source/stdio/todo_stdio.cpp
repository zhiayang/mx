// todo.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "../../include/stdio.h"

extern "C" int feof(FILE* f)
{
	// TODO
	(void) f;
	return 0;
}

extern "C" void clearerr(FILE*)
{
}

extern "C" int ferror(FILE*)
{
	return 0;
}

extern "C" int fflush(FILE*)
{
	return 0;
}

extern "C" int fgetpos(FILE* str, fpos_t* pos)
{
	(void) str;
	*pos = 0;

	return 0;
}

extern "C" char* fgets(char* str, int num, FILE* stream)
{
	(void) str;
	(void) num;
	(void) stream;

	return str;
}

extern "C" int fscanf(FILE* str, const char* fmt, ...)
{
	(void) str;
	va_list args;
	va_start(args, fmt);
	va_end(args);

	return EOF;
}

extern "C" int scanf(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	va_end(args);

	return EOF;
}

extern "C" int sscanf(const char* s, const char* fmt, ...)
{
	(void) s;
	va_list args;
	va_start(args, fmt);
	va_end(args);

	return EOF;
}

extern "C" int fseek(FILE* str, off_t offset, int origin)
{
	(void) str;
	(void) offset;
	(void) origin;

	return 0;
}

extern "C" int fsetpos(FILE* str, const fpos_t* pos)
{
	(void) str;
	(void) pos;

	return 0;
}

extern "C" off_t ftell(FILE* str)
{
	(void) str;
	return EOF;
}

extern "C" int remove(const char* path)
{
	(void) path;
	return 0;
}

extern "C" int rename(const char* oldname, const char* newname)
{
	(void) oldname;
	(void) newname;

	return -1;
}

extern "C" void rewind(FILE*)
{
}

extern "C" void setbuf(FILE*, char*)
{
}

extern "C" int setvbuf(FILE*, char*, int, size_t)
{
	return -1;
}

extern "C" FILE* tmpfile()
{
	return NULL;
}

extern "C" int ungetc(int c, FILE* str)
{
	(void) c;
	(void) str;

	return EOF;
}



















