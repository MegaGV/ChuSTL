#pragma once

#ifndef _CHUSTL_ITERATOR_H_
#define _CHUSTL_ITERATOR_H_

#include <cstddef>
#include "TypeTraits.h"

namespace ChuSTL {

	//五个作为标记用的类型(tag types)
	struct input_iterator_tag {};
	struct output_iterator_tag {};
	struct forward_iterator_tag : public input_iterator_tag {};
	struct bidirectional_iterator_tag : public forward_iterator_tag {};
	struct random_access_iterator_tag : public bidirectional_iterator_tag {};



	/*
	* 通过对输入迭代器解除引用，它将引用对象，而对象可能位于集合中。
	* 通常用于传递地址。
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
	* 该类迭代器和输入迭代器极其相似，也只能单步向前迭代元素，不同的是该类迭代器对元素只有写的权力。
	* 通常用于返回地址。
	*/
	struct output_iterator {
		typedef output_iterator_tag	iterator_category;
		typedef void				value_type;
		typedef void				difference_type;
		typedef void				pointer;
		typedef void				reference;
	};

	/*
	* 前向迭代器可以在一个正确的区间中进行读写操作。
	* 它拥有输入迭代器的所有特性和输出迭代器的部分特性，以及单步向前迭代元素的能力。
	* 通常用于遍历。
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
	* 该类迭代器是在前向迭代器的基础上提供了单步向后迭代元素的能力。
	* 前向迭代器的高级版。
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
	* 该类迭代器能完成上面所有迭代器的工作，可以像指针那样进行算术计算。
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
	* 标准std::iterator，保证符合STL所需规范
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
	* traits编程技法
	*/
	

	// 决定迭代器类型的函数
	template<class Iterator>
	inline typename iterator_traits<Iterator>::iterator_category
		iterator_category(const Iterator&) {
		typedef typename iterator_traits<Iterator>::iterator_category category;
		return category();
	}

	// 决定迭代器指向对象类型的函数
	template<class Iterator>
	inline typename iterator_traits<Iterator>::value_type*
		value_type(const Iterator& It) {
		return static_cast<typename iterator_traits<Iterator>::value_type*>(0);
	}

	// 决定迭代器距离类型的函数
	template<class Iterator>
	inline typename iterator_traits<Iterator>::difference_type*
		difference_type(const Iterator& It) {
		return static_cast<typename iterator_traits<Iterator>::difference_type*>(0);
	}

	/*
	* advance 函数: 用于定向访问到迭代器的某个变量。提供四种迭代器的重载版本
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
	* 对外开放的上层控制接口，用以调用上述各重载的__advance()
	* 参数命名为InputIterator，是因为STL的算法命名规则
	* ————以算法所能接受最低阶迭代器类型来为其迭代器类型参数命名
	*/
	template<class InputIterator, class Distance>
	inline void advance(InputIterator& it, Distance n) {
		__advance(it, n, iterator_traits<InputIterator>::iterator_category());
	}

	/*
	* distance 函数: 用于计算迭代器间距离
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