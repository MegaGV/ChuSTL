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
		typedef T*								iterator;	// vector�ĵ���������ָͨ��
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
		iterator start;				// Ŀǰʹ�ÿռ��ͷ��
		iterator finish;			// Ŀǰʹ�ÿռ��β��
		iterator end_of_storage;	// Ŀǰ���ÿռ��β��

		void insert_aux(iterator position, const T& x);
		void insert(iterator position, size_type n, const T& x);

		void deallocate() {
			if (start) {
				data_allocator::deallocate(start, end_of_storage - start);
			}
		}

		// ��䲢���Գ�ʼ��
		void fill_initialize(size_type n, const T& value) {
			start = allocate_and_fill(n, value);
			finish = start + n;
			end_of_storage = finish;
		}

		// ���ö������
		iterator allocate_and_fill(size_type n, const T& x) {
			iterator result = data_allocator::allocate(n);	// ����n��Ԫ�ؿռ�
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

		// ���캯��������ָ��vector��Сn�ͳ�ֵvalue
		vector() : start(0), finish(0), end_of_storage(0) {}
		vector(size_type n, const T& value) { fill_initialize(n, value); }
		vector(int n, const T& value) { fill_initialize(n, value); }
		vector(long n, const T& value) { fill_initialize(n, value); }
		explicit vector(size_type n) { fill_initialize(n, T()); }

		~vector() {
			destroy(start, finish);
			deallocate();
		}
		reference front() { return *begin(); }	// ���ص�һ��Ԫ��
		reference back() { return *(end() - 1); }	// �������һ��Ԫ��
		pointer data() { return start; }	// ����ͷ��ָ��

		void push_back(const T& x) {	// ��Ԫ�ز��������
			if (finish != end_of_storage) {
				construct(finish, x);
				++finish;
			}
			else {
				insert_aux(end(), x);
			}
		}

		void pop_back() {	// �����Ԫ��ȡ��
			--finish;
			destroy(finish);
		}

		iterator erase(iterator position) {	// ���ĳλ���ϵ�Ԫ��
			if (position + 1 != end()) {
				copy(position + 1, finish, position);	// ����Ԫ��ǰ��
			}
			--finish;
			destroy(finish);
			return position;
		}
		iterator erase(iterator first, iterator last) { // ���[first, last)�е�����Ԫ��
			iterator i = copy(last, finish, first);
			destroy(i, finish);
			finish = finish - (last - first);
			return first;
		}
		void resize(size_type new_size, const T& x) {
			if (new_size < size())	// �¿ռ�С��ԭ�ռ䣬�ͷŶ���ռ�
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
			// �ڱ��ÿռ���ʼ������һ��Ԫ�أ�����vector���һ��Ԫ��ֵΪ���ֵ
			construct(finish, *(finish - 1);
			++finish;
			T x_copy = x;
			copy_backward(position, finish - 2, finish - 1);
			*position = x_copy;
		}
		else {
			const size_type old_size = size();
			const size_type len = old_size != 0 ? 2 * old_size : 1;
			// ���ԭ��СΪ0��������1��Ԫ�ش�С
			// ���ԭ��С��Ϊ0��������ԭ��С����

			iterator new_start = data_allocator::allocate(len);
			iterator new_finish = new_start;

			try {
				// ��ԭvector���ݿ�������vector
				new_finish = uninitialized_copy(start, position, new_start);
				// Ϊ��Ԫ���趨��ֵx
				construct(new_finish, x);
				++new_finish;
				// ��������ԭ����Ҳ��������
				new_finish = uninitialized_copy(position, finish, new_finish);
			}
			catch (...) {
				// "commit or rollback" semantics.
				destroy(new_start, new_finish);
				data_allocator::deallocate(new_start, len);
				throw;
			}

			// �������ͷ�ԭvector
			destroy(begin(), end());
			deallocate();

			// ������������ָ����vector
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
				// ���ÿռ���ڵ�������Ԫ�ظ���
				T x_copy = x;
				// ���¼�������֮�������Ԫ�ظ���
				const size_type elems_after = finish - position;
				iterator old_finish = finish;
				if (elems_after > n) {
					// �����֮�������Ԫ�ظ�����������Ԫ�ظ���
					uninitialized_copy(finish - n, finish, finish);
					finish += n;
					copy_backward(position, old_finish - n, old_finish);
					fill(position, position + n, x_copy);
				}
				else {
					// �����֮�������Ԫ�ظ���С�ڵ�������Ԫ�ظ���
					uninitialized_fill_n(finish, n - elems_after, x_copy);
					finish += n - elems_after;
					uninitialized_copy(position, old_finish, finish);
					finish += elems_after;
					fill(position, old_finish, x_copy);
				}
			}
			else {
				// ���ÿռ�С������Ԫ�ظ����������ö����ڴ�
				// �����³��ȣ��ɳ���������ɳ���+����Ԫ�ظ���
				const size_type old_size = size();
				const size_type len = old_size + max(old_size, n);
				// �����µ�vector�ռ�
				iterator new_start = data_allocator::allocate(len);
				iterator new_finish = new_start;
				__STL_TRY{
					// ���Ƚ���vector�Ĳ����֮ǰ��Ԫ�ظ��Ƶ��¿ռ�
					new_finish = uninitialized_copy(start, position, new_start);
					// ������Ԫ�������¿ռ䣨��ֵΪn��
					new_finish = uninitialized_fill_n(new_finish, n, x);
					// ����vector�������Ԫ�ظ��Ƶ��¿ռ�
					new_finish = uninitialized_copy(position, finish, new_finish);
				}
		#ifdef __STL_USE_EXCEPTIONS
					catch (...) {
					// �����쳣������ʵ��"commit or rollback" semantics.
					destroy(new_start, new_finish);
					data_allocator::deallocate(new_start, len);
					throw;
				}
		#endif // __STL_USE_EXCEPTIONS
				// ������ͷž�vector
				destroy(start, finish);
				deallocate();
				// ������vectorλ�ñ��
				start = new_start;
				finish = new_finish;
				end_of_storage = new_start + len;
			}
			
		}
		
	}
}

#endif  // !_CHUSTL_VECTOR_H_