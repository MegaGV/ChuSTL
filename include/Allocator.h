#pragma once

#ifndef _CHUSTL_ALLOCATOR_H_
#define _CHUSTL_ALLOCATOR_H_

#include <new>			// for placement new
#include <cstddef>		// for ptrdiff_t, size_t
#include <cstdlib>		// for exit()
#include <climits>		// for UINT_MAX
#include <iostream>		// for cerr

namespace ChuSTL {

	template<typename T>
	inline T* _allocate(ptrdiff_t size, T*) {
		std::set_new_handler(0);
		T* tmp = (T*) (::operator new ((size_t) (size * sizeof(T))));
		if (tmp == 0) {
			std::cerr << "out of memory" << std::endl;
			exit(1);
		}
		return tmp;
	}

	template<typename T>
	inline void _deallocate(T* buffer) {
		::operator delete(buffer);
	}

	template<typename T1, typename T2>
	inline void _construct(T1* p, const T2& value) {
		new (p) T1(value);							// placement new. invoke ctor of T1
	}

	//第一版本，接受一个指针
	template<typename T>
	inline void _destroy(T* ptr) {
		ptr->~T();
	}

	template<typename T>
	class allocator 
	{
	public:
		typedef T				value_type;
		typedef T*				pointer;
		typedef const T*		const_pointer;
		typedef T&				reference;
		typedef const T&		const_reference;
		typedef size_t			size_type;
		typedef ptrdiff_t		difference_type;

		//rebind allocator of type U
		//nested class template
		//other means allocator<U>
		template<typename U>
		struct rebind {
			typedef allocator<U> other;
		};
		
		//get objesct's address
		//a.address(x) -->> &x
		pointer address(reference x) {
			return (pointer)&x;
		}

		//get const objesct's address
		//a.address(x) -->> &x
		const_pointer const_address(const_reference x) {
			return (const_pointer)&x;
		}
		
		//allocate spaces, can keep object T for n
		//hint can increase locality or ignore it 
		pointer allocate(size_type n, const void* hint = 0) {
			return _allocate((difference_type)n, (pointer)0);
		}

		//deallocate spaces
		void deallocate(pointer p, size_type n) {
			_deallocate(p);
		}

		//the same as new ((void*) p) T(x)
		void construct(pointer p, const T& value) {
			_construct(p, value);
		}

		//the same as p->~T()
		void destroy(pointer p) {
			_destroy(p);
		}

		//get max size of T
		size_type max_size() const {
			return size_type(UINT_MAX / sizeof(T));
		}

	};

}

#endif