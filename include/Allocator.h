#pragma once

#ifndef _CHUSTL_ALLOCATOR_H_
#define _CHUSTL_ALLOCATOR_H_

#include <new>			// for placement new
#include <cstddef>		// for ptrdiff_t, size_t
#include <cstdlib>		// for exit()
#include <climits>		// for UINT_MAX
#include <iostream>		// for cerr

//#include <type_traits>

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
		new (p) T1(value);							// 调用T1::T1(value)
	}

	// 第一版本，接受一个指针
	template<typename T>
	inline void _destroy(T* ptr) {
		ptr->~T();
	}

	// 第二版本，接受两个迭代器
	// 利用value_type获取类型，再利用__type_traits<>判断是否其析构函数是否重要————trivial destructor
	template<class ForwardIterator>
	inline void destroy(ForwardIterator first, ForwardIterator last) {
		__destroy(first, last, value_type(first));
	}

	// 判断元素数值类型是否有trivial destructor，指...
	// 如果用户不定义析构函数，而是用系统自带的，则说明析构函数基本没有什么用（但默认会被调用）
	template<class ForwardIterator, class T>
	inline void __destroy(ForwardIterator first, ForwardIterator last, T*) {
		typedef typename __type_traits<T>::has_trivial_destructor trivial_destructor;
		__destroy_aux(fist, last, trivial_destructor());
	}

	// 如果元素的数值类型没有trivial destructor
	template<class ForwardIterator, class T>
	inline void
		__destroy_aux(ForwardIterator first, ForwardIterator last, std::false_type) { // __false_type
		for (; first < last; ++first) {
			destroy(&*first);
		}
	}

	// 如果元素的数值类型有trivial destructor
	template<class ForwardIterator>
	inline void
		__destroy_aux(ForwardIterator first, ForwardIterator last, std::true_type) { } // __true_type

	// 对char*和wchar_t*的特化版
	inline void destroy(char*, char*) {}
	inline void destroy(wchar_t*, wchar_t*) {}

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

#endif  // !_CHUSTL_ALLOCATOR_H_