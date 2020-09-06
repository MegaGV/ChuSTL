#pragma once

#ifndef _CHUSTL_UNINITIALIZED_H_
#define _CHUSTL_UNINITIALIZED_H_

#include "Iterator.h"

namespace ChuSTL {

	/*
	* 以下三种函数均具有"原子性"，要么产生所有的元素，要么回滚不产生任何元素
	* 任意一次复制构造发生异常时，析构全部已产生的元素
	*/

	// 针对输入范围[first, last)内的每一个迭代器i，在输出范围上生成*i的复制
	// 调用construct(&*(result + (i - first)), *i)
	template<class InputIterator, class ForwardIterator>
	ForwardIterator uninitialized_copy(InputIterator first, InputIterator last, ForwardIterator result) {
		return __uninitialized_copy(first, last, result, value_type(result));
	}

	// 针对输入范围[first, last)内的每一个迭代器i，在i处生成x的复制
	// 调用construct(&*i, x)
	template<class ForwardIterator, class T>
	void uninitialized_fill(ForwardIterator first, ForwardIterator last, const T& x) {
		__uninitialized_fill(first, last, x, value_type(first));
	}

	// 针对输入范围[first, first + n)内的每一个迭代器i，在i处生成x的复制
	// 调用construct(&*i, x)
	template<class ForwardIterator, class Size, class T>
	ForwardIterator uninitialized_fill_n(ForwardIterator first, Size n, const T& x) {
		return __uninitiallized_fill_n(first, n, x, value_type(first));
	}


	/*
	* POD->Plain Old Data 标量类型(传统C struct类型)
	* POD类型必然拥有trivial ctor/dtor/copy/assignment 函数
	* 因此对POD类型采用最有效率的手法，对non-POD类型采用最保险安全的手法
	* 且当是POD类型时，复制构造等同于赋值，且析构函数是trivial的时才有效
	*/

	template<class InputIterator, class ForwardIterator, class T>
	inline ForwardIterator
		__uninitiallized_copy(InputIterator first, InputIterator last) {
		typedef typename __type_traits<T>::is_POD_type is_POD;
		return __uninitiallized_copy_aux(first, last, result, is_POD());
	}

	template<class InputIterator, class ForwardIterator>
	inline ForwardIterator
		__uninitiallized_copy_aux(InputIterator first, InputIterator last, ForwardIterator result, std::true_type) { // __true_type
		return copy(first, last, result);
	}

	template<class InputIterator, class ForwardIterator>
	inline ForwardIterator
		__uninitiallized_copy_aux(InputIterator first, InputIterator last, ForwardIterator result, std::false_type) { // __false_type
		ForwardIterator cur = result;
		for (; first != last; ++first, ++cur) {
			construct(&*cur, *first);
		}
		return cur;
	}

	// 对char*和wchar_t*的特化版
	inline char* uninitialized_copy(const char* first, const char* last, char* result);
	inline wchar_t* uninitialized_copy(const wchar_t* first, const wchar_t* last, wchar_t* result);

	template<class ForwardIterator, class T, class T1>
	inline void 
		__uninitialized_fill(ForwardIterator first, ForwardIterator last, const T& x, T1*) {
		typedef typename __type_traits<T1>::is_POD_type is_POD;
		__uninitialized_fill_aux(first, last, x, is_POD());
	}

	template<class ForwardIterator, class T>
	inline void
		__uninitialized_fill_aux(ForwardIterator first, ForwardIterator last, const T& x, std::true_type) { // __true_type
		fill(first, last, x);
	}

	template<class ForwardIterator, class T>
	inline void
		__uninitialized_fill_aux(ForwardIterator first, ForwardIterator last, const T& x, std::false_type) { // __false_type
		ForwardIterator cur = first;
		for (; cur != last; ++cur) {
			construct(&*cur, x);
		}
	}

	template<class ForwardIterator, class Size, class T, class T1>
	inline ForwardIterator 
		__uninitialized_fill_n(ForwardIterator first, Size n, const T& x, T1*) {
		typedef typename __type_traits<T1>::is_POD_type is_POD;
		return __uninitialized_fill_n_aux(first, n, x, is_POD());
	}

	template<class ForwardIterator, class Size, class T>
	inline ForwardIterator 
		__uninitialized_fill_n_aux(ForwardIterator first, Size n, const T& x, std::true_type) { // __true_type
		return fill_n(first, n, x);
	}
		
	template<class ForwardIterator, class Size, class T>
	ForwardIterator 
		__uninitialized_fill_n_aux(ForwardIterator first, Size n, const T& x, std::false_type) { // __false_type
		ForwardIterator cur = first;
		for (; n > 0; --n, ++cur) {
			construct(&*cur, x);
		}
		return cur;
	}

	


}

#endif // !_CHUSTL_UNINITIALIZED_H_
