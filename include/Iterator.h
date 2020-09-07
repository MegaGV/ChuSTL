#pragma once

#ifndef _CHUSTL_ITERATOR_H_
#define _CHUSTL_ITERATOR_H_

#include <cstddef>
#include "TypeTraits.h"

namespace ChuSTL {

	//�����Ϊ����õ�����(tag types)
	struct input_iterator_tag {};
	struct output_iterator_tag {};
	struct forward_iterator_tag : public input_iterator_tag {};
	struct bidirectional_iterator_tag : public forward_iterator_tag {};
	struct random_access_iterator_tag : public bidirectional_iterator_tag {};



	/*
	* ͨ�������������������ã��������ö��󣬶��������λ�ڼ����С�
	* ͨ�����ڴ��ݵ�ַ��
	*/
	template<class T, class Distance>
	struct input_iterator {
		typedef input_iterator_tag	iterator_category;
		typedef T					value_type;
		typedef Distance			difference_type;
		typedef T*					pointer;
		typedef T&					reference;
	};

	/*
	* ���������������������������ƣ�Ҳֻ�ܵ�����ǰ����Ԫ�أ���ͬ���Ǹ����������Ԫ��ֻ��д��Ȩ����
	* ͨ�����ڷ��ص�ַ��
	*/
	struct output_iterator {
		typedef output_iterator_tag	iterator_category;
		typedef void				value_type;
		typedef void				difference_type;
		typedef void				pointer;
		typedef void				reference;
	};

	/*
	* ǰ�������������һ����ȷ�������н��ж�д������
	* ��ӵ��������������������Ժ�����������Ĳ������ԣ��Լ�������ǰ����Ԫ�ص�������
	* ͨ�����ڱ�����
	*/
	template <class T, class Distance>
	struct forward_iterator {
		typedef forward_iterator_tag	iterator_category;
		typedef T						value_type;
		typedef Distance				difference_type;
		typedef T*						pointer;
		typedef T&						reference;
	};

	/*
	* �������������ǰ��������Ļ������ṩ�˵���������Ԫ�ص�������
	* ǰ��������ĸ߼��档
	*/
	template <class T, class Distance>
	struct bidirectional_iterator {
		typedef bidirectional_iterator_tag	iterator_category;
		typedef T							value_type;
		typedef Distance					difference_type;
		typedef T*							pointer;
		typedef T&							reference;
	};

	/*
	* ���������������������е������Ĺ�����������ָ�����������������㡣
	*/
	template <class T, class Distance>
	struct random_access_iterator {
		typedef random_access_iterator_tag	iterator_category;
		typedef T							value_type;
		typedef Distance					difference_type;
		typedef T*							pointer;
		typedef T&							reference;
	};

	/*
	* ��׼std::iterator����֤����STL����淶
	*/
	template<class Category, class T, class Distance = ptrdiff_t,
		class Pointer = T*, class Reference = T&>
		struct iterator
	{
		typedef Category	iterator_category;
		typedef T			value_type;
		typedef Distance	difference_type;
		typedef Pointer		pointer;
		typedef Reference	reference;
	};

	/*
	* traits��̼���
	*/
	

	// �������������͵ĺ���
	template<class Iterator>
	inline typename iterator_traits<Iterator>::iterator_category
		iterator_category(const Iterator&) {
		typedef typename iterator_traits<Iterator>::iterator_category category;
		return category();
	}

	// ����������ָ��������͵ĺ���
	template<class Iterator>
	inline typename iterator_traits<Iterator>::value_type*
		value_type(const Iterator& It) {
		return static_cast<typename iterator_traits<Iterator>::value_type*>(0);
	}

	// �����������������͵ĺ���
	template<class Iterator>
	inline typename iterator_traits<Iterator>::difference_type*
		difference_type(const Iterator& It) {
		return static_cast<typename iterator_traits<Iterator>::difference_type*>(0);
	}

	/*
	* advance ����: ���ڶ�����ʵ���������ĳ���������ṩ���ֵ����������ذ汾
	*/
	template<class InputIterator, class Distance>
	inline void __advance(InputIterator& it, Distance n, input_iterator_tag) {
		while (n--)
			++it;
	}

	template<class ForwardIterator, class Distance>
	inline void __advance(ForwardIterator& it, Distance n, forward_iterator_tag) {
		__advance(it, n, input_iterator_tag());
	}

	template<class BidirectionalIterator, class Distance>
	inline void __advance(BidirectionalIterator& it, Distance n, bidirectional_iterator_tag) {
		if (n >= 0)
			while (n--) ++it;
		else
			while (n++) --it;
	}

	template<class RandomAccessIterator, class Distance>
	inline void __advance(RandomAccessIterator& it, Distance n, random_access_iterator_tag) {
		it += n;
	}

	/*
	* ���⿪�ŵ��ϲ���ƽӿڣ����Ե������������ص�__advance()
	* ��������ΪInputIterator������ΪSTL���㷨��������
	* �����������㷨���ܽ�����ͽ׵�����������Ϊ����������Ͳ�������
	*/
	template<class InputIterator, class Distance>
	inline void advance(InputIterator& it, Distance n) {
		__advance(it, n, iterator_traits<InputIterator>::iterator_category());
	}

	/*
	* distance ����: ���ڼ�������������
	*/
	template<class InputIterator>
	inline typename iterator_traits<InputIterator>::difference_type
		__distance(InputIterator first, InputIterator last, input_iterator_tag) {
		typename iterator_traits<InputIterator>::difference_type distance = 0;
		while (first++ != last)
			++distance;
		return distance;
	}

	template<class RandomAccessIterator>
	inline typename iterator_traits<RandomAccessIterator>::difference_type
		__distance(RandomAccessIterator first, RandomAccessIterator last, random_access_iterator_tag) {
		return last - first;
	}

	template<class InputIterator>
	inline typename iterator_traits<InputIterator>::difference_type
		distance(InputIterator first, InputIterator last) {
		typedef typename iterator_traits<InputIterator>::iterator_category category;
		return __distance(first, last, category());
	}
}

#endif // !_CHUSTL_ITERATOR_H_