/*

   Copyright 2009 Pierre KRIEGER

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

*/

#ifndef _STDC_LIMITS_H_
#define _STDC_LIMITS_H_

#include "machine/_sizes.h"

#define CHAR_BIT		__CHAR_BIT__

#define SCHAR_MIN		(-__SCHAR_MAX__ - 1)
#define SCHAR_MAX		__SCHAR_MAX__
#define UCHAR_MAX		((1 << CHAR_BIT) - 1)

#define SHRT_MIN		(-__SHRT_MAX__ - 1)
#define SHRT_MAX		__SHRT_MAX__
#define USHRT_MAX		((1 << (CHAR_BIT * __SIZEOF_SHORT__)) - 1)

#define INT_MIN			(-__INT_MAX__ - 1)
#define INT_MAX			__INT_MAX__
#define UINT_MAX		((1 << (CHAR_BIT * __SIZEOF_INT__)) - 1)

#define LONG_MIN		(-__LONG_MAX__ - 1)
#define LONG_MAX		__LONG_MAX__
#define ULONG_MAX		((1 << (CHAR_BIT * __SIZEOF_LONG__)) - 1)

#define LLONG_MIN		(-__LLONG_MAX__ - 1)
#define LLONG_MAX		__LLONG_MAX__
#define ULLONG_MAX		((1 << (CHAR_BIT * __SIZEOF_LONG_LONG__)) - 1)

#endif

