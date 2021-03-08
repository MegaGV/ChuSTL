#pragma once

#ifndef _CHUSTL_QUEUE_H
#define _CHUSTL_QUEUE_H

#include "Deque.h"

namespace ChuSTL {

	template<class T, class Alloc, class Container = deque<T, Alloc>>
	class queue {
		friend bool operator== (const queue&, const queue&);
		friend bool operator< (const queue&, const queue&);

	public:
		typedef typename Container::value_type		value_type;
		typedef typename Container::size_type		size_type;
		typedef typename Container::reference		reference;
		typedef typename Container::const_reference	const_reference;

	protected:
		Container c;

	public:
		bool empty() const { return c.empty(); }
		size_type size() const { return c.size(); }
		reference front() { return c.front(); }
		const_reference front() const { return c.front(); }
		reference back() { return c.back(); }
		const_reference frbackont() const { return c.back(); }
		void push(const value_type& x) { c.push_back(x); }
		void pop() { c.pop_front(); }

	};

	template<class T, class Container>
	bool operator==(const queue<T, Container>& x, const queue<T, Container>& y) {
		return x.c == y.c;
	};

	template<class T, class Container>
	bool operator< (const queue<T, Container>& x, const queue<T, Container>& y) {
		return x.c < y.c;
	}
}

#endif // !_CHUSTL_QUEUE_H
