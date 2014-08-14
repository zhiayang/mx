#ifndef _STDC_STDARG_H_
#define _STDC_STDARG_H_

#include "stddef.h"

#define va_list					__builtin_va_list
#define va_start				__builtin_va_start
#define va_end					__builtin_va_end
#define va_arg					__builtin_va_arg

#endif
