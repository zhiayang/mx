#ifndef RDESTL_COMMON_H
#define RDESTL_COMMON_H

#ifndef RDESTL_STANDALONE
	#define RDESTL_STANDALONE	1
#endif

// comment
#if RDESTL_STANDALONE
#   include <assert.h>
#   include <string.h>
#	define RDE_FORCEINLINE	inline

#	ifdef _DEBUG
#		undef RDE_DEBUG
#		define RDE_DEBUG	1
#	endif

#	define RDE_ASSERT	assert
#	define RDE_DEBUG	0

	namespace rde
	{
		// # Meh. MSVC doesnt seem to have <stdint.h>
		// @todo	Fixes to make this portable.
		namespace Sys
		{
			RDE_FORCEINLINE void MemCpy(void* to, const void* from, size_t bytes)
			{
				memcpy(to, from, bytes);
			}
			RDE_FORCEINLINE void MemMove(void* to, const void* from, size_t bytes)
			{
				memmove(to, from, bytes);
			}
			RDE_FORCEINLINE void MemSet(void* buf, unsigned char value, size_t bytes)
			{
				memset(buf, value, bytes);
			}
		} // sys
	}
#else
#	include "core/RdeAssert.h"
#	include "core/System.h"
#endif

namespace rde
{
enum e_noinitialize
{
	noinitialize
};
}

#endif // #ifndef RDESTL_H
