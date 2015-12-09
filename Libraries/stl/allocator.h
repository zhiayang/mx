/*
 * Acess2 C++ Library
 * - By John Hodge (thePowersGang)
 *
 * string (header)
 * - C++'s allocator
 */

#ifndef STL_ALLOCATOR_H
#define STL_ALLOCATOR_H

#include <stddef.h>
#include "_libcxx_helpers.h"
#include "utility.h"

namespace stl {

template <class T> class allocator;
namespace _bits
{
	template <class T>
	class allocator_real
	{
		public:
			typedef T			value_type;
			typedef T*			pointer;
			typedef T&			reference;
			typedef const T*	const_pointer;
			typedef const T&	const_reference;
			typedef size_t		size_type;
			typedef ptrdiff_t	difference_type;

			template <class Type>
			struct rebind
			{
				typedef allocator<Type> other;
			};

			allocator_real()
			{
			}

			allocator_real(const allocator_real& alloc)
			{
				(void) alloc;
			}

			template <class U>
			allocator_real(const allocator_real<U>& alloc)
			{
				(void) alloc;
			}

			~allocator_real()
			{
			}

			pointer address(reference x) const
			{
				return &x;
			}

			const_pointer address(const_reference x) const
			{
				return &x;
			}

			static pointer allocate(size_type n, const void* hint = 0)
			{
				(void) hint;
				// hint = hint;
				return static_cast<pointer>( ::operator new (n * sizeof(value_type)) );
			}

			static void deallocate(pointer p, size_type n)
			{
				(void) n;
				// n = n;
				::operator delete(p);
			}

			size_type max_size()
			{
				return ((size_type) -1) / sizeof(value_type);
			}

			void construct(pointer p, const_reference val)
			{
				new ((void*) p) value_type (val);
			}

			// C++11
			#if _CXX11_AVAIL
			template<class U, class... Args>
			void construct(U* p, Args&&... args)
			{
				::new ((void*) p) U (::stl::forward<Args>(args)...);
			}
			#endif

			void destroy(pointer p)
			{
				p->~value_type();
			}
	};

	template <class T>
	class allocator_noconstruct : public ::stl::_bits::allocator_real<T>
	{
		public:
			void construct(typename allocator_real<T>::pointer p, typename allocator_real<T>::const_reference val)
			{
				*p = val;
			}
			void destroy(typename allocator_real<T>::pointer p)
			{
				(void) p;
			}
	};

}

template <class T>
class allocator : public _bits::allocator_real<T>
{
};

// #if 1
template <>
class allocator<unsigned char> : public _bits::allocator_noconstruct<unsigned char>
{
};

template <>
class allocator<unsigned long> : public _bits::allocator_noconstruct<unsigned long>
{
};

#ifndef TINYSTL_ALLOCATOR
#define TINYSTL_ALLOCATOR ::stl::allocator
#endif

// #endif

}
#endif









