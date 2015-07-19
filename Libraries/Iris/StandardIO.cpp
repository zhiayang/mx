// userspace/StandardIO.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "HeaderFiles/StandardIO.hpp"
#include <string.h>
#include "HeaderFiles/Memory.hpp"
#include "HeaderFiles/Utility.hpp"
#include <stdlib.h>


#if 1

namespace Kernel
{
	namespace Console
	{
		void PrintChar(uint8_t c);
		void SetColour(uint32_t Colour);
	}
}

namespace Library {
namespace StandardIO
{
	static void PrintChar(uint8_t c, void (*pf)(uint8_t))
	{
		if(pf)
			pf(c);

		else
			Kernel::Console::PrintChar(c);
	}

	static void SwallowChar(uint8_t)
	{
	}

	uint64_t PrintString(const char* string, size_t length, void (*pf)(uint8_t))
	{
		if(string == 0) return 0;

		uint64_t _len = ((length == (size_t) -1 || length == 0) ? strlen(string) : length);

		uint64_t i = 0;
		for(; i < _len; i++)
		{
			PrintChar((uint8_t) string[i], pf);
		}

		return i;
	}


	static uint64_t PrintHex_NoPrefix(uint64_t n, bool ReverseEndianness = false, bool lowercase = false, void (*pf)(uint8_t) = 0)
	{
		int64_t tmp = 0;
		int64_t i = 0;
		uint64_t ret = 0;


		// Mask bits of variables to determine size, and therefore how many digits to print.

		if((n & 0xF000000000000000) == 0)
			if((n & 0xFF00000000000000) == 0)
				if((n & 0xFFF0000000000000) == 0)
					if((n & 0xFFFF000000000000) == 0)
						if((n & 0xFFFFF00000000000) == 0)
							if((n & 0xFFFFFF0000000000) == 0)
								if((n & 0xFFFFFFF000000000) == 0)
									if((n & 0xFFFFFFFF00000000) == 0)
										if((n & 0xFFFFFFFFF0000000) == 0)
											if((n & 0xFFFFFFFFFF000000) == 0)
												if((n & 0xFFFFFFFFFFF00000) == 0)
													if((n & 0xFFFFFFFFFFFF0000) == 0)
														if((n & 0xFFFFFFFFFFFFF000) == 0)
															if((n & 0xFFFFFFFFFFFFFF00) == 0)
																if((n & 0xFFFFFFFFFFFFFFF0) == 0)
																	i = 0;
																else
																	i = 4;
															else
																i = 8;
														else
															i = 12;
													else
														i = 16;
												else
													i = 20;
											else
												i = 24;
										else
											i = 28;
									else
										i = 32;
								else
									i = 36;
							else
								i = 40;
						else
							i = 44;
					else
						i = 48;
				else
					i = 52;
			else
				i = 56;
		else
			i = 60;


		if(!ReverseEndianness)
		{
			for(; i >= 0; i -= 4)
			{
				tmp = (n >> i) & 0xF;


				if(tmp >= 0xA)
					PrintChar((uint8_t)(tmp - 0xA + (lowercase ? 'a' : 'A')), pf);

				else
					PrintChar((uint8_t)(tmp + '0'), pf);

				ret++;
			}
		}
		else
		{
			for(int z = 0; z <= i; z += 8)
			{
				tmp = (n >> z) & 0xFF;
				return PrintHex_NoPrefix((uint8_t) tmp, ReverseEndianness, lowercase, pf);
			}
		}

		return ret;
	}

	static uint64_t PrintHex_Precision_NoPrefix(uint64_t n, int8_t leadingzeroes, bool ReverseEndianness, bool lowercase,
		bool padzeroes, int8_t prec, bool ppf, void (*pf)(uint8_t) = 0)
	{
		(void) padzeroes;
		if(prec < 0)
		{
			if(ppf)
				PrintString("0x", -1, pf);

			return PrintHex_NoPrefix(n, ReverseEndianness, lowercase, pf) + 2;
		}

		int64_t tmp;
		int64_t i = (prec * 4) - 4;
		uint64_t ret = 0;

		if(ppf)
			PrintString("0x", -1, pf);

		if(n == 0)
		{
			for(int8_t d = 0; d < leadingzeroes; d++)
				PrintString("0", -1, pf);

			return leadingzeroes;
		}


		if(!ReverseEndianness)
		{
			for(; i >= 0; i -= 4)
			{
				tmp = (n >> i) & 0xF;


				if(tmp >= 0xA)
					PrintChar((uint8_t) tmp - 0xA + (lowercase ? 'a' : 'A'), pf);

				else
					PrintChar((uint8_t)(tmp + '0'), pf);

				ret++;
			}
		}
		else
		{
			for(int z = 0; z <= i; z += 8)
			{
				tmp = (n >> z) & 0xFF;
				return PrintHex_NoPrefix((uint8_t) tmp, ReverseEndianness, lowercase, pf);
			}
		}

		return ret;
	}








	static uint64_t PrintInteger_Signed(int64_t num, int8_t Width = -1, void (*pf)(uint8_t) = 0)
	{
		uint64_t ret = 0;

		if(num == 0)
		{
			if(Width != -1)
			{
				for(int g = 0; g < Width; g++)
				{
					PrintChar('0', pf);
					ret++;
				}
			}
			else
			{
				PrintChar('0', pf);
				ret++;
			}

			return ret;
		}

		if(num < 0){ PrintChar('-', pf); ret++; }
		if(Width != -1)
		{
			uint64_t n = (uint64_t) __abs(num);
			uint8_t k = 0;
			while(n > 0)
			{
				n /= 10;
				k++;
			}

			while(Width > k)
			{
				PrintChar('0', pf);
				k++;
				ret++;
			}
		}
		char out[32] = { 0 };
		char* k = Utility::ConvertToString(num, out);
		auto r = PrintString(k, -1, pf) + ret;

		return r;
	}

	#define truncate(x)			((double) ((int64_t) (x)))
	#define round(x)			(((x) < 0) ? ((double) ((int64_t) ((x) - 0.5))) : ((double) ((int64_t) ((x) + 0.5))))




	static uint8_t PrintFloat(double fl, int8_t precision = 15, void (*pf)(uint8_t) = 0)
	{
		if(precision < 0)
		{
			precision = 15;
		}

		// Put integer part first
		PrintInteger_Signed((int64_t) truncate(fl), -1, pf);
		PrintChar('.', pf);

		if(truncate(fl) == fl)
		{
			return (uint8_t) precision;
		}

		if(fl < 0)
		{
			fl  = -fl;
		}

		// Get decimal part
		fl -= truncate(fl);

		uint32_t digit = 0;
		while(fl > 0 && precision > 0)
		{
			fl *= 10;

			if(precision == 1)
				digit = (uint32_t) round(fl);

			else
				digit = (uint32_t) fl;

			if(!(digit + '0' >= '0' && digit + '0' <= '9'))
			{
				PrintChar('0', pf);
				return 0;
			}

			PrintChar((uint8_t) digit + '0', pf);
			precision--;
			fl -= digit;
		}
		// Return the remaining number -- handle in printk() to print trailing zeroes
		return (uint8_t) precision;
	}





	void PrintFormatted(void (*pf)(uint8_t), const char* str, ...)
	{
		va_list args;
		va_start(args, str);
		PrintFormatted(pf, str, args);
		va_end(args);
	}

	void PrintFormatted(const char* str, ...)
	{
		va_list args;
		va_start(args, str);
		PrintFormatted(0, str, args);
		va_end(args);
	}

	extern "C" void printf(const char* str, ...)
	{
		va_list args;
		va_start(args, str);
		PrintFormatted(0, str, args);
		va_end(args);
	}

	extern "C" int puts(const char* str)
	{
		PrintString(str);
		return 0;
	}

	void PrintFormatted(const char* str, va_list args)
	{
		PrintFormatted(0, str, args);
	}


	void PrintFormatted(void (*pf)(uint8_t), const char* string, va_list args)
	{
		// Note:
		/*
			This is totally non-standard printf.
			A number of custom formatters are available:
			%r resets to white. (no arguments)

			The behaviour of hex printing is quite different:
			%x prints in uppercase.
			%X prints in lowercase.

			%x prints /with/ the leading '0x'
			%#x prints without.

			%[width]d now uses spaces for padding by default, while %0[width]d uses zeroes, as per printf-standard.

			We now 'almost' follow the printf standard of  %[parameter][flags][width][.precision][length]type
			/almost/.

			'Parameter' is not supported. It might be, when a valid use case is encountered.
			'Flags' in this case is either '0', '#', '+', '^' (custom) or ' '.
			'Width' is fully supported.
			'Precision' is also fully supported.

			'Length' is only partially supported -- they will only be taken into account for floating point numbers,
			because for other types we accept the largest type by default.
			Additionally, we always accept at least a 'double' for those -- therefore 'L' and 'l' function the same way,
			in that we instead look for a long double.



			However, because front-padding (align right) for hex numbers is handled in the PrintHex_Precision_ functions,
			the Precision formatter is used to control how many zeroes to print when the input number is zero.

			For example: %16x will print [width] zeroes by default if the input is zero.
			However, %16.3x will print up to 3 zeroes, padding the rest with spaces.
		*/


		bool IsFormat = false;
		bool DisplaySign = false;
		bool OmitZeroX = false;
		bool IsParsingPrecision = false;
		bool LeftAlign = false;
		bool PadZeroes = false;
		bool PadSignedSpace = false;

		// we don't support ints smaller than 64bit.
		// 1 is long double.
		int8_t ArgSize = -1;
		int8_t Precision = -1;
		int8_t Width = -1;

		uint64_t PrintedChars = 0;

		char c = 0;
		int64_t z = 0;
		uint64_t x = 0;
		double f = 0.00;
		char* s = 0;
		char ch = 0;
		bool b = false;

		uint64_t length = strlen(string);

		// char* widthbuf = char[8];
		// char* precsbuf = char[8];

		char widthbuf[8] = { 0 };
		char precsbuf[8] = { 0 };

		// %[parameter][flags][width][.precision][length]type

		for(uint64_t i = 0; i < length; i++)
		{
			c = string[i];

			if(!IsFormat && c == '%')
			{
				IsFormat = true;
				continue;
			}
			if(IsFormat)
			{
				switch(c)
				{
					case '%':
						PrintChar('%', pf);
						break;

					// Standard, parameter types

					case 'd':
					case 'i':
					case 'u':
						z = ArgSize == 0 ? va_arg(args, int32_t) : va_arg(args, int64_t);
						if(DisplaySign)
						{
							if(z > 0)
								PrintChar('+', pf);

							DisplaySign = false;
						}

						if(PadSignedSpace)
						{
							PadSignedSpace = false;
							if(z < 0)
								PrintChar(' ', pf);
						}

						PrintedChars = PrintInteger_Signed(z, !LeftAlign && PadZeroes ? Width : -1, SwallowChar);

						// check if we need to align left.
						if(LeftAlign && Width > 0)
						{
							PrintInteger_Signed(z, !LeftAlign && PadZeroes ? Width : -1, pf);
							LeftAlign = false;
							for(uint64_t tps = 0; tps < (uint8_t) Width - PrintedChars; tps++)
							{
								PrintChar(' ', pf);
							}
						}

						else if(Width > 0 && !LeftAlign && !PadZeroes)
						{
							for(uint64_t tps = 0; tps < (uint8_t) Width - PrintedChars; tps++)
							{
								PrintChar(' ', pf);
							}

							PrintInteger_Signed(z, -1, pf);
						}

						else
						{
							PrintInteger_Signed(z, Width, pf);
						}

						PadZeroes = false;
						break;

					case 's':
						s = va_arg(args, char*);

						// check to pad with spaces.
						if(Width > 0)
						{
							int64_t wd = Width;
							for(int64_t m = 0; m < wd; m++)
								PrintChar(' ', pf);
						}

						PrintString(s, Precision, pf);
						break;


					case 'X':
					case 'x':
					case 'p':
						x = va_arg(args, uint64_t);

						if(c == 'p')
							Precision = 16;

						// now handle the padding rubbish.
						// check if we need to align left.
						if(LeftAlign && Width > 0)
						{
							PrintedChars = PrintHex_Precision_NoPrefix(x, -1, false, c == 'X', PadZeroes, Precision, !OmitZeroX, pf);

							LeftAlign = false;
							for(uint64_t tps = 0; tps < (uint8_t) Width - PrintedChars; tps++)
							{
								PrintChar(' ', pf);
							}
						}
						else
						{
							// handle cases where we use width instead.
							if(Width > 0)
								Precision = Width;

							PrintHex_Precision_NoPrefix(x, Width > 0 ? Width : 1, false, c == 'X', PadZeroes, Precision, !OmitZeroX, pf);
						}

						PadZeroes = false;
						OmitZeroX = false;
						break;

					case 'c':
						ch = (char) va_arg(args, int);
						PrintChar((uint8_t) ch, pf);
						break;


					case 'f':
					case 'F':
						if(ArgSize == 1)
							f = va_arg(args, double);

						else if(ArgSize == 0)
							f = va_arg(args, double);

						else
							f = va_arg(args, double);

						if(Precision > 0)
						{
							uint8_t remaining = PrintFloat(f, Precision, pf);
							for(; remaining > 0; remaining--)
								PrintChar('0', pf);
						}
						else
						{
							PrintFloat(f, Precision, pf);
						}
						break;

					case 'b':
						b = va_arg(args, int);
						PrintString(b ? "true" : "false", -1, pf);
						break;


					case 'z':
					case 'j':
					case 't':
					case 'h':
						continue;



					// Argument sizes
					case 'l':
					case 'L':
						ArgSize = 1;
						continue;


					// Flags
					case '+':
						DisplaySign = true;
						IsFormat = true;
						continue;

					// Note this is the reverse of standard behaviour; we'll print '0x' every time unless # is specified.
					case '#':
						OmitZeroX = true;
						IsFormat = true;
						continue;

					case '^':
						IsFormat = true;
						continue;

					case '-':
						LeftAlign = true;
						IsFormat = true;
						continue;

					case ' ':
						PadSignedSpace = true;
						IsFormat = true;
						continue;


					// Width/precision
					case '.':
						IsParsingPrecision = true;
						IsFormat = true;
						continue;


					default:
						if(IsParsingPrecision)
						{
							int z1 = 0;
							uint64_t f1 = i;
							for(z1 = 0, f1 = i; ((string[f1] >= '0') && (string[f1] <= '9')) || string[f1] == '*'; z1++, f1++)
							{
								precsbuf[z1] = string[f1];
							}
							if(precsbuf[0] == '*')
								Precision = (int8_t) va_arg(args, uint64_t);

							else
								Precision = (int8_t) Utility::ConvertToInt(precsbuf, 10);

							// -1 because continue; increments i
							i = f1 - 1;
							IsFormat = true;

							Memory::Set(precsbuf, 0, 8);
							IsParsingPrecision = false;

							continue;
						}
						else
						{
							int z1 = 0;
							uint64_t f1 = i;
							for(z1 = 0, f1 = i; ((string[f1] >= '0') && (string[f1] <= '9')) || string[f1] == '*'; z1++, f1++)
							{
								widthbuf[z1] = string[f1];
							}

							if(widthbuf[0] == '0')
								PadZeroes = true;

							if(widthbuf[0] == '*')
								Width = (int8_t) va_arg(args, uint64_t);

							else
								Width = (int8_t) Utility::ConvertToInt(widthbuf, 10);



							// -1 because continue; increments i
							i = f1 - 1;
							IsFormat = true;

							Memory::Set(widthbuf, 0, 8);
							continue;
						}
						break;
				}
				IsFormat = false;
				DisplaySign = false;

				Precision = -1;
				Width = -1;
				IsParsingPrecision = false;
			}
			else
			{
				PrintChar((uint8_t) c, pf);
			}
		}
	}
}
}

#endif

