/*
 * Acess2 C++ Library
 * - By John Hodge (thePowersGang)
 *
 * initialiser_list (header)
 * - C++'s initialiser_list type
 */

#ifndef STL_INITIALISER_LIST_H
#define STL_INITIALISER_LIST_H

#include <stdint.h>
#include <stddef.h>

namespace stl
{
	template <class T>
	class initialiser_list
	{
	public:
		typedef T			value_type;
		typedef const T&	reference;
		typedef const T&	const_reference;
		typedef size_t		size_type;
		typedef const T*	iterator;
		typedef const T*	const_iterator;
	private:
		// ORDER MATTERS : The first item must be a pointer to the array, the second must be the size
		value_type*	m_values;
		size_type	m_len;
	public:
		constexpr initialiser_list() noexcept : m_len(0)
		{
		}

		size_type size() const noexcept
		{
			return m_len;
		}

		const T* begin() const noexcept
		{
			return &m_values[0];
		}
		const T* end() const noexcept
		{
			return &m_values[m_len];
		}
	};

}

template <class T> const T* begin(const ::stl::initialiser_list<T>& il) { return il.begin(); }
template <class T> const T* end  (const ::stl::initialiser_list<T>& il) { return il.end(); }

#endif










