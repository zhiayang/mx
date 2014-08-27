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

#ifndef _STDC_STDINT_H_
#define _STDC_STDINT_H_

#include "machine/_sizes.h"


#if	__CHAR_BIT__ == 8
typedef char				int8_t;
typedef unsigned char			uint8_t;
#endif

#if	__CHAR_BIT__ >= 8
typedef char				int_least8_t;
typedef unsigned char			uint_least8_t;
#else
#error Could not determine int_least8_t and uint_least8_t
#endif


#if	__CHAR_BIT__ == 16
typedef char				int16_t;
typedef unsigned char			uint16_t;
typedef int16_t				int_least16_t;
typedef uint16_t			uint_least16_t;

#elif	__CHAR_BIT__ * __SIZEOF_SHORT__ == 16
typedef short				int16_t;
typedef unsigned short		uint16_t;
typedef int16_t				int_least16_t;
typedef uint16_t			uint_least16_t;

#elif	__CHAR_BIT__ * __SIZEOF_INT__ == 16
typedef int					int16_t;
typedef unsigned int			uint16_t;
typedef int16_t				int_least16_t;
typedef uint16_t			uint_least16_t;

#elif	__CHAR_BIT__ * __SIZEOF_LONG__ == 16
typedef long				int16_t;
typedef unsigned long		uint16_t;
typedef int16_t				int_least16_t;
typedef uint16_t			uint_least16_t;

#elif	__CHAR_BIT__ * __SIZEOF_LONG_LONG__ == 16
typedef long long			int16_t;
typedef unsigned long long		uint16_t;
typedef int16_t				int_least16_t;
typedef uint16_t			uint_least16_t;

#elif	__CHAR_BIT__ * __SIZEOF_LONG_LONG__ >= 16
typedef long long			int_least16_t;
typedef unsigned long long		uint_least16_t;

#else
#error Could not determine int_least16_t and uint_least16_t
#endif


#if	__CHAR_BIT__ == 32
typedef char				int32_t;
typedef unsigned char			uint32_t;
typedef int32_t				int_least32_t;
typedef uint32_t			uint_least32_t;

#elif	__CHAR_BIT__ * __SIZEOF_SHORT__ == 32
typedef short				int32_t;
typedef unsigned short			uint32_t;
typedef int32_t				int_least32_t;
typedef uint32_t			uint_least32_t;

#elif	__CHAR_BIT__ * __SIZEOF_INT__ == 32
typedef int					int32_t;
typedef unsigned int			uint32_t;
typedef int32_t				int_least32_t;
typedef uint32_t			uint_least32_t;

#elif	__CHAR_BIT__ * __SIZEOF_LONG__ == 32
typedef long				int32_t;
typedef unsigned long			uint32_t;
typedef int32_t				int_least32_t;
typedef uint32_t			uint_least32_t;

#elif	__CHAR_BIT__ * __SIZEOF_LONG_LONG__ == 32
typedef long long			int32_t;
typedef unsigned long long		uint32_t;
typedef int32_t				int_least32_t;
typedef uint32_t			uint_least32_t;

#elif	__CHAR_BIT__ * __SIZEOF_LONG_LONG__ >= 32
typedef long long			int_least32_t;
typedef unsigned long long		uint_least32_t;

#else
#error Could not determine int_least32_t and uint_least32_t
#endif


#if	__CHAR_BIT__ == 64
typedef char				int64_t;
typedef unsigned char			uint64_t;
typedef int64_t				int_least64_t;
typedef uint64_t			uint_least64_t;

#elif	__CHAR_BIT__ * __SIZEOF_SHORT__ == 64
typedef short				int64_t;
typedef unsigned short			uint64_t;
typedef int64_t				int_least64_t;
typedef uint64_t			uint_least64_t;

#elif	__CHAR_BIT__ * __SIZEOF_INT__ == 64
typedef int						int64_t;
typedef unsigned int			uint64_t;
typedef int64_t				int_least64_t;
typedef uint64_t			uint_least64_t;

#elif	__CHAR_BIT__ * __SIZEOF_LONG__ == 64
typedef long				int64_t;
typedef unsigned long			uint64_t;
typedef int64_t				int_least64_t;
typedef uint64_t			uint_least64_t;

#elif	__CHAR_BIT__ * __SIZEOF_LONG_LONG__ == 64
typedef long long			int64_t;
typedef unsigned long long		uint64_t;
typedef int64_t				int_least64_t;
typedef uint64_t			uint_least64_t;

#elif	__CHAR_BIT__ * __SIZEOF_LONG_LONG__ >= 64
typedef long long			int_least64_t;
typedef unsigned long long		uint_least64_t;

#else
#error Could not determine int_least64_t and uint_least64_t
#endif


typedef int_least8_t			int_fast8_t;
typedef uint_least8_t			uint_fast8_t;
typedef int_least16_t			int_fast16_t;
typedef uint_least16_t			uint_fast16_t;
typedef int_least32_t			int_fast32_t;
typedef uint_least32_t			uint_fast32_t;
typedef int_least64_t			int_fast64_t;
typedef uint_least64_t			uint_fast64_t;


typedef long int intmax_t;
typedef unsigned long int uintmax_t;

#if	__SIZEOF_POINTER__ <= 1
typedef char				intptr_t;
typedef unsigned char			uintptr_t;

#elif	__SIZEOF_POINTER__ <= __SIZEOF_SHORT__
typedef short				intptr_t;
typedef unsigned short			uintptr_t;

#elif	__SIZEOF_POINTER__ <= __SIZEOF_INT__
typedef int				intptr_t;
typedef unsigned int			uintptr_t;

#elif	__SIZEOF_POINTER__ <= __SIZEOF_LONG__
typedef long				intptr_t;
typedef unsigned long			uintptr_t;

#elif	__SIZEOF_POINTER__ <= __SIZEOF_LONG_LONG__
typedef long long			intptr_t;
typedef unsigned long long		uintptr_t;
#endif


/* typedef some dumb things */
typedef uint8_t		__uint8_t;
typedef int8_t		__int8_t;
typedef uint16_t	__uint16_t;
typedef int16_t		__int16_t;
typedef uint32_t	__uint32_t;
typedef int32_t		__int32_t;
typedef uint64_t	__uint64_t;
typedef int64_t		__int64_t;


#endif
