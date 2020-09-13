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

		T* cur;				// �õ�������ָ�������еĵ�ǰԪ��
		T* first;			// �õ�������ָ��������ͷ
		T* last;			// �õ�������ָ��������β
		map_pointer node;	// ָ��ܿ�����

		static size_type buffer_size() {
			return __deque_buf_size(BufSize, sizeof(value_type));
		}
		// ���n��Ϊ0������n����ʾbuffer size�û��Զ���
		// ���nΪ0����ʾbuffer sizeʹ��Ĭ��ֵ����
		//	���sz��Ԫ�ش�С��sizeof(value_type)��С��512�� ���� 512/sz
		//	���sz����512�� ����1
		inline size_type __deque_buf_size(size_type n, size_type sz) {
			return n != 0 ? n : (sz < 512 ? size_type(512 / sz) : size_type(1));
		}

		// �л�ָ��Ļ�����
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

		// ��Ŀ��������أ�����ʽ����ǰ��ʽ��ɣ����ƶ�ʱ��Ҫ�ж��Ƿ���Ҫ���л������л�
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
		// �����ȡ����������ֱ����Ծ�������
		self& operator+=(difference_type n) {
			difference_type offset = n + (cur - first);
			// Ŀ��λ������ͬһ�������ڣ�ֱ���ƶ�
			if (offset >= 0 && offset < difference_type(buffer_size())) {
				cur += n;
			}
			// Ŀ��λ�ò���ͬһ�����������л����µĻ��������ƶ�
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
		// map�ǿ������ռ䣬����ÿ��Ԫ�ض���һ��ָ��(�ڵ�)��ָ��һ�黺����
		// map_size��map�ڿ����ɵ�ָ����
		map_pointer map;
		size_type map_size;

		// ר���ռ���������ÿ��Ϊһ��Ԫ��(ָ��)���ÿռ�
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
			// β����������������Ԫ�صı��ÿռ�ʱ��ֱ���ڱ��ÿռ��Ϲ���Ԫ��
			// ���������µĻ�����
			if (finish.cur != finish.last - 1) {
				construct(finish.cur, value);
				++finish.cur;
			}
			else {
				push_back_aux(value);
			}
		}
		void push_front(const value_type& value) {
			// ͷ���������б��ÿռ�ʱ��ֱ���ڱ��ÿռ��Ϲ���Ԫ��
			// ���������µĻ�����
			if (start.cur != start.first) {
				construct(start.cur, value);
				--start.cur;
			}
			else {
				push_front_aux(value);
			}
		}
		void pop_back() {
			// β����������Ԫ��ʱ������ָ���������Ԫ��
			// �����ͷ�β������
			if (finish.cur != finish.first) {
				--finish.cur;
				destroy(finish.cur);
			}
			else
				pop_back_aux();
		}
		void pop_front() {
			// ͷ������������������Ԫ��ʱ������ָ��������ǰԪ��
			// �����ͷ�ͷ������
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
			// index�����֮ǰ��Ԫ�ظ���
			// ��������ʱ�ƶ������֮ǰ��Ԫ��
			// �����ƶ������֮���Ԫ��
			// ע���ų��ƶ���Ϻ������Ԫ��
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
			// ����嵽��ǰ�ˣ�����push_front
			// ����嵽���Σ�����push_back
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
			// ���β�˽ڵ㱸�ÿռ䲻�㣬�������ø����map
			if (nodes_to_add + 1 > map_size - (finish.node - map))
				reallocate_map(nodes_to_add, false);
		}
		void reserve_map_at_front(size_type nodes_to_add = 1) {
			// ���ͷ�˽ڵ㱸�ÿռ䲻�㣬�������ø����map
			if (nodes_to_add > first.node - map)
				reallocate_map(nodes_to_add, true);
		}
		void reallocate_map(size_type nodes_to_add, bool add_at_front);
		void pop_back_aux();
		void pop_front_aux();
		iterator insert_aux(iterator pos, const value_type& x);

	};

	// fill_initialize���������ź�deque�Ľṹ(��create_map_and_nodes���)�������ú�Ԫ�صĳ�ֵ
	template<class T, class Alloc, size_t BufSizee>
	void deque<T, Alloc, BufSizee>::fill_initialize(size_type n, const value_type& value) {
		create_map_and_nodes(n);
		map_pointer cur;
		//__STL_TRY
		try{
			// Ϊÿ���ڵ�Ļ������趨��ֵ
			for (cur = start.node; cur < finish.node; ++cur)
				uninitialized_fill(*cur, *cur + buffer_size(), value);
			// ���һ���ڵ���Ϊβ�˿����б��ÿռ䲻���ֵ
			uninitialized_fill(finish.first, finish.cur, value);
		}
		catch (...) {}
	}

	template<class T, class Alloc, size_t BufSizee>
	void deque<T, Alloc, BufSizee>::create_map_and_nodes(size_type num_elements) {
		// �ڵ��� = ��Ԫ�ظ��� / ÿ�������������ɵ�Ԫ�ظ�����+ 1
		// ����պ��������������һ���ڵ�
		size_type num_nodes = num_elements / buffer_size() + 1;

		// һ��map���ɵĽڵ���������8�����������ڵ���+2
		// ��ǰ�����һ���ڵ��������䣩
		map_size = max(initial_map_size(), num_nodes + 2);
		map = amp_allocator::allocate(map_size);

		// �ֱ�ָ��ͷβ�ڵ���м�λ��ȷ����������һ����
		map_pointer nstart = map + (map_size - num_nodes) / 2;
		map_pointer nfinish = nstart + num_nodes - 1;

		map_pointer cur;
		//__STL_TRY
		try {
			// Ϊmap��ÿ�����ýڵ����û�����
			for (cur = nstart; cur <= nfinish; ++cur)
				*cur = allocate_node();
		}
		catch (...) {
			// "commit or rollback"
		}

		// ����ͷβ��������λ��
		start.set_node(nstart);
		finish.set_node(nfinish);
		start.cur = start.first;
		finish.cur = finish.first + num_elements % buffer_size();
	}

	template<class T, class Alloc, size_t BufSizee>
	void deque<T, Alloc, BufSizee>::push_back_aux(const value_type& value) {
		value_type value_copy = value;
		// �Ƿ�����mamp
		reserve_map_at_back();
		// �����µĻ�����
		*(finish.node + 1) = allocate_node(); 
		//__STL_TRY
		try {
			// ���첢����finishָ���½ڵ㼰����ָ��λ��
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
		// �Ƿ�����mamp
		reserve_map_at_front();
		// �����µĻ�����
		*(start.node - 1 1) = allocate_node();
		//__STL_TRY
		try {
			// ����startָ���½ڵ㼰����ָ��λ�ò�����
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
		deallocate_node(finish.first);	// �ͷ�β������
		finish.set_node(finish.node - 1);	// ����finishָ����һ�����������һ��Ԫ��
		finish.cur = finish.last - 1;	
		destroy(finish.cur);	// �������һ��Ԫ��
	}

	template<class T, class Alloc, size_t BufSize>
	void deque<T, Alloc, BufSize>::pop_front_aux() {
		destroy(start.cur);	// ����ͷ�����������һ��Ԫ��
		deallocate_node(start.first);	// �ͷ�ͷ������
		start.set_node(start.node + 1);	// ����startָ����һ�������ĵ�һ��Ԫ��
		start.cur = start.first;
	}

	template<class T, class Alloc, size_t BufSize>
	void deque<T, Alloc, BufSize>::clear() {
		// ��ͷβ���⻺����������Ԫ���������ͷŻ�����
		for (map_pointer node = start.node + 1; node < finish.node; ++node) {
			destroy(*node, *node + buffer_size());
			data_allocator::deallocate(*node, buffer_size());
		}

		// ͷβ�������ֱ�������������ʱ����������ȫ��Ԫ��
		// �ͷ�β������������ͷ������
		// �����������������ȫ��Ԫ��
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
		// ����������������deque��ֱ��clear
		if (first == start && last == finish) {
			clear();
			return finish;
		}
		else {
			difference_type n = last - first;
			difference_type elems_before = first - start;
			// ������䳤��n���������ǰ��Ԫ�ظ���elems_before
			// ���ǰ��Ԫ���٣�����ƶ�ǰ��Ԫ��
			// ������ǰ�ƶ���Ԫ��
			// ע�����µ�start��finish������ƶ��������Ԫ��
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
		// index�����ǰ��Ԫ�ظ���
		// ���ǰ��Ԫ���٣�����ǰ�˼������һԪ��ֵͬ��Ԫ�ز������ƶ�
		// ��������β�˼��������Ԫ��ֵͬ��Ԫ�ز��ƶ�
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
