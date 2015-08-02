// SafeStandardIO::PrintFormatted.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "HeaderFiles/Utility.hpp"
#include "HeaderFiles/SafePrintf.hpp"
#include "HeaderFiles/StandardIO.hpp"

#include <ctype.h>
#include <assert.h>
#include <string.h>

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

using namespace Library;
static astl::string _getfmt(uint16_t flags, uint8_t width, uint8_t prec)
{
	astl::string ret;
	if(flags & FLAG_LEFT_ALIGN)		ret += "-";
	if(flags & FLAG_PREPEND_SIGN)	ret += "+";
	if(flags & FLAG_PREPEND_ZERO)	ret += "0";
	if(flags & FLAG_ALT_FORM)		ret += "#";

	static char tmp[64] = { };
	memset(tmp, 0, sizeof(tmp));


	if(width != (uint8_t) -1)
		ret += Utility::ConvertToString(width, tmp);

	memset(tmp, 0, sizeof(tmp));


	if(prec != (uint8_t) -1)
		ret += astl::string(".") + Utility::ConvertToString(prec, tmp);

	memset(tmp, 0, sizeof(tmp));

	return "%" + ret;
}


static astl::string _getSpecForHex(uint16_t f, bool u)
{
	if(f & FLAG_PRINT_CAPS)
	{
		if(f & FLAG_PRINT_HEX)		return "X";
		else						return (u ? "u" : "d");
	}
	else
	{
		if(f & FLAG_PRINT_HEX)		return "x";
		else						return (u ? "u" : "d");
	}
}

static astl::string _getSpecForFloat(uint16_t f)
{
	if(f & FLAG_PRINT_CAPS)
	{
		if(f & FLAG_PRINT_DOUBLE)	return "F";
		if(f & FLAG_PRINT_EXP)		return "E";
		if(f & FLAG_PRINT_FLTG)		return "G";
		if(f & FLAG_PRINT_0X_HEX)	return "A";

		assert(0);
		return "";
	}
	else
	{
		if(f & FLAG_PRINT_DOUBLE)	return "f";
		if(f & FLAG_PRINT_EXP)		return "e";
		if(f & FLAG_PRINT_FLTG)		return "g";
		if(f & FLAG_PRINT_0X_HEX)	return "a";

		assert(0);
		return "";
	}
}




// usage:
// print("%[flags][width].[prec] %[flags][width].[prec] etc.");
//


// base cases (aka template specialisations)

template <typename T> void _print(T, uint16_t, uint8_t, uint8_t);

// strings
template <>
void _print(astl::string str, uint16_t f, uint8_t w, uint8_t p)
{
	astl::string fmt = _getfmt(f, w, p);
	fmt += "s";

	StandardIO::PrintFormatted(fmt.c_str(), str.c_str());
}

template <>
void _print(const char* c, uint16_t f, uint8_t w, uint8_t p)
{
	astl::string fmt = _getfmt(f, w, p);
	fmt += "s";

	StandardIO::PrintFormatted(fmt.c_str(), c);
}

template <>
void _print(char* c, uint16_t f, uint8_t w, uint8_t p)
{
	astl::string fmt = _getfmt(f, w, p);
	fmt += "s";

	StandardIO::PrintFormatted(fmt.c_str(), c);
}




// numbers
template <>
void _print(char c, uint16_t f, uint8_t w, uint8_t p)
{
	astl::string fmt = _getfmt(f, w, p);
	fmt += astl::string("hh") + _getSpecForHex(f, false);

	StandardIO::PrintFormatted(fmt.c_str(), c);
}

template <>
void _print(unsigned char c, uint16_t f, uint8_t w, uint8_t p)
{
	astl::string fmt = _getfmt(f, w, p);
	fmt += astl::string("hh") + _getSpecForHex(f, true);

	StandardIO::PrintFormatted(fmt.c_str(), c);
}

template <>
void _print(short c, uint16_t f, uint8_t w, uint8_t p)
{
	astl::string fmt = _getfmt(f, w, p);
	fmt += astl::string("h") + _getSpecForHex(f, false);

	StandardIO::PrintFormatted(fmt.c_str(), c);
}

template <>
void _print(unsigned short c, uint16_t f, uint8_t w, uint8_t p)
{
	astl::string fmt = _getfmt(f, w, p);
	fmt += astl::string("h") + _getSpecForHex(f, true);

	StandardIO::PrintFormatted(fmt.c_str(), c);
}

template <>
void _print(int c, uint16_t f, uint8_t w, uint8_t p)
{
	astl::string fmt = _getfmt(f, w, p);
	fmt += _getSpecForHex(f, false);

	StandardIO::PrintFormatted(fmt.c_str(), c);
}

template <>
void _print(unsigned int c, uint16_t f, uint8_t w, uint8_t p)
{
	astl::string fmt = _getfmt(f, w, p);
	fmt += _getSpecForHex(f, true);

	StandardIO::PrintFormatted(fmt.c_str(), c);
}

template <>
void _print(long c, uint16_t f, uint8_t w, uint8_t p)
{
	astl::string fmt = _getfmt(f, w, p);
	fmt += astl::string("l") + _getSpecForHex(f, false);

	StandardIO::PrintFormatted(fmt.c_str(), c);
}

template <>
void _print(unsigned long c, uint16_t f, uint8_t w, uint8_t p)
{
	astl::string fmt = _getfmt(f, w, p);
	fmt += astl::string("l") + _getSpecForHex(f, true);

	StandardIO::PrintFormatted(fmt.c_str(), c);
}

template <>
void _print(long long c, uint16_t f, uint8_t w, uint8_t p)
{
	astl::string fmt = _getfmt(f, w, p);
	fmt += astl::string("ll") + _getSpecForHex(f, false);

	StandardIO::PrintFormatted(fmt.c_str(), c);
}

template <>
void _print(unsigned long long c, uint16_t f, uint8_t w, uint8_t p)
{
	astl::string fmt = _getfmt(f, w, p);
	fmt += astl::string("ll") + _getSpecForHex(f, true);

	StandardIO::PrintFormatted(fmt.c_str(), c);
}




// floating point shits.
template <>
void _print(float c, uint16_t f, uint8_t w, uint8_t p)
{
	astl::string fmt = _getfmt(f, w, p);
	fmt += _getSpecForFloat(f);

	StandardIO::PrintFormatted(fmt.c_str(), c);
}

template <>
void _print(double c, uint16_t f, uint8_t w, uint8_t p)
{
	astl::string fmt = _getfmt(f, w, p);
	fmt += astl::string("l") + _getSpecForFloat(f);

	StandardIO::PrintFormatted(fmt.c_str(), c);
}

template <>
void _print(long double c, uint16_t f, uint8_t w, uint8_t p)
{
	astl::string fmt = _getfmt(f, w, p);
	fmt += astl::string("L") + _getSpecForFloat(f);

	StandardIO::PrintFormatted(fmt.c_str(), c);
}

void print(const char* fmt)
{
	StandardIO::PrintFormatted("%s", fmt);
}











