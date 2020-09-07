#pragma once

#ifndef _CHUSTL_VECTOR_H_
#define _CHUSTL_VECTOR_H_

#include "Allocator.h"
#include "Alloc.h"
#include "Iterator.h"

namespace ChuSTL {

	template<class T, class Alloc> // class Alloc = alloc
	class vector {
	public:
		typedef T								value_type;
		typedef T*								pointer;
		typedef const T*						const_pointer;
		typedef T*								iterator;	// vector的迭代器是普通指针
		typedef const T*						const_iterator;
		//typedef reverse_iterator_t<T*>			reverse_iterator;
		//typedef reverse_iterator_t<const T*>	const_reverse_iterator;
		typedef T&								reference;
		typedef const T&						const_reference;
		typedef size_t							size_type;
		typedef ptrdiff_t						difference_type;

	protected:
		//typedef simple_alloc<value_type, Alloc> data_allocator;
		typedef Alloc data_allocator;
		iterator start;				// 目前使用空间的头部
		iterator finish;			// 目前使用空间的尾部
		iterator end_of_storage;	// 目前可用空间的尾部

		void insert_aux(iterator position, const T& x);
		void insert(iterator position, size_type n, const T& x);

		void deallocate() {
			if (start) {
				data_allocator::deallocate(start, end_of_storage - start);
			}
		}

		// 填充并予以初始化
		void fill_initialize(size_type n, const T& value) {
			start = allocate_and_fill(n, value);
			finish = start + n;
			end_of_storage = finish;
		}

		// 配置而后填充
		iterator allocate_and_fill(size_type n, const T& x) {
			iterator result = data_allocator::allocate(n);	// 配置n个元素空间
			uninitialized_fill_n(result, n, x);
			return result;
		}

	public:
		iterator begin() { return start; }
		const_iterator cbegin() const { return start; }
		iterator end() { return finish; }
		const_iterator cend() const { return finish; }

		size_type size() const { return size_type(end() - begin()); }
		size_type capacity() const { return size_type(end_of_storage - begin()); }
		bool empty() const { return begin() = end(); }
		reference operator[](size_type n) { return *(begin() + n); }
		const_reference operator[](size_type n) const { return *(cbegin() + n); }

		// 构造函数，允许指定vector大小n和初值value
		vector() : start(0), finish(0), end_of_storage(0) {}
		vector(size_type n, const T& value) { fill_initialize(n, value); }
		vector(int n, const T& value) { fill_initialize(n, value); }
		vector(long n, const T& value) { fill_initialize(n, value); }
		explicit vector(size_type n) { fill_initialize(n, T()); }

		~vector() {
			destroy(start, finish);
			deallocate();
		}
		reference front() { return *begin(); }	// 返回第一个元素
		reference back() { return *(end() - 1); }	// 返回最后一个元素
		pointer data() { return start; }	// 返回头部指针

		void push_back(const T& x) {	// 将元素插入至最后
			if (finish != end_of_storage) {
				construct(finish, x);
				++finish;
			}
			else {
				insert_aux(end(), x);
			}
		}

		void pop_back() {	// 将最后元素取出
			--finish;
			destroy(finish);
		}

		iterator erase(iterator position) {	// 清除某位置上的元素
			if (position + 1 != end()) {
				copy(position + 1, finish, position);	// 后续元素前移
			}
			--finish;
			destroy(finish);
			return position;
		}
		iterator erase(iterator first, iterator last) { // 清除[first, last)中的所有元素
			iterator i = copy(last, finish, first);
			destroy(i, finish);
			finish = finish - (last - first);
			return first;
		}
		void resize(size_type new_size, const T& x) {
			if (new_size < size())	// 新空间小于原空间，释放多余空间
				erase(begin() + new_size, end());
			else
				insert(end(), new_size - size(), x);
		}
		void resize(size_type new_size) {
			resize(new_size, T());
		}
		void clear() {
			erase(begin(), end());
		}

		/*

		template<class InputIterator>
		vector(InputIterator first, InputIterator last);
		template<class Integer>
		vector_aux(Integer n, const T& value, std::true_type);
		vector(const vector& v);
		vector<T, Alloc>& operator=(const vector& v);

		reverse_iterator rend();
		const_reverse_iterator crend() const;
		reverse_iterator rbegin();
		const_reverse_iterator crbegin() const;

		void reserve(size_type n);

		template<class InputIterator>
		void reallocateAndCopy(iterator position, InputIterator first, InputIterator last);

		*/
	};

	template<class T, class Alloc>
	void vector<T, Alloc>::insert_aux(iterator position, const T& x)
	{
		if (finish != end_of_storage) {
			// 在备用空间起始处构造一个元素，并以vector最后一个元素值为其初值
			construct(finish, *(finish - 1);
			++finish;
			T x_copy = x;
			copy_backward(position, finish - 2, finish - 1);
			*position = x_copy;
		}
		else {
			const size_type old_size = size();
			const size_type len = old_size != 0 ? 2 * old_size : 1;
			// 如果原大小为0，则配置1个元素大小
			// 如果原大小不为0，则配置原大小两倍

			iterator new_start = data_allocator::allocate(len);
			iterator new_finish = new_start;

			try {
				// 将原vector内容拷贝到新vector
				new_finish = uninitialized_copy(start, position, new_start);
				// 为新元素设定初值x
				construct(new_finish, x);
				++new_finish;
				// 将安插点的原内容也拷贝过来
				new_finish = uninitialized_copy(position, finish, new_finish);
			}
			catch (...) {
				// "commit or rollback" semantics.
				destroy(new_start, new_finish);
				data_allocator::deallocate(new_start, len);
				throw;
			}

			// 析构并释放原vector
			destroy(begin(), end());
			deallocate();

			// 调整迭代器，指向新vector
			start = new_start;
			finish = new_finish;
			end_of_storage = new_start + len;
		}
	}

	template<class T, class Alloc>
	void vector<T, Alloc>::insert(iterator position, size_type n, const T& x)
	{
		if (n != 0) {
			if (size_type(end_of_storage - finish) >= n) {
				// 备用空间大于等于新增元素个数
				T x_copy = x;
				// 以下计算插入点之后的现有元素个数
				const size_type elems_after = finish - position;
				iterator old_finish = finish;
				if (elems_after > n) {
					// 插入点之后的现有元素个数大于新增元素个数
					uninitialized_copy(finish - n, finish, finish);
					finish += n;
					copy_backward(position, old_finish - n, old_finish);
					fill(position, position + n, x_copy);
				}
				else {
					// 插入点之后的现有元素个数小于等于新增元素个数
					uninitialized_fill_n(finish, n - elems_after, x_copy);
					finish += n - elems_after;
					uninitialized_copy(position, old_finish, finish);
					finish += elems_after;
					fill(position, old_finish, x_copy);
				}
			}
			else {
				// 备用空间小于新增元素个数，需配置额外内存
				// 决定新长度：旧长度两倍或旧长度+新增元素个数
				const size_type old_size = size();
				const size_type len = old_size + max(old_size, n);
				// 配置新的vector空间
				iterator new_start = data_allocator::allocate(len);
				iterator new_finish = new_start;
				__STL_TRY{
					// 首先将旧vector的插入点之前的元素复制到新空间
					new_finish = uninitialized_copy(start, position, new_start);
					// 将新增元素填入新空间（初值为n）
					new_finish = uninitialized_fill_n(new_finish, n, x);
					// 将旧vector插入点后的元素复制到新空间
					new_finish = uninitialized_copy(position, finish, new_finish);
				}
		#ifdef __STL_USE_EXCEPTIONS
					catch (...) {
					// 如有异常发生，实现"commit or rollback" semantics.
					destroy(new_start, new_finish);
					data_allocator::deallocate(new_start, len);
					throw;
				}
		#endif // __STL_USE_EXCEPTIONS
				// 清除并释放旧vector
				destroy(start, finish);
				deallocate();
				// 调整新vector位置标记
				start = new_start;
				finish = new_finish;
				end_of_storage = new_start + len;
			}
			
		}
		
	}
}

#endif  // !_CHUSTL_VECTOR_H_