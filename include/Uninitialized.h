#pragma once

#ifndef _CHUSTL_UNINITIALIZED_H_
#define _CHUSTL_UNINITIALIZED_H_

#include "Iterator.h"

namespace ChuSTL {

	/*
	* �������ֺ���������"ԭ����"��Ҫô�������е�Ԫ�أ�Ҫô�ع��������κ�Ԫ��
	* ����һ�θ��ƹ��췢���쳣ʱ������ȫ���Ѳ�����Ԫ��
	*/

	// ������뷶Χ[first, last)�ڵ�ÿһ��������i���������Χ������*i�ĸ���
	// ����construct(&*(result + (i - first)), *i)
	template<class InputIterator, class ForwardIterator>
	ForwardIterator uninitialized_copy(InputIterator first, InputIterator last, ForwardIterator result) {
		return __uninitialized_copy(first, last, result, value_type(result));
	}

	// ������뷶Χ[first, last)�ڵ�ÿһ��������i����i������x�ĸ���
	// ����construct(&*i, x)
	template<class ForwardIterator, class T>
	void uninitialized_fill(ForwardIterator first, ForwardIterator last, const T& x) {
		__uninitialized_fill(first, last, x, value_type(first));
	}

	// ������뷶Χ[first, first + n)�ڵ�ÿһ��������i����i������x�ĸ���
	// ����construct(&*i, x)
	template<class ForwardIterator, class Size, class T>
	ForwardIterator uninitialized_fill_n(ForwardIterator first, Size n, const T& x) {
		return __uninitiallized_fill_n(first, n, x, value_type(first));
	}


	/*
	* POD->Plain Old Data ��������(��ͳC struct����)
	* POD���ͱ�Ȼӵ��trivial ctor/dtor/copy/assignment ����
	* ��˶�POD���Ͳ�������Ч�ʵ��ַ�����non-POD���Ͳ�����հ�ȫ���ַ�
	* �ҵ���POD����ʱ�����ƹ����ͬ�ڸ�ֵ��������������trivial��ʱ����Ч
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

	// ��char*��wchar_t*���ػ���
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
