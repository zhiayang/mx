// StandardIO.cpp
// Copyright (c) 2011 - 2014, Jonas 'Sortie' Termansen
// Licensed under the GNU LGPL. Refer to Libraries/libc/stdio/_format.cpp for original license text.

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

size_t convert_integer(char* destination, uint64_t value, uint64_t base, const char* digits)
{
	size_t result = 1;
	uint64_t copy = value;
	while(base <= copy)
	{
		copy /= base;
		result++;
	}

	for(size_t i = result; i != 0; i--)
	{
		destination[i - 1] = digits[value % base];
		value /= base;
	}

	return result;
}

static size_t noop_callback(void*, const char*, size_t amount)
{
	return amount;
}

static size_t callback_character(size_t (*callback)(void*, const char*, size_t), void* user, char c)
{
	return callback(user, &c, 1);
}



// note: this is patched below to give our custom behaviour, namely when printing hex:
// caps with 'x', small with 'X'
// 0x without '#', disable with '#'
size_t _vprintf_callback(size_t (*callback)(void*, const char*, size_t), void* user, const char* format, va_list parameters)
{
	if(!callback)
		callback = noop_callback;

	size_t written = 0;
	bool rejected_bad_specifier = false;

	while(*format != '\0')
	{
		if(*format != '%')
		{
			print_c:

			size_t amount = 1;
			while(format[amount] && format[amount] != '%')
				amount++;

			if(callback(user, format, amount) != amount)
				return LONG_MAX;

			format += amount;
			written += amount;
			continue;
		}

		const char* format_begun_at = format;

		if(*(++format) == '%')
			goto print_c;

		if(rejected_bad_specifier)
		{
			incomprehensible_conversion:
			rejected_bad_specifier = true;

			unsupported_conversion:
			format = format_begun_at;
			goto print_c;
		}

		bool alternate = false;
		bool zero_pad = false;
		bool field_width_is_negative = false;
		bool prepend_blank_if_positive = false;
		bool prepend_plus_if_positive = false;
		bool group_thousands = false;
		bool alternate_output_digits = false;

		(void) group_thousands;
		(void) alternate_output_digits;

		while(true)
		{
			switch(*format++)
			{
				case '#': alternate = true; continue;
				case '0': zero_pad = true; continue;
				case '-': field_width_is_negative = true; continue;
				case ' ': prepend_blank_if_positive = true; continue;
				case '+': prepend_plus_if_positive = true; continue;
				case '\'': group_thousands = true; continue;
				case 'I': alternate_output_digits = true; continue;
				default: format--; break;
			}
			break;
		}

		int field_width = 0;
		if(*format == '*' && (format++, true))
			field_width = va_arg(parameters, int);

		else while('0' <= *format && *format <= '9')
			field_width = 10 * field_width + *format++ - '0';

		if(field_width_is_negative)
			field_width = -field_width;

		size_t abs_field_width = (size_t) abs(field_width);

		size_t precision = LONG_MAX;
		if(*format == '.' && (format++, true))
		{
			precision = 0;
			if(*format == '*' && (format++, true))
			{
				int int_precision = va_arg(parameters, int);
				precision = 0 <= int_precision ? (size_t) int_precision : 0;
			}
			else
			{
				if(*format == '-' && (format++, true))
					while('0' <= *format && *format <= '9')
						format++;
				else
					while('0' <= *format && *format <= '9')
						precision = 10 * precision + *format++ - '0';
			}
		}

		enum length
		{
			LENGTH_SHORT_SHORT,
			LENGTH_SHORT,
			LENGTH_DEFAULT,
			LENGTH_LONG,
			LENGTH_LONG_LONG,
			LENGTH_LONG_DOUBLE,
			LENGTH_INTMAX_T,
			LENGTH_SIZE_T,
			LENGTH_PTRDIFF_T,
		};

		struct length_modifer
		{
			const char* name;
			enum length length;
		};

		struct length_modifer length_modifiers[] =
		{
			{ "hh", LENGTH_SHORT_SHORT },
			{ "h", LENGTH_SHORT },
			{ "", LENGTH_DEFAULT },
			{ "l", LENGTH_LONG },
			{ "ll", LENGTH_LONG_LONG },
			{ "L", LENGTH_LONG_DOUBLE },
			{ "j", LENGTH_INTMAX_T },
			{ "z", LENGTH_SIZE_T },
			{ "t", LENGTH_PTRDIFF_T },
		};

		enum length length = LENGTH_DEFAULT;
		size_t length_length = 0;
		for(size_t i = 0; i < sizeof(length_modifiers) / sizeof(length_modifiers[0]); i++)
		{
			size_t name_length = strlen(length_modifiers[i].name);
			if(name_length < length_length)
				continue;

			if(strncmp(format, length_modifiers[i].name, name_length) != 0)
				continue;

			length = length_modifiers[i].length;
			length_length = name_length;
		}

		format += length_length;

		if(*format == 'd' || *format == 'i' || *format == 'o' || *format == 'u' || *format == 'x' || *format == 'X' || *format == 'p')
		{
			char conversion = *format++;

			bool negative_value = false;
			uint64_t value;
			if(conversion == 'p')
			{
				value = (uint64_t) va_arg(parameters, void*);
				conversion = 'x';
				alternate = false;
				prepend_blank_if_positive = false;
				prepend_plus_if_positive = false;
			}
			else if(conversion == 'i' || conversion == 'd')
			{
				intmax_t signed_value;
				if(length == LENGTH_SHORT_SHORT)
					signed_value = va_arg(parameters, int);

				else if(length == LENGTH_SHORT)
					signed_value = va_arg(parameters, int);

				else if(length == LENGTH_DEFAULT)
					signed_value = va_arg(parameters, int);

				else if(length == LENGTH_LONG)
					signed_value = va_arg(parameters, long);

				else if(length == LENGTH_LONG_LONG)
					signed_value = va_arg(parameters, long long);

				else if(length == LENGTH_INTMAX_T)
					signed_value = va_arg(parameters, intmax_t);

				else if(length == LENGTH_SIZE_T)
					signed_value = va_arg(parameters, size_t);

				else if(length == LENGTH_PTRDIFF_T)
					signed_value = va_arg(parameters, ptrdiff_t);

				else
					goto incomprehensible_conversion;

				value = (negative_value = signed_value < 0) ? -(uint64_t) signed_value : (uint64_t) signed_value;
			}
			else
			{
				if(length == LENGTH_SHORT_SHORT)
					value = va_arg(parameters, unsigned int);

				else if(length == LENGTH_SHORT)
					value = va_arg(parameters, unsigned int);

				else if(length == LENGTH_DEFAULT)
					value = va_arg(parameters, unsigned int);

				else if(length == LENGTH_LONG)
					value = va_arg(parameters, unsigned long);

				else if(length == LENGTH_LONG_LONG)
					value = va_arg(parameters, unsigned long long);

				else if(length == LENGTH_INTMAX_T)
					value = va_arg(parameters, uint64_t);

				else if(length == LENGTH_SIZE_T)
					value = va_arg(parameters, size_t);

				else if(length == LENGTH_PTRDIFF_T)
					value = (uint64_t) va_arg(parameters, ptrdiff_t);

				else
					goto incomprehensible_conversion;

				prepend_blank_if_positive = false;
				prepend_plus_if_positive = false;
			}

			// note: flipped here
			// used to be (conversion == 'X' ? ... )
			const char* digits = conversion == 'x' ? "0123456789ABCDEF" : "0123456789abcdef";

			uint64_t base = (conversion == 'x' || conversion == 'X') ? 16 : conversion == 'o' ? 8 : 10;
			char prefix[3];
			size_t prefix_length = 0;
			size_t prefix_digits_length = 0;

			if(negative_value)
				prefix[prefix_length++] = '-';
			else if(prepend_plus_if_positive)
				prefix[prefix_length++] = '+';
			else if(prepend_blank_if_positive)
				prefix[prefix_length++] = ' ';

			// and here.
			if(!alternate /* (used to be if(alternate ...) */ && (conversion == 'x' || conversion == 'X' || conversion == 'p') && value != 0)
			{
				prefix[prefix_digits_length++, prefix_length++] = '0';
				prefix[prefix_digits_length++, prefix_length++] = conversion;
			}
			if(alternate && conversion == 'o' && value != 0)
				prefix[prefix_digits_length++, prefix_length++] = '0';

			char output[sizeof(uint64_t) * 3];
			size_t output_length = convert_integer(output, value, base, digits);
			if(!precision && output_length == 1 && output[0] == '0')
			{
				output_length = 0;
				output[0] = '\0';
			}

			size_t output_length_with_precision = precision != LONG_MAX && output_length < precision ? precision : output_length;

			size_t digits_length = prefix_digits_length + output_length;
			size_t normal_length = prefix_length + output_length;
			size_t length_with_precision = prefix_length + output_length_with_precision;

			bool use_precision = precision != LONG_MAX;
			bool use_zero_pad = zero_pad && 0 <= field_width && !use_precision;
			bool use_left_pad = !use_zero_pad && 0 <= field_width;
			bool use_right_pad = !use_zero_pad && field_width < 0;

			if(use_left_pad)
			{
				for(size_t i = length_with_precision; i < abs_field_width; i++)
				{
					if(callback_character(callback, user, ' ') != 1)
						return LONG_MAX;
					else
						written++;
				}
			}
			if(callback(user, prefix, prefix_length) != prefix_length)
				return LONG_MAX;

			written += prefix_length;
			if(use_zero_pad)
			{
				for(size_t i = normal_length; i < abs_field_width; i++)
				{
					if(callback_character(callback, user, '0') != 1)
						return LONG_MAX;
					else
						written++;
				}
			}

			if(use_precision)
			{
				for(size_t i = digits_length; i < precision; i++)
				{
					if(callback_character(callback, user, '0') != 1)
						return LONG_MAX;
					else
						written++;
				}
			}

			if(callback(user, output, output_length) != output_length)
				return LONG_MAX;
			written += output_length;
			if(use_right_pad)
			{
				for(size_t i = length_with_precision; i < abs_field_width; i++)
				{
					if(callback_character(callback, user, ' ') != 1)
						return LONG_MAX;
					else
						written++;
				}
			}
		}

		else if(*format == 'e' || *format == 'E' || *format == 'f' || *format == 'F' || *format == 'g' || *format == 'G' || *format == 'a' || *format == 'A')
		{
			char conversion = *format++;

			long double value;
			if(length == LENGTH_DEFAULT)
				value = va_arg(parameters, double);

			else if(length == LENGTH_LONG_DOUBLE)
				value = va_arg(parameters, long double);

			else
				goto incomprehensible_conversion;

			// TODO: Implement floating-point printing.
			(void) conversion;
			(void) value;

			goto unsupported_conversion;
		}

		else if(*format == 'c' && (format++, true))
		{
			char c;
			if(length == LENGTH_DEFAULT)
				c = (char) va_arg(parameters, int);

			// else if(length == LENGTH_LONG)
			// {
			// 	// TODO: Implement wide character printing.
			// 	(void) va_arg(parameters, wint_t);
			// 	goto unsupported_conversion;
			// }
			else
				goto incomprehensible_conversion;

			if(!field_width_is_negative && 1 < abs_field_width)
			{
				for(size_t i = 1; i < abs_field_width; i++)
				{
					if(callback_character(callback, user, ' ') != 1)
						return LONG_MAX;
					else
						written++;
				}
			}

			if(callback(user, &c, 1) != 1)
				return LONG_MAX;

			written++;

			if(field_width_is_negative && 1 < abs_field_width)
			{
				for(size_t i = 1; i < abs_field_width; i++)
				{
					if(callback_character(callback, user, ' ') != 1)
						return LONG_MAX;
					else
						written++;
				}
			}
		}
		else if(*format == 'm' || *format == 's')
		{
			char conversion = *format++;

			const char* string;
			// if(conversion == 'm')
			// 	string = strerror(errno), conversion = 's';

			if(length == LENGTH_DEFAULT)
				string = va_arg(parameters, const char*);

			// else if(length == LENGTH_LONG)
			// {
			// 	// TODO: Implement wide character string printing.
			// 	(void) va_arg(parameters, const wchar_t*);
			// 	goto unsupported_conversion;
			// }
			else
				goto incomprehensible_conversion;

			if(conversion == 's' && !string)
				string = "(null)";

			size_t string_length = 0;
			for(size_t i = 0; i < precision && string[i]; i++)
				string_length++;

			if(!field_width_is_negative && string_length < abs_field_width)
			{
				for(size_t i = string_length; i < abs_field_width; i++)
				{
					if(callback_character(callback, user, ' ') != 1)
						return LONG_MAX;
					else
						written++;
				}
			}

			if(callback(user, string, string_length) != string_length)
				return LONG_MAX;

			written += string_length;

			if(field_width_is_negative && string_length < abs_field_width)
			{
				for(size_t i = string_length; i < abs_field_width; i++)
				{
					if(callback_character(callback, user, ' ') != 1)
						return LONG_MAX;
					else
						written++;
				}
			}
		}
		else if(*format == 'n' && (format++, true))
		{
			if(length == LENGTH_SHORT_SHORT)
				*va_arg(parameters, signed char*) = (signed char) written;

			else if(length == LENGTH_SHORT)
				*va_arg(parameters, short*) = (short) written;

			else if(length == LENGTH_DEFAULT)
				*va_arg(parameters, int*) = (int) written;

			else if(length == LENGTH_LONG)
				*va_arg(parameters, long*) = (long) written;

			else if(length == LENGTH_LONG_LONG)
				*va_arg(parameters, long long*) = (long long) written;

			else if(length == LENGTH_INTMAX_T)
				*va_arg(parameters, uint64_t*) = (uint64_t) written;

			else if(length == LENGTH_SIZE_T)
				*va_arg(parameters, size_t*) = (size_t) written;

			else if(length == LENGTH_PTRDIFF_T)
				*va_arg(parameters, ptrdiff_t*) = (ptrdiff_t) written;

			else
				goto incomprehensible_conversion;
		}
		else
			goto incomprehensible_conversion;
	}

	return written;
}

size_t _printf_callback(size_t (*callback)(void*, const char*, size_t), void* user, const char* format, ...)
{
	va_list list;
	va_start(list, format);

	auto ret = _vprintf_callback(callback, user, format, list);

	va_end(list);

	return ret;
}













namespace Kernel {
namespace Console
{
	void PrintChar(uint8_t c);
}
}


static size_t ConsolePrintCallback(void* user, const char* string, size_t stringlen)
{
	for(size_t i = 0; i < stringlen; i++)
		Kernel::Console::PrintChar(string[i]);

	return stringlen;
}

static size_t CallbackTransform(void* user, const char* string, size_t stringlen)
{
	void (*pf)(uint8_t) = (void (*)(uint8_t)) user;

	for(size_t i = 0; i < stringlen; i++)
		pf(string[i]);

	return stringlen;
}


namespace StdIO
{
	void PrintFmt(void (*pf)(uint8_t), const char* fmt, va_list list)
	{
		_vprintf_callback(CallbackTransform, (void*) pf, fmt, list);
	}

	void PrintFmt(const char* fmt, va_list list)
	{
		_vprintf_callback(ConsolePrintCallback, 0, fmt, list);
	}



	void PrintFmt(void (*pf)(uint8_t), const char* fmt, ...)
	{
		va_list list;
		va_start(list, fmt);

		PrintFmt(pf, fmt, list);

		va_end(list);
	}

	void PrintFmt(const char* fmt, ...)
	{
		va_list list;
		va_start(list, fmt);

		PrintFmt(fmt, list);

		va_end(list);
	}

	size_t PrintStr(const char* s, size_t len)
	{
		ConsolePrintCallback(0, s, len);

		return len;
	}

	size_t PrintStr(uint8_t* s, size_t len)
	{
		ConsolePrintCallback(0, (const char*) s, len);

		return len;
	}
}






















int _vprintf(const char* format, va_list list)
{
	size_t result = _vprintf_callback(ConsolePrintCallback, 0, format, list);
	if(result == LONG_MAX)
		return -1;

	return (int) result;
}

int _printf(const char* format, ...)
{
	va_list list;
	va_start(list, format);
	int result = _vprintf(format, list);
	va_end(list);
	return result;
}












typedef struct vsnprintf_struct
{
	char* str;
	size_t size;
	size_t produced;
	size_t written;

} vsnprintf_t;

static size_t StringPrintCallback(void* user, const char* string, size_t stringlen)
{
	vsnprintf_t* info = (vsnprintf_t*) user;
	if(info->produced < info->size)
	{
		size_t available = info->size - info->produced;
		size_t possible = (stringlen < available) ? stringlen : available;
		memcpy(info->str + info->produced, string, possible);
		info->written += possible;
	}

	info->produced += stringlen;
	return stringlen;
}

int _vsnprintf(char* str, size_t size, const char* format, va_list list)
{
	vsnprintf_t info;
	info.str = str;
	info.size = size ? size - 1 : 0;
	info.produced = 0;
	info.written = 0;

	_vprintf_callback(StringPrintCallback, &info, format, list);

	if(size)
	{
		info.str[info.written] = '\0';
	}

	return (int) info.produced;
}

int _snprintf(char* str, size_t size, const char* format, ...)
{
	va_list list;
	va_start(list, format);
	int result = _vsnprintf(str, size, format, list);

	va_end(list);
	return result;
}

int _vsprintf(char* str, const char* format, va_list ap)
{
	return _vsnprintf(str, LONG_MAX, format, ap);
}

int _sprintf(char* str, const char* format, ...)
{
	va_list list;
	va_start(list, format);
	int result = _vsprintf(str, format, list);

	va_end(list);
	return result;
}




















