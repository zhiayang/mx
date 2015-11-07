// // VariadicPrintf.hpp
// // Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// // Licensed under the Apache License Version 2.0.


// #pragma once

// #include <ctype.h>
// #include <stdint.h>
// #include <stdarg.h>
// #include <stddef.h>
// #include <string.h>

// #include "../../stl/string.h"

// // namespace stl
// // {
// // 	class string;
// // }

// #include "Utility.hpp"

// namespace StdIO
// {
// 	void PrintFmt(const char* c, ...);


// 	// safe printf stuff
// 	// formatting stuff
// 	#define __FLAG_LEFT_ALIGN	0x1
// 	#define __FLAG_PREPEND_SIGN	0x2
// 	#define __FLAG_PREPEND_ZERO	0x4
// 	#define __FLAG_ALT_FORM		0x8

// 	#define __FLAG_PRINT_CAPS	0x10
// 	#define __FLAG_PRINT_HEX	0x20
// 	#define __FLAG_PRINT_DOUBLE	0x40
// 	#define __FLAG_PRINT_FLTG	0x80
// 	#define __FLAG_PRINT_EXP	0x100
// 	#define __FLAG_PRINT_0X_HEX	0x200


// 	template <typename T> stl::string _print(T, uint16_t, uint8_t, uint8_t);

// 	// strings
// 	template <> stl::string _print(stl::string str, uint16_t f, uint8_t w, uint8_t p);
// 	template <> stl::string _print(const char* c, uint16_t f, uint8_t w, uint8_t p);
// 	template <> stl::string _print(char* c, uint16_t f, uint8_t w, uint8_t p);

// 	// numbers
// 	template <> stl::string _print(int8_t c, uint16_t f, uint8_t w, uint8_t p);
// 	template <> stl::string _print(uint8_t c, uint16_t f, uint8_t w, uint8_t p);
// 	template <> stl::string _print(int16_t c, uint16_t f, uint8_t w, uint8_t p);
// 	template <> stl::string _print(uint16_t c, uint16_t f, uint8_t w, uint8_t p);
// 	template <> stl::string _print(int32_t c, uint16_t f, uint8_t w, uint8_t p);
// 	template <> stl::string _print(uint32_t c, uint16_t f, uint8_t w, uint8_t p);
// 	template <> stl::string _print(int64_t c, uint16_t f, uint8_t w, uint8_t p);
// 	template <> stl::string _print(uint64_t c, uint16_t f, uint8_t w, uint8_t p);
// 	template <> stl::string _print(void* c, uint16_t f, uint8_t w, uint8_t p);
// 	template <> stl::string _print(long long c, uint16_t f, uint8_t w, uint8_t p);
// 	template <> stl::string _print(unsigned long long c, uint16_t f, uint8_t w, uint8_t p);


// 	// floating point shits.
// 	template <> stl::string _print(float c, uint16_t f, uint8_t w, uint8_t p);
// 	template <> stl::string _print(double c, uint16_t f, uint8_t w, uint8_t p);
// 	template <> stl::string _print(long double c, uint16_t f, uint8_t w, uint8_t p);


// 	// misc
// 	template <> stl::string _print(bool c, uint16_t f, uint8_t w, uint8_t p);



// 	stl::string Format(const char* fmt);

// 	// let the template voodoo begin
// 	template <typename T, typename... Args>
// 	stl::string Format(const char* fmt, T&& head, Args&&... rest)
// 	{
// 		size_t len = strlen(fmt);
// 		size_t i = 0;

// 		stl::string ret;

// 		while(*fmt && (i < len))
// 		{
// 			if(*fmt == '%')
// 			{
// 				// we have a formatting thing... maybe.
// 				if((i + 1 < len) && *(fmt + 1) == '%')
// 				{
// 					// nope, just a literal '%'.
// 					// skip.
// 					ret += Format("%%");

// 					i++;
// 					fmt++;
// 					continue;
// 				}
// 				else
// 				{
// 					// print all that we have now.
// 					// _print(ofmt, 0, 0, fmt - ofmt);

// 					// eat the %
// 					fmt++; i++;

// 					uint16_t flags = 0;
// 					uint8_t width = (uint8_t) -1;
// 					uint8_t precs = (uint8_t) -1;

// 					// scan for flags
// 					while(*fmt == '-' || *fmt == '+' || *fmt == '0' || *fmt == '#')
// 					{
// 						if(*fmt == '-')	{ if(flags & __FLAG_LEFT_ALIGN) break;	flags |= __FLAG_LEFT_ALIGN;	}
// 						if(*fmt == '+')	{ if(flags & __FLAG_PREPEND_SIGN) break;	flags |= __FLAG_PREPEND_SIGN;	}
// 						if(*fmt == '0')	{ if(flags & __FLAG_PREPEND_ZERO) break;	flags |= __FLAG_PREPEND_ZERO;	}
// 						if(*fmt == '#')	{ if(flags & __FLAG_ALT_FORM) break;		flags |= __FLAG_ALT_FORM;		}

// 						fmt++; i++;
// 					}


// 					// if we're not a dot, we still have width to handle.
// 					if(*fmt != '.')
// 					{
// 						stl::string tmp;
// 						while(*fmt >= '0' && *fmt <= '9')
// 							tmp += *fmt++, i++;

// 						if(tmp.length() > 0)
// 							width = (uint8_t) Library::Utility::ParseInteger(tmp.c_str(), 0, 10);
// 					}

// 					// if we have a '.' now, parse precision.
// 					if(*fmt == '.')
// 					{
// 						if(!isdigit(*(fmt + 1)))
// 						{
// 							// oops, we actually just wanted to print a period.
// 							// don't do anything.
// 						}
// 						else
// 						{
// 							// eat the '.'
// 							fmt++; i++;

// 							stl::string tmp;
// 							while(*fmt >= '0' && *fmt <= '9')
// 								tmp += *fmt++, i++;

// 							if(tmp.length() > 0)
// 								precs = (uint8_t) Library::Utility::ParseInteger(tmp.c_str(), 0, 10);
// 						}
// 					}

// 					// handle the "special" formatters:
// 					// x/X, g/G, e/E, f/F, a/A.

// 					if(tolower(*fmt) == 'x' || tolower(*fmt) == 'p')
// 					{
// 						flags |= __FLAG_PRINT_HEX;

// 						if(tolower(*fmt) == 'p')
// 							flags |= __FLAG_ALT_FORM;

// 						fmt++; i++;

// 						if(isupper(*fmt)) flags |= __FLAG_PRINT_CAPS;
// 					}

// 					if(tolower(*fmt) == 'a')
// 					{
// 						flags |= __FLAG_PRINT_0X_HEX;
// 						fmt++; i++;

// 						if(isupper(*fmt)) flags |= __FLAG_PRINT_CAPS;
// 					}
// 					if(tolower(*fmt) == 'f')
// 					{
// 						flags |= __FLAG_PRINT_DOUBLE;
// 						fmt++; i++;

// 						if(isupper(*fmt)) flags |= __FLAG_PRINT_CAPS;
// 					}
// 					if(tolower(*fmt) == 'e')
// 					{
// 						flags |= __FLAG_PRINT_EXP;
// 						fmt++; i++;

// 						if(isupper(*fmt)) flags |= __FLAG_PRINT_CAPS;
// 					}
// 					if(tolower(*fmt) == 'g')
// 					{
// 						flags |= __FLAG_PRINT_FLTG;
// 						fmt++; i++;

// 						if(isupper(*fmt)) flags |= __FLAG_PRINT_CAPS;
// 					}

// 					// print the current thing
// 					ret += _print(head, flags, width, precs);

// 					// now, we call print().
// 					ret += Format(fmt, rest...);
// 					break;
// 				}
// 			}
// 			else
// 			{
// 				ret += *fmt;
// 				fmt++; i++;
// 			}
// 		}

// 		return ret;
// 	}

// 	void PrintF(const char* fmt);

// 	template <typename T, typename... Args>
// 	void PrintF(const char* fmt, T&& head, Args&&... rest)
// 	{
// 		stl::string ret = Format(fmt, head, rest...);
// 		PrintFmt(ret.c_str());
// 	}
// }





















































