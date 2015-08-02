// SafePrintf.hpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#pragma once

#include <ctype.h>
#include <stdint.h>
#include <string.h>

#include "../../acess2_stl/string.h"

#include "Utility.hpp"
#include "StandardIO.hpp"

// formatting stuff
#define FLAG_LEFT_ALIGN		0x1
#define FLAG_PREPEND_SIGN	0x2
#define FLAG_PREPEND_ZERO	0x4
#define FLAG_ALT_FORM		0x8

#define FLAG_PRINT_CAPS		0x10
#define FLAG_PRINT_HEX		0x20
#define FLAG_PRINT_DOUBLE	0x40
#define FLAG_PRINT_FLTG		0x80
#define FLAG_PRINT_EXP		0x100
#define FLAG_PRINT_0X_HEX	0x200


template <typename T> void _print(T, uint16_t, uint8_t, uint8_t);

// strings
template <> void _print(astl::string str, uint16_t f, uint8_t w, uint8_t p);
template <> void _print(const char* c, uint16_t f, uint8_t w, uint8_t p);
template <> void _print(char* c, uint16_t f, uint8_t w, uint8_t p);

// numbers
template <> void _print(int8_t c, uint16_t f, uint8_t w, uint8_t p);
template <> void _print(uint8_t c, uint16_t f, uint8_t w, uint8_t p);
template <> void _print(int16_t c, uint16_t f, uint8_t w, uint8_t p);
template <> void _print(uint16_t c, uint16_t f, uint8_t w, uint8_t p);
template <> void _print(int32_t c, uint16_t f, uint8_t w, uint8_t p);
template <> void _print(uint32_t c, uint16_t f, uint8_t w, uint8_t p);
template <> void _print(int64_t c, uint16_t f, uint8_t w, uint8_t p);
template <> void _print(uint64_t c, uint16_t f, uint8_t w, uint8_t p);
template <> void _print(void* c, uint16_t f, uint8_t w, uint8_t p);
template <> void _print(long long c, uint16_t f, uint8_t w, uint8_t p);
template <> void _print(unsigned long long c, uint16_t f, uint8_t w, uint8_t p);


// floating point shits.
template <> void _print(float c, uint16_t f, uint8_t w, uint8_t p);
template <> void _print(double c, uint16_t f, uint8_t w, uint8_t p);
template <> void _print(long double c, uint16_t f, uint8_t w, uint8_t p);



void print(const char* fmt);

// let the template voodoo begin
template <typename T, typename... Args>
void print(const char* fmt, T&& head, Args&&... rest)
{
	size_t len = strlen(fmt);
	size_t i = 0;

	while(*fmt && (i < len))
	{
		if(*fmt == '%')
		{
			// we have a formatting thing... maybe.
			if((i + 1 < len) && *(fmt + 1) == '%')
			{
				// nope, just a literal '%'.
				// skip.
				Library::StandardIO::PrintFormatted("%%");

				i++;
				fmt++;
				continue;
			}
			else
			{
				// print all that we have now.
				// _print(ofmt, 0, 0, fmt - ofmt);

				// eat the %
				fmt++; i++;

				uint16_t flags = 0;
				uint8_t width = (uint8_t) -1;
				uint8_t precs = (uint8_t) -1;

				// scan for flags
				while(*fmt == '-' || *fmt == '+' || *fmt == '0' || *fmt == '#')
				{
					if(*fmt == '-')	{ if(flags & FLAG_LEFT_ALIGN) break;	flags |= FLAG_LEFT_ALIGN;	}
					if(*fmt == '+')	{ if(flags & FLAG_PREPEND_SIGN) break;	flags |= FLAG_PREPEND_SIGN;	}
					if(*fmt == '0')	{ if(flags & FLAG_PREPEND_ZERO) break;	flags |= FLAG_PREPEND_ZERO;	}
					if(*fmt == '#')	{ if(flags & FLAG_ALT_FORM) break;		flags |= FLAG_ALT_FORM;		}

					fmt++; i++;
				}


				// if we're not a dot, we still have width to handle.
				if(*fmt != '.')
				{
					astl::string tmp;
					while(*fmt >= '0' && *fmt <= '9')
						tmp += *fmt++, i++;

					if(tmp.length() > 0)
						width = (uint8_t) Library::Utility::ParseInteger(tmp.c_str(), 0, 10);
				}

				// if we have a '.' now, parse precision.
				if(*fmt == '.')
				{
					// eat the '.'
					fmt++; i++;

					astl::string tmp;
					while(*fmt >= '0' && *fmt <= '9')
						tmp += *fmt++, i++;

					if(tmp.length() > 0)
						precs = (uint8_t) Library::Utility::ParseInteger(tmp.c_str(), 0, 10);
				}

				// handle the "special" formatters:
				// x/X, g/G, e/E, f/F, a/A.

				if(tolower(*fmt) == 'x') { flags |= FLAG_PRINT_HEX;		fmt++; i++; if(isupper(*fmt)) flags |= FLAG_PRINT_CAPS; }
				if(tolower(*fmt) == 'a') { flags |= FLAG_PRINT_0X_HEX;	fmt++; i++; if(isupper(*fmt)) flags |= FLAG_PRINT_CAPS; }
				if(tolower(*fmt) == 'f') { flags |= FLAG_PRINT_DOUBLE;	fmt++; i++; if(isupper(*fmt)) flags |= FLAG_PRINT_CAPS; }
				if(tolower(*fmt) == 'e') { flags |= FLAG_PRINT_EXP;		fmt++; i++; if(isupper(*fmt)) flags |= FLAG_PRINT_CAPS; }
				if(tolower(*fmt) == 'g') { flags |= FLAG_PRINT_FLTG;	fmt++; i++; if(isupper(*fmt)) flags |= FLAG_PRINT_CAPS; }

				// print the current thing
				_print(head, flags, width, precs);

				// now, we call print().
				print(fmt, rest...);
				break;
			}
		}
		else
		{
			Library::StandardIO::PrintFormatted("%c", *fmt);
			fmt++; i++;
		}
	}
}

















