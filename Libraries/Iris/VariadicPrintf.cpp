// // SafeStandardIO::PrintFmt.cpp
// // Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// // Licensed under the Apache License Version 2.0.

// #include "HeaderFiles/Utility.hpp"
// #include "HeaderFiles/StandardIO.hpp"

// #include <ctype.h>
// #include <assert.h>
// #include <string.h>

// using namespace Library;
// namespace StdIO
// {
// 	static stl::string _getfmt(uint16_t flags, uint8_t width, uint8_t prec)
// 	{
// 		stl::string ret;
// 		if(flags & __FLAG_LEFT_ALIGN)	ret += "-";
// 		if(flags & __FLAG_PREPEND_SIGN)	ret += "+";
// 		if(flags & __FLAG_PREPEND_ZERO)	ret += "0";
// 		if(flags & __FLAG_ALT_FORM)		ret += "#";

// 		static char tmp[64] = { };
// 		memset(tmp, 0, sizeof(tmp));


// 		if(width != (uint8_t) -1)
// 			ret += Utility::ConvertToString(width, tmp);

// 		memset(tmp, 0, sizeof(tmp));


// 		if(prec != (uint8_t) -1)
// 			ret += stl::string(".") + Utility::ConvertToString(prec, tmp);

// 		memset(tmp, 0, sizeof(tmp));

// 		return "%" + ret;
// 	}


// 	static stl::string _getSpecForHex(uint16_t f, bool u)
// 	{
// 		if(f & __FLAG_PRINT_CAPS)
// 		{
// 			if(f & __FLAG_PRINT_HEX)	return "X";
// 			else						return (u ? "u" : "d");
// 		}
// 		else
// 		{
// 			if(f & __FLAG_PRINT_HEX)	return "x";
// 			else						return (u ? "u" : "d");
// 		}
// 	}

// 	static stl::string _getSpecForFloat(uint16_t f)
// 	{
// 		if(f & __FLAG_PRINT_CAPS)
// 		{
// 			if(f & __FLAG_PRINT_DOUBLE)	return "F";
// 			if(f & __FLAG_PRINT_EXP)	return "E";
// 			if(f & __FLAG_PRINT_FLTG)	return "G";
// 			if(f & __FLAG_PRINT_0X_HEX)	return "A";

// 			assert(0);
// 			return "";
// 		}
// 		else
// 		{
// 			if(f & __FLAG_PRINT_DOUBLE)	return "f";
// 			if(f & __FLAG_PRINT_EXP)		return "e";
// 			if(f & __FLAG_PRINT_FLTG)		return "g";
// 			if(f & __FLAG_PRINT_0X_HEX)	return "a";

// 			assert(0);
// 			return "";
// 		}
// 	}




// 	// usage:
// 	// print("%[flags][width].[prec] %[flags][width].[prec] etc.");
// 	//


// 	// base cases (aka template specialisations)

// 	static size_t appendToString(void* v, const char* s, size_t len)
// 	{
// 		stl::string* str = (stl::string*) v;
// 		str->append(s, len);

// 		return len;
// 	}


// 	// strings
// 	template <>
// 	stl::string _print(stl::string str, uint16_t f, uint8_t w, uint8_t p)
// 	{
// 		stl::string fmt = _getfmt(f, w, p);
// 		fmt += "s";

// 		stl::string ret;
// 		_printf_callback(appendToString, &ret, fmt.c_str(), str.c_str());
// 		return ret;
// 	}

// 	template <>
// 	stl::string _print(const char* c, uint16_t f, uint8_t w, uint8_t p)
// 	{
// 		stl::string fmt = _getfmt(f, w, p);
// 		fmt += "s";

// 		stl::string ret;
// 		_printf_callback(appendToString, &ret, fmt.c_str(), c);
// 		return ret;
// 	}

// 	template <>
// 	stl::string _print(char* c, uint16_t f, uint8_t w, uint8_t p)
// 	{
// 		stl::string fmt = _getfmt(f, w, p);
// 		fmt += "s";

// 		stl::string ret;
// 		_printf_callback(appendToString, &ret, fmt.c_str(), c);
// 		return ret;
// 	}




// 	// numbers
// 	template <>
// 	stl::string _print(char c, uint16_t f, uint8_t w, uint8_t p)
// 	{
// 		stl::string fmt = _getfmt(f, w, p);
// 		fmt += stl::string("hh") + _getSpecForHex(f, false);

// 		stl::string ret;
// 		_printf_callback(appendToString, &ret, fmt.c_str(), c);
// 		return ret;
// 	}

// 	template <>
// 	stl::string _print(unsigned char c, uint16_t f, uint8_t w, uint8_t p)
// 	{
// 		stl::string fmt = _getfmt(f, w, p);
// 		fmt += stl::string("hh") + _getSpecForHex(f, true);

// 		stl::string ret;
// 		_printf_callback(appendToString, &ret, fmt.c_str(), c);
// 		return ret;
// 	}

// 	template <>
// 	stl::string _print(short c, uint16_t f, uint8_t w, uint8_t p)
// 	{
// 		stl::string fmt = _getfmt(f, w, p);
// 		fmt += stl::string("h") + _getSpecForHex(f, false);

// 		stl::string ret;
// 		_printf_callback(appendToString, &ret, fmt.c_str(), c);
// 		return ret;
// 	}

// 	template <>
// 	stl::string _print(unsigned short c, uint16_t f, uint8_t w, uint8_t p)
// 	{
// 		stl::string fmt = _getfmt(f, w, p);
// 		fmt += stl::string("h") + _getSpecForHex(f, true);

// 		stl::string ret;
// 		_printf_callback(appendToString, &ret, fmt.c_str(), c);
// 		return ret;
// 	}

// 	template <>
// 	stl::string _print(int c, uint16_t f, uint8_t w, uint8_t p)
// 	{
// 		stl::string fmt = _getfmt(f, w, p);
// 		fmt += _getSpecForHex(f, false);

// 		stl::string ret;
// 		_printf_callback(appendToString, &ret, fmt.c_str(), c);
// 		return ret;
// 	}

// 	template <>
// 	stl::string _print(unsigned int c, uint16_t f, uint8_t w, uint8_t p)
// 	{
// 		stl::string fmt = _getfmt(f, w, p);
// 		fmt += _getSpecForHex(f, true);

// 		stl::string ret;
// 		_printf_callback(appendToString, &ret, fmt.c_str(), c);
// 		return ret;
// 	}

// 	template <>
// 	stl::string _print(long c, uint16_t f, uint8_t w, uint8_t p)
// 	{
// 		stl::string fmt = _getfmt(f, w, p);
// 		fmt += stl::string("l") + _getSpecForHex(f, false);

// 		stl::string ret;
// 		_printf_callback(appendToString, &ret, fmt.c_str(), c);
// 		return ret;
// 	}

// 	template <>
// 	stl::string _print(unsigned long c, uint16_t f, uint8_t w, uint8_t p)
// 	{
// 		stl::string fmt = _getfmt(f, w, p);
// 		fmt += stl::string("l") + _getSpecForHex(f, true);

// 		stl::string ret;
// 		_printf_callback(appendToString, &ret, fmt.c_str(), c);
// 		return ret;
// 	}

// 	template <>
// 	stl::string _print(long long c, uint16_t f, uint8_t w, uint8_t p)
// 	{
// 		stl::string fmt = _getfmt(f, w, p);
// 		fmt += stl::string("ll") + _getSpecForHex(f, false);

// 		stl::string ret;
// 		_printf_callback(appendToString, &ret, fmt.c_str(), c);
// 		return ret;
// 	}

// 	template <>
// 	stl::string _print(unsigned long long c, uint16_t f, uint8_t w, uint8_t p)
// 	{
// 		stl::string fmt = _getfmt(f, w, p);
// 		fmt += stl::string("ll") + _getSpecForHex(f, true);

// 		stl::string ret;
// 		_printf_callback(appendToString, &ret, fmt.c_str(), c);
// 		return ret;
// 	}




// 	// floating point shits.
// 	template <>
// 	stl::string _print(float c, uint16_t f, uint8_t w, uint8_t p)
// 	{
// 		stl::string fmt = _getfmt(f, w, p);
// 		fmt += _getSpecForFloat(f);

// 		stl::string ret;
// 		_printf_callback(appendToString, &ret, fmt.c_str(), c);
// 		return ret;
// 	}

// 	template <>
// 	stl::string _print(double c, uint16_t f, uint8_t w, uint8_t p)
// 	{
// 		stl::string fmt = _getfmt(f, w, p);
// 		fmt += stl::string("l") + _getSpecForFloat(f);

// 		stl::string ret;
// 		_printf_callback(appendToString, &ret, fmt.c_str(), c);
// 		return ret;
// 	}

// 	template <>
// 	stl::string _print(long double c, uint16_t f, uint8_t w, uint8_t p)
// 	{
// 		stl::string fmt = _getfmt(f, w, p);
// 		fmt += stl::string("L") + _getSpecForFloat(f);

// 		stl::string ret;
// 		_printf_callback(appendToString, &ret, fmt.c_str(), c);
// 		return ret;
// 	}



// 	 // misc things
// 	template <> stl::string _print(bool c, uint16_t f, uint8_t w, uint8_t p)
// 	{
// 		stl::string fmt = _getfmt(f, w, p);

// 		fmt += "s";

// 		stl::string ret;
// 		_printf_callback(appendToString, &ret, fmt.c_str(), (f & __FLAG_ALT_FORM) ? (c ? "yes" : "no") : (c ? "true" : "false"));
// 		return ret;
// 	}




// 	void PrintF(const char* fmt)
// 	{
// 		PrintFmt(fmt);
// 	}

// 	stl::string Format(const char* fmt)
// 	{
// 		return stl::string(fmt);
// 	}
// }











