#pragma once

#ifndef _CHUSTL_DEQUE_H
#define _CHUSTL_DEQUE_H

#include "Allocator.h"
#include "Alloc.h"
#include "Iterator.h"
#include "Uninitialized.h"

namespace ChuSTL {

	template<class T, class Ref, class Ptr, size_t BufSize>
	struct __deque_iterator {
		typedef T													value_type;
		typedef Ptr													pointer;
		typedef Ref													reference;
		typedef size_t												size_type;
		typedef ptrdiff_t											difference_type;

		typedef random_access_iterator_tag							iterator_category;
		typedef __deque_iterator									self;
		typedef __deque_iterator<T, T&, T*, BufSize>				iterator;
		typedef __deque_iterator<T, const T&, const T*, BufSize>	const_iterator;
		typedef T**													map_pointer;

		T* cur;				// 该迭代器所指缓冲区中的当前元素
		T* first;			// 该迭代器所指缓冲区的头
		T* last;			// 该迭代器所指缓冲区的尾
		map_pointer node;	// 指向管控中心

		static size_type buffer_size() {
			return __deque_buf_size(BufSize, sizeof(value_type));
		}
		// 如果n不为0，返回n，表示buffer size用户自定义
		// 如果n为0，表示buffer size使用默认值，即
		//	如果sz（元素大小，sizeof(value_type)）小于512， 返回 512/sz
		//	如果sz大于512， 返回1
		inline size_type __deque_buf_size(size_type n, size_type sz) {
			return n != 0 ? n : (sz < 512 ? size_type(512 / sz) : size_type(1));
		}

		// 切换指向的缓冲区
		void set_node(map_pointer new_node) {
			node = new_node;
			first = *new_node;
			last = first + difference_type(buffer_size());
		}

		reference operator* () const { return *cur; }
		pointer operator-> () const { return &(*cur) }
		difference_type operator-(const self& x) const {
			return difference_type(buffer_size()) * (node - x.node - 1) +
				(cur - first) + (x.last - x.cur);
		}

		// 单目运算符重载，后置式调用前置式完成，在移动时需要判断是否需要进行缓冲区切换
		self& operator++() {
			++cur;
			if (cur == last) {
				self_node(node + 1);
				cur = first;
			}
			return *this;
		}
		self operator++(int) {
			self tmp = *this;
			++* this;
			return tmp;
		}
		self& operator--() {
			if (cur == first) {
				self_node(node - 1);
				cur = last;
			}
			--cur;
			return *this;
		}
		self operator--(int) {
			self tmp = *this;
			--* this;
			return tmp;
		}
		// 随机存取，迭代器可直接跳跃多个距离
		self& operator+=(difference_type n) {
			difference_type offset = n + (cur - first);
			// 目标位置仍在同一缓冲区内，直接移动
			if (offset >= 0 && offset < difference_type(buffer_size())) {
				cur += n;
			}
			// 目标位置不在同一缓冲区，需切换到新的缓冲区再移动
			else {
				difference_type node_offset =
					offset > 0 ? offset / difference_type(buffer_size())
								: -difference_type((-offset - 1) / buffer_size()) - 1;
				set_node(node + node_offset);
				cur = first + (offset - node_offset * difference_type(buffer_size()));
			}
			return *this;
		}
		self operator+(difference_type n) const {
			self tmp = *this;
			return tmp += n;
		}
		self& operator-=(difference_type n) {
			return *this += -n;
		}
		self operator-(difference_type n) const {
			self tmp = *this;
			return tmp -= n;
		}
		reference operator[](difference_type n) const {
			return *(*this + n);
		}
		bool operator==(const self& x) const {
			return cur == x.cur;
		}
		bool operator!=(const self& x) const {
			return cur != x.cur;
		}
		bool operator<(const self& x) const {
			return (node == x.node) ? (cur < x.cur) : (node < x.node);
		}

	};

	template<class T, class Alloc, size_t BufSize = 0>
	class deque {
	public :
		typedef T			value_type;
		typedef T*			pointer;
		typedef T&			reference;
		typedef size_t		size_type;
		typedef ptrdiff_t	difference_type;

		typedef __deque_iterator<T, T&, T*, BufSize> iterator;

	protected:
		typedef pointer* map_pointer;

		iterator start;
		iterator finish;
		// map是块连续空间，其内每个元素都是一个指针(节点)，指向一块缓冲区
		// map_size是map内可容纳的指针数
		map_pointer map;
		size_type map_size;

		// 专属空间配置器，每次为一个元素(指针)配置空间
		typedef simple_alloc<value_tpe, Alloc> data_allocator;
		typedef simple_alloc<pointer, Alloc> map_allocator;


	public:
		deque(int n, const value_type& value) 
			: start(0), finish(0), map(0), map_size(0) 
		{ fill_initialize(n, value); }

		iterator begin() { return start; }
		iterator end() { return finish; }
		reference operator[](size_type n) { return start[difference_type(n)]; }
		reference front() { return *begin(); }
		reference back() { return *(end() - 1); }
		size_type size() const { return end() - begin(); }
		size_type max_size() const { return size_type(-1); }
		bool empty() { return end() == begin(); }

		void push_back(const value_type& value) {
			// 尾缓冲区有两个以上元素的备用空间时，直接在备用空间上构造元素
			// 否则配置新的缓冲区
			if (finish.cur != finish.last - 1) {
				construct(finish.cur, value);
				++finish.cur;
			}
			else {
				push_back_aux(value);
			}
		}
		void push_front(const value_type& value) {
			// 头缓冲区还有备用空间时，直接在备用空间上构造元素
			// 否则配置新的缓冲区
			if (start.cur != start.first) {
				construct(start.cur, value);
				--start.cur;
			}
			else {
				push_front_aux(value);
			}
		}
		void pop_back() {
			// 尾缓冲区还有元素时，调整指针析构最后元素
			// 否则释放尾缓冲区
			if (finish.cur != finish.first) {
				--finish.cur;
				destroy(finish.cur);
			}
			else
				pop_back_aux();
		}
		void pop_front() {
			// 头缓冲区还有两个以上元素时，调整指针析构最前元素
			// 否则释放头缓冲区
			if (start.cur != start.last - 1) {
				destroy(start.cur);
				++start.cur;
			}
			else
				pop_front_aux();
		}
		void clear();
		iterator erase(iterator pos) {
			iterator next = pos;
			++next;
			difference_type index = pos - start;
			// index清除点之前的元素个数
			// 个数较少时移动清除点之前的元素
			// 否则移动清除点之后的元素
			// 注意排除移动完毕后的冗余元素
			if (index < (size() >> 1)) {
				copy_backward(start, pos, next);
				pop_front();
			}
			else {
				copy(next, finish, pos);
				pop_back();
			}
			return start + index;
		}
		iterator erase(iterator first, iterator last);
		iterator insert(iterator pos, const value_type& x) {
			// 如果插到最前端，调用push_front
			// 如果插到最后段，调用push_back
			if (pos.cur == start.cur) {
				push_front(x);
				return start;
			}
			else if (pos.cur == finish.cur) {
				push_back(x);
				iterator tmp = finish;
				tmp--;
				return tmp;
			}
			else {
				return insert_aux(pos, x);
			}
		}
		

	protected:
		void fill_initialize(size_type n, const value_type& value);
		void create_map_and_nodes(size_type num_elements);

		void push_back_aux(const value_type& value);
		void push_front_aux(const value_type& value);
		void reserve_map_at_back(size_type nodes_to_add = 1) {
			// 如果尾端节点备用空间不足，重新配置更大的map
			if (nodes_to_add + 1 > map_size - (finish.node - map))
				reallocate_map(nodes_to_add, false);
		}
		void reserve_map_at_front(size_type nodes_to_add = 1) {
			// 如果头端节点备用空间不足，重新配置更大的map
			if (nodes_to_add > first.node - map)
				reallocate_map(nodes_to_add, true);
		}
		void reallocate_map(size_type nodes_to_add, bool add_at_front);
		void pop_back_aux();
		void pop_front_aux();
		iterator insert_aux(iterator pos, const value_type& x);

	};

	// fill_initialize产生并安排好deque的结构(由create_map_and_nodes完成)，并设置好元素的初值
	template<class T, class Alloc, size_t BufSizee>
	void deque<T, Alloc, BufSizee>::fill_initialize(size_type n, const value_type& value) {
		create_map_and_nodes(n);
		map_pointer cur;
		//__STL_TRY
		try{
			// 为每个节点的缓冲区设定初值
			for (cur = start.node; cur < finish.node; ++cur)
				uninitialized_fill(*cur, *cur + buffer_size(), value);
			// 最后一个节点因为尾端可能有备用空间不设初值
			uninitialized_fill(finish.first, finish.cur, value);
		}
		catch (...) {}
	}

	template<class T, class Alloc, size_t BufSizee>
	void deque<T, Alloc, BufSizee>::create_map_and_nodes(size_type num_elements) {
		// 节点数 = （元素个数 / 每个缓冲区可容纳的元素个数）+ 1
		// 如果刚好整除，会多配置一个节点
		size_type num_nodes = num_elements / buffer_size() + 1;

		// 一个map容纳的节点数。最少8个，最多所需节点数+2
		// （前后各留一个节点用于扩充）
		map_size = max(initial_map_size(), num_nodes + 2);
		map = amp_allocator::allocate(map_size);

		// 分别指向头尾节点的中间位置确保两端扩充一样大
		map_pointer nstart = map + (map_size - num_nodes) / 2;
		map_pointer nfinish = nstart + num_nodes - 1;

		map_pointer cur;
		//__STL_TRY
		try {
			// 为map内每个现用节点配置缓冲区
			for (cur = nstart; cur <= nfinish; ++cur)
				*cur = allocate_node();
		}
		catch (...) {
			// "commit or rollback"
		}

		// 设置头尾迭代器的位置
		start.set_node(nstart);
		finish.set_node(nfinish);
		start.cur = start.first;
		finish.cur = finish.first + num_elements % buffer_size();
	}

	template<class T, class Alloc, size_t BufSizee>
	void deque<T, Alloc, BufSizee>::push_back_aux(const value_type& value) {
		value_type value_copy = value;
		// 是否扩充mamp
		reserve_map_at_back();
		// 配置新的缓冲区
		*(finish.node + 1) = allocate_node(); 
		//__STL_TRY
		try {
			// 构造并设置finish指向新节点及调整指针位置
			_construct(finish.cur, value_copy);
			finish.set_node(finish.node + 1);
			finish.cur = finish.first;
		}
		//__STL_UNWIND
		catch (...) {
			deallocate_node(*(finish.node + 1));
		}
	}

	template<class T, class Alloc, size_t BufSizee>
	void deque<T, Alloc, BufSizee>::push_front_aux(const value_type& value) {
		value_type value_copy = value;
		// 是否扩充mamp
		reserve_map_at_front();
		// 配置新的缓冲区
		*(start.node - 1 1) = allocate_node();
		//__STL_TRY
		try {
			// 设置start指向新节点及调整指针位置并构造
			start.set_node(start.node - 1);
			start.cur = start.last - 1;
			_construct(start.cur, value_copy);

		}
		catch (...) {
			start.set_node(start.node + 1);
			start.cur = start.first;
			deallocate_node(*(start.node - 1));
			throw;
		}
	}

	template<class T, class Alloc, size_t BufSize>
	void deque<T, Alloc, BufSize>::reallocate_map(size_type nodes_to_add, bool add_at_front) {
		size_type old_num_nodes = finish.node - start.node + 1;
		size_type new_num_nodes = old_num_nodes + nodes_to_add;

		map_pointer new_nstart;
		if (map_size > 2 * new_num_nodes) {
			new_nstart = map + (map_size - new_num_nodes) / 2
				+ (add_at_front ? nodes_to_add : 0);
			if (new_nstart < start.node)
				copy(start.node, finish.node + 1, new_nstart);
			else
				copy_backward(start.node, finish.node + 1, new_nstart + old_num_nodes);
		}
		else {
			size_type new_map_size = map_size + max(map_size, nodes_to_add) + 2;
			map_pointer new_map = map_allocator::allocate(new_map_size);
			new_nstart = new_map + (new_map_size - new_num_nodes) / 2
				+ (add_at_front ? nodes_to_add : 0);
			copy(start.node, finish.node + 1, new_nstart);
			map_allocator::deallocate(map, map_size);

			map = new_map;
			map_size = new_map_size;
		}

		start.set_node(new_nstart);
		finish.set_node(new_nstart + old_num_nodes - 1);
	}

	template<class T, class Alloc, size_t BufSize>
	void deque<T, Alloc, BufSize>::pop_back_aux() {
		deallocate_node(finish.first);	// 释放尾缓冲区
		finish.set_node(finish.node - 1);	// 调整finish指向上一缓冲区的最后一个元素
		finish.cur = finish.last - 1;	
		destroy(finish.cur);	// 析构最后一个元素
	}

	template<class T, class Alloc, size_t BufSize>
	void deque<T, Alloc, BufSize>::pop_front_aux() {
		destroy(start.cur);	// 析构头缓冲区的最后一个元素
		deallocate_node(start.first);	// 释放头缓冲区
		start.set_node(start.node + 1);	// 调整start指向下一缓冲区的第一个元素
		start.cur = start.first;
	}

	template<class T, class Alloc, size_t BufSize>
	void deque<T, Alloc, BufSize>::clear() {
		// 将头尾以外缓冲区内所有元素析构并释放缓冲区
		for (map_pointer node = start.node + 1; node < finish.node; ++node) {
			destroy(*node, *node + buffer_size());
			data_allocator::deallocate(*node, buffer_size());
		}

		// 头尾缓冲区分别是两个缓冲区时，析构两者全部元素
		// 释放尾缓冲区，保留头缓冲区
		// 否则仅析构缓冲区内全部元素
		if (start.node != finish.node) {
			destroy(start.cur, start.last);
			destroy(finish.first, finish.cur);
			data_allocator::deallocate(finish.first, buffer_size());
		}
		else {
			destroy(start.cur, finish.cur);
		}
		finish = start;
	}

	template<class T, class Alloc, size_t BufSize>
	deque<T, Alloc, BufSize>::iterator
	deque<T, Alloc, BufSize>::erase(iterator first, iterator last)
	{
		// 如果清除区域是整个deque，直接clear
		if (first == start && last == finish) {
			clear();
			return finish;
		}
		else {
			difference_type n = last - first;
			difference_type elems_before = first - start;
			// 清除区间长度n、清除区间前方元素个数elems_before
			// 如果前方元素少，向后移动前方元素
			// 否则向前移动后方元素
			// 注意标记新的start和finish并清除移动后的冗余元素
			if (elems_before < (size() - n) / 2) {
				copy_backward(start, first, last);
				iterator new_start = start + n;
				destroy(start, new_start);
				for (map_pointer cur = start.node; cur < new_start.node; ++cur)
					data_allocator::deallocate(*cur, buffer_size());
				start = new_start;
			}
			else {
				copy(last, finish, first);
				iterator new_finish = finish - n;
				destroy(new_finish, finish);
				for (map_pointer cur = new_finish.node + 1, cur <= first; ++cur)
					data_allocator::deallocate(*cur, buffer_size());
				finish = new_finish;
			}
			return start + elems_before;
		}
	}

	template<class T, class Alloc, size_t BufSize>
	deque<T, Alloc, BufSize>::iterator
		deque<T, Alloc, BufSize>::insert_aux(iterator pos, const value_type& x)
	{
		difference_type index = pos - start;
		value_type x_copy = x;
		// index插入点前的元素个数
		// 如果前方元素少，在最前端加入与第一元素同值的元素并进行移动
		// 否则在最尾端加入与最后元素同值的元素并移动
		if (index < size() / 2) {
			push_front(front());
			iterator front1 = start;
			++front1;
			iterator front2 = front1;
			++front2;
			pos = start + index;
			iterator pos1 = pos;
			++pos1;
			copy(front2, pos1, front1);
		}
		else {
			push_back(back());
			iterator back1 = finish;
			--back1;
			iterator back2 = back1;
			--back2;
			pos = start + index;
			copy_backward(pos, back2, back1);
		}
		*pos = x_copy;
		return pos;
	}

}

#endif // !_CHUSTL_DEQUE_H
