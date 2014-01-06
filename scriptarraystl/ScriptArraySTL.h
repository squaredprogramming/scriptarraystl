// This is aimple wrapper for CScriptArray so it can be used like std::vector when programming in C++.
// This does not affect how arrays are used in AngelScript and the CScriptArray addon must be registered
// with the script engine before this can be used.
// Before accessing the data in the array, InitArray() should be called with the script engine, the
// AngelScript declaration for the array (ie "array<int>"), and optionally, the starting size of the array.
// When the C++ programmer has finished with the array, he/she should call Release().
// This class was written by Dominque Douglas
// squaredprogramming@gmail.com
// Copyright (c) 2014, Dominque A Douglas
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer
//    in the documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
// OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
#pragma once

#include <memory>
#include <stdexcept>

#include "..\scriptarray\scriptarray.h"

#define ASSERT_IF_UNITIALIZED


template <class T, class TArrayClass = CScriptArray>
class CScriptArraySTL
{
public:
	typedef T          value_type;
	typedef T         *pointer;
	typedef const T   *const_pointer;
	typedef T         &reference;
	typedef const T   &const_reference;
	typedef size_t     size_type;
	typedef ptrdiff_t  difference_type;

	// iterator -------------------------------------------------------------------------------
	template <class Ty, class TScriptArrayClass>
	class CScriptArraySTL_iterator : public std::iterator<std::random_access_iterator_tag, Ty>
	{
		private:
			TScriptArrayClass	*_buf;
			size_type			 _pos;

		public:
			// b should be a null terminated string in UTF-8
			// if this is the end start_pos should be the index of the null terminator
			// start_pos should be the valid start of a character
			CScriptArraySTL_iterator(TScriptArrayClass	*b, size_type p)
				:_buf(b), _pos(p)
			{
			}

			// copy constructor
			CScriptArraySTL_iterator(const CScriptArraySTL_iterator &other)
				:_buf(other._buf), _pos(other._pos)
			{
			}

			reference operator*() const
			{
				// returns the character currently being pointed to
				return (*_buf)[_pos];
			}

			// does not check to see if it goes past the end
			// iterating past the end is undefined
			CScriptArraySTL_iterator &operator++()
			{
				++_pos;

				return *this;
			}

			// does not check to see if it goes past the end
			// iterating past the end is undefined
			CScriptArraySTL_iterator operator++(int)
			{
				CScriptArraySTL_iterator copy(*this);

				// increment
				++_pos;

				return copy;
			}

			// does not check to see if it goes past the end
			// iterating past begin is undefined
			CScriptArraySTL_iterator &operator--()
			{
				--_pos;
				return *this;
			}

			// does not check to see if it goes past the end
			// iterating past begin is undefined
			CScriptArraySTL_iterator operator--(int)
			{
				CScriptArraySTL_iterator copy(*this);

				--_pos;

				return copy;
			}

			bool operator == ( const CScriptArraySTL_iterator &other) const
			{
				return (_buf == other._buf) && (_pos == other._pos);
			}
 
			bool operator != (const CScriptArraySTL_iterator &other) const
			{
				return (_buf != other._buf) || (_pos != other._pos);
			}

			bool operator < ( const CScriptArraySTL_iterator &other) const
			{
				return _pos < other._pos;
			}
 
			bool operator > (const CScriptArraySTL_iterator &other) const
			{
				return _pos > other._pos;
			}
		
			bool operator <= ( const CScriptArraySTL_iterator &other) const
			{
				return _pos <= other._pos;
			}
 
			bool operator >= (const CScriptArraySTL_iterator &other) const
			{
				return _pos >= other._pos;
			}

			CScriptArraySTL_iterator &operator += (size_type n)
			{
				_pos += n;
				return *this;
			}

			CScriptArraySTL_iterator &operator -= (size_type n)
			{
				_pos -= n;
				return *this;
			}

			CScriptArraySTL_iterator operator + ( size_type n ) const
			{
				return CScriptArraySTL_iterator(_buf, _pos + n);
			}

			CScriptArraySTL_iterator operator - ( size_type n ) const
			{
				return CScriptArraySTL_iterator(_buf, _pos - n);
			}

			size_type operator - ( const CScriptArraySTL_iterator &other ) const
			{
				return _pos - other._pos;
			}

			template <class T, class TArrayClass>
			friend CScriptArraySTL_iterator operator + ( size_type n, const CScriptArraySTL_iterator &it )
			{
				return CScriptArraySTL<T, TArrayClass>::CScriptArraySTL_iterator(it._buf, it._pos + n);
			}
	};

	// make our iterator types here
	typedef CScriptArraySTL_iterator<value_type, CScriptArraySTL>				iterator;
	typedef CScriptArraySTL_iterator<const value_type, const CScriptArraySTL>	const_iterator;
	typedef std::reverse_iterator<iterator>										reverse_iterator;
	typedef std::reverse_iterator<const_iterator>								const_reverse_iterator;

	// Constructors, Destructors and AngelScript initialization -------------------------------
	CScriptArraySTL(void)
		:m_as_array_ptr(NULL)
	{
	}

	~CScriptArraySTL(void)
	{
#if DEBUG || _DEBUG
		assert((m_as_array_ptr == NULL) && "Should be destoyed by calling Release() before the script engine has been released.");
#endif
	}

	// Initializes the array so it can be directly accessed using AngelScript
	// This must be called before the array can be used
	int InitArray(asIScriptEngine *engine, char *declaration, size_type init_length = 0)
	{
		// The script array needs to know its type to properly handle the elements.
		// Note that the object type should be cached to avoid performance issues
		// if the function is called frequently.
		asIObjectType* t = engine->GetObjectTypeById(engine->GetTypeIdByDecl(declaration));
		if(t == NULL) return -1; // the type doesn't exist

		m_as_array_ptr = new TArrayClass(init_length, t);

		return 0;
	}

	// returns a pointer to an array class that can be used in an AS script
	TArrayClass *GetRef(bool inc_ref_count = false, bool release_self = false)
	{
#ifdef ASSERT_IF_UNITIALIZED
		assert((m_as_array_ptr != NULL) && "InitArray() must be called before use.");
#endif
		if(inc_ref_count)
		{
			// increments the reference count
			// this is useful if we want to return a reference that can
			// persist in AngelScript even after this object has been destroyed
			m_as_array_ptr->AddRef();
		}

		if(release_self)
		{
			// this will disconnect the AngelScript Array from this CScriptArraySTL object
			// useful when this object is only used for setting up an array with
			// initial data and then passing the array to AngelScript
			TArrayClass *rval = m_as_array_ptr;
			m_as_array_ptr = NULL;

			return rval;
		}
		else
		{
			return m_as_array_ptr;
		}
	}

	// Releases a reference to the array. After this is called, this class can
	// no longer access the array data, but the data may still exist inside
	// AngelScript until the refernce count is 0.
	void Release()
	{
#ifdef ASSERT_IF_UNITIALIZED
		assert((m_as_array_ptr != NULL) && "InitArray() must be called before use.");
#endif
		m_as_array_ptr->Release();
		m_as_array_ptr = NULL;
	}

	// Capacity ----------------------------------------------------------------------------------

	// returns the number of elements in the array
	size_type size() const
	{
		return (size_type)m_as_array_ptr->GetSize();
	}
	
	// resizes the array, adding unitialized data if the new size is bigger than the old size or
	// deleting data if the new size is smaller
	void resize(size_type n)
	{
#ifdef ASSERT_IF_UNITIALIZED
		assert((m_as_array_ptr != NULL) && "InitArray() must be called before use.");
#endif
		m_as_array_ptr->Resize(n);
	}

	// returns true if the array is empty
	bool empty() const
	{
#ifdef ASSERT_IF_UNITIALIZED
		assert((m_as_array_ptr != NULL) && "InitArray() must be called before use.");
#endif
		return m_as_array_ptr->IsEmpty();
	}

	// grows the buffer capacity
	void reserve(size_type n)
	{
#ifdef ASSERT_IF_UNITIALIZED
		assert((m_as_array_ptr != NULL) && "InitArray() must be called before use.");
#endif
		m_as_array_ptr->Reserve(n);
	}

	// iterators ----------------------------------------------------------------------------
	// returns an iterator to the begining of the array
	iterator begin()
	{
		return iterator(this, 0);
	}
	// returns an iterator to the end of the array
	iterator end()
	{
		return iterator(this, size());
	}
	// returns a constant iterator to the begining of the array
	const_iterator cbegin() const
	{
		return const_iterator(this, 0);
	}
	// returns a constant iterator to the end of the array
	const_iterator cend() const
	{
		return const_iterator(this, size());
	}
	// returns a constant iterator to the begining of the array
	iterator begin() const
	{
		return cbegin();
	}
	// returns a constant iterator to the end of the array
	iterator end() const
	{
		return cend();
	}

	// returns a reverse iterator to the begining of the array
	reverse_iterator rbegin()
	{
		return reverse_iterator(end());
	}
	// returns a reverse iterator to the end of the array
	reverse_iterator rend()
	{
		return reverse_iterator(begin());
	}
	// returns a constant reverse iterator to the begining of the array
	const_reverse_iterator crbegin() const
	{
		return const_reverse_iterator(cend());
	}
	// returns a constant reverse iterator to the end of the array
	const_reverse_iterator crend() const
	{
		return const_reverse_iterator(cbegin());
	}
	// returns a constant reverse iterator to the begining of the array
	const_reverse_iterator rbegin() const
	{
		return crbegin();
	}
	// returns a constant reverse iterator to the end of the array
	const_reverse_iterator rend() const
	{
		return crend();
	}

	// Element Access -----------------------------------------------------------------------
	// returns a reference to an element in the array. This will not throw an out-of-range exception.
	// undefined behavior if out of range.
	reference operator[](size_type index)
	{
		// doesn't throw an exception
		// undefined if out of range
		// since the array is actually being controlled by AngelScript, make sure we've been initialized first
#ifdef ASSERT_IF_UNITIALIZED
		assert((m_as_array_ptr != NULL) && "InitArray() must be called before use.");
#endif
		pointer element = (pointer)m_as_array_ptr->At(index);

		return (*element);
	}

	// returns a const reference to an element in the array. This will not throw an out-of-range exception.
	// undefined behavior if out of range.
	const_reference operator[](size_type index) const
	{
		// doesn't throw an exception
		// undefined if out of range

		// since the array is actually being controlled by AngelScript, make sure we've been initialized first
#ifdef ASSERT_IF_UNITIALIZED
		assert((m_as_array_ptr != NULL) && "InitArray() must be called before use.");
#endif
		const_pointer element = (const_pointer)m_as_array_ptr->At(index);

		return (*element);
	}

	// returns a reference to an element in the array. This will throw an out-of-range exception.
	reference at(size_type index)
	{
		// throws an exception if out of range
		// since the array is actually being controlled by AngelScript, make sure we've been initialized first
#ifdef ASSERT_IF_UNITIALIZED
		assert((m_as_array_ptr != NULL) && "InitArray() must be called before use.");
#endif
		pointer element = (pointer)m_as_array_ptr->At(index);
		if(element == NULL)
		{
			throw std::out_of_range("pos out of range");
		}
		return (*element);
	}

	// returns a constant reference to an element in the array. This will throw an out-of-range exception.
	const_reference at(size_type) const
	{
		// throws an exception if out of range
		// since the array is actually being controlled by AngelScript, make sure we've been initialized first
#ifdef ASSERT_IF_UNITIALIZED
		assert((m_as_array_ptr != NULL) && "InitArray() must be called before use.");
#endif
		const_pointer element = (const_pointer)m_as_array_ptr->At(index);
		if(element == NULL)
		{
			throw std::out_of_range("pos out of range");
		}
		return (*element);
	}

	// returns a reference to the first element
	// undefined if empty
	reference front()
	{
		return (*this)[0];
	}

	// returns a constant reference to the first element
	// undefined if empty
	const_reference front() const
	{
		return (*this)[0];
	}

	// returns a reference to the last element
	// undefined if empty
	reference back()
	{
		return(*this)[size()-1];
	}

	// returns a constant reference to the last element
	// undefined if empty
	const_reference back() const
	{
		// returns last element
		// undefined if empty
		return (*this)[size()-1];
	}

	// Modifiers ------------------------------------------------------------------------------------
	// adds a value to the end of the array
	void push_back (const value_type& val)
	{
#ifdef ASSERT_IF_UNITIALIZED
		assert((m_as_array_ptr != NULL) && "InitArray() must be called before use.");
#endif
		m_as_array_ptr->InsertLast((void *)&val);
	}

	void pop_back()
	{
		// undefined if empty
		resize(size()-1);
	}

	// assigns new data to the array using iterators.
	template <class InputIterator>
	void assign (InputIterator first, InputIterator last)
	{
		resize(0);
		for(auto it = first; it < last; it++)
		{
			push_back(*it);
		}
	}

	// fills the array 
	void assign (size_type n, const value_type& val)
	{
		resize(n);

		for(size_type i = 0; i < n; ++i)
		{
			(*this)[i] = val;
		}
	}

	// clears the contents of the array
	void clear()
	{
		resize(0);
	}

private:
	TArrayClass *m_as_array_ptr; // Reference counted within AngelScript
};

