#pragma once

#ifndef _CHUSTL_LIST_H
#define _CHUSTL_LIST_H

#include "Allocator.h"
#include "Alloc.h"
#include "Iterator.h"
#include "TypeTraits.h"
#include "Uninitialized.h"


namespace ChuSTL {

	template<class T>
	struct __list_node {
		typedef void* void_pointer;
		void_pointer prev;	// or __list_node<T>*
		void_pointer next;
		T data;
	};

	template<class T, class Ref, class Ptr>
	struct __list_iterator {
		typedef __list_iterator<T, T&, T*>		iterator;
		typedef __list_iterator<T, Ref, Ptr>	self;
		typedef __list_node<T>* link_type;
		typedef bidirectional_iterator_tag		iterator_category;
		typedef T								value_type;
		typedef Ptr								pointer;
		typedef Ref								reference;
		typedef size_t							size_type;
		typedef ptrdiff_t						difference_type;

		// 迭代器内的普通指针，指向list节点
		link_type node;

		__list_iterator() {}
		__list_iterator(link_type x) : node(x) {}
		__list_iterator(const iterator& x) : node(x.node) {}

		bool operator== (const self& x) const {
			return node == x.node;
		}

		bool operator!= (const self& x) const {
			return node != x.node;
		}

		reference operator* () const {
			return (*node).data;
		}

		pointer operator-> () const {
			return &(operator* ());
		}

		self& operator++() {
			node = (link_type)((*node).next);
		}

		self& operator++(int) {
			self tmp = *this;
			++* this;
			return tmp;
		}

		self& operator--() {
			node = (link_type)((*node).prev);
		}

		self& operator--(int) {
			self tmp = *this;
			--* this;
			return tmp;
		}
	};

	// SGI list是一个环状双向链表
	template<class T, class Alloc> // Alloc = alloc
	class list {
	protected:
		typedef __list_node<T> list_node;
		// 专属空间配置器，每次配置一个节点大小
		typedef simple_alloc<list_node, Alloc> list_node_allocator;
		// 用一个指针表示整个环状双向链表
		// node指向尾端的一个空白节点，符合STL前闭后开条件
		link_type node;

	public:
		typedef list_node* link_type; 

	protected:
		// 配置、释放、构造、销毁节点
		link_type get_node() {
			return list_node_allocator::allocate();
		}
		void put_node(link_type p) {
			link_node_allocator::deallocate(p);
		}
		link_type create_node(const T& x) {
			link_type p = get_node();
			construct(&p->data, x);
			return p;
		}
		void destroy_node(link_type p) {
			destroy(p->data);
			put_node(p);
		}

		void empty_initialize() {
			node = get_node();
			node->next = node;
			node->prev = node;
		}

		void transfer(iterator position, iterator first, iterator last) {
			if (position != last) {
				(*(link_type((*last.node).prev))).next = position.node;
				(*(link_type((*first.node).prev))).next = last.node;
				(*(link_type((*position.node).prev))).next = first.node;
				link_type tmp = link_type((*position.node).prev);
				(*position.node).prev = (*last.node).prev;
				(*last.node).prev = (*first.node).prev;
				(*first.node).prev = tmp;
			}
		}

	public:
		list() { empty_initialize(); }

		iterator begin() { return (link_type)((*node).next); }
		iterator end() { return node; }
		bool empty() { return node == node->next; }
		reference front() { return *begin(); }
		reference back() { return *(--end()); }
		size_type size() const {
			size_type result = 0;
			distance(begin(), end(), result);
			return result;
		}

		iterator insert(iterator position, const T& x) {
			link_type tmp = create_node(x);
			tmp->next = position.node;
			tmp->prev = position.node->prev;
			(link_type(position.node->prev))->next = tmp;
			position.node->prev = tmp;
			return tmp;
		}
		void push_front(const T& x) { insert(begin(), x); }
		void push_back(const T& x) { insert(end(), x); }
		iterator erase(iterator position) {
			link_type next_node = link_type(position.node->next);
			link_type prev_node = link_type(position.node->prev);
			prev_node->next = next_node;
			next_node->prev = prev_node;
			destroy(position.node);
			return iterator(next_node);
		}
		void pop_front() { erase(begin()); }
		void pop_back() { erase(--end()); }
		void clear(); // 清除所有节点
		void remove(const T& value); // 将数值为value的所有元素移除
		void unique(); // 移除数值相同的连续元素

		// 将x接合于positon所指位置之前，x必须不同于*this
		void splice(iterator position, list& x) {
			if (!x.empty())
				transfer(postion, x.begin(), x.end());
		}
		// 将i所指元素接合于position所指位置之前。position和i可能指向同一个list
		void splice(iterator position, list&, iterator i) {
			iterator j = i;
			++j;
			if (position == i || position == j)
				return;
			transfer(position, i, j);
		}
		// 将[first, last)内的所有元素接合于position所指位置之前
		// position和[first, last)可能指向同一个list
		// 但position不能位于[first, last)之内
		void splice(iterator position, list&, iterator first, iterator last) {
			if (first != last)
				transfer(position, first, last);
		}
		void merge(list<T, Alloc>& x); // 将x合并到*this上，两个list内容必须先经过递增排序
		void reverse(); // 将*this内容逆向重置
		void sort(); // list不能使用stl的sort()，只能使用自己的成员函数
	};

	template<class T, class Alloc>
	void list<T, Alloc>::clear(){
		link_type cur = (link_type) node->next;
		while (cur != node) {
			link_type tmp = cur;
			cur = (link_type)cur->next;
			destroy_node(tmp);
		}
		node->next = node;
		node->prev = node;
	}
	template<class T, class Alloc>
	void list<T, Alloc>::remove(const T& value) {
		iterator first = begin();
		iterator last = end();
		while (first != last) {
			iterator next = first;
			next++;
			if (*first == value)
				erase(first);
			first = next;
		}
	}
	template<class T, class Alloc>
	void list<T, Alloc>::unique() {
		iterator first = begin();
		iterator last = end();
		if (first == last)
			return;
		iterator next = first;
		while (++next != last) {
			if (*first == *next)
				erase(next);
			else
				first = next;
			next = first;
		}
	}
	template<class T, class Alloc>
	void list<T, Alloc>::merge(list<T, Alloc>& x) {
		iterator first1 = begin();
		iterator last1 = end();
		iterator first2 = x.begin();
		iterator last2 = x.end();

		while (first1 != last1 && first2 != last2) {
			if (*first2 < *first1) {
				iterator next = firsr2;
				transfer(first1, first2, ++next);
			}
			else {
				++first1;
			}
		}
		if (first2 != last2)
			transfer(last1, first2, last2);
	}
	template<class T, class Alloc>
	void list<T, Alloc>::reverse() {
		if (node->next == node || link_type(node->next)->next == node)
			return;
		iterator first = begin();
		++first;
		while (first != end()) {
			iterator old = first;
			++first;
			transfer(begin(), old, first);
		}
	}
	template<class T, class Alloc>
	void list<T, Alloc>::sort() {
		if (node->next == node || link_type(node->next)->next == node)
			return;
		list<T, Alloc> carry;
		list<T, Alloc> counter[64];
		int fill = 0;
		while (!empty()) {
			carry.splice(carry.begin(), *this, begin());
			int i = 0;
			while (i < fill && !counter[i].empty()) {
				counter[i].merge(carry);
				carry.swap(counter[i++]);
			}
			carry.swap(counter[i]);
			if (i == fill)
				++fill;
		}

		for (int i = 1; i < fill; i++)
			counter[i].merge(counter[i - 1]);
		swap(counter[fill - 1]);
	}

}

#endif // !_CHUSTL_LIST_H
