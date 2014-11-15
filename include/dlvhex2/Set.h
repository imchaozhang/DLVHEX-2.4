/* dlvhex -- Answer-Set Programming with external interfaces.
 * Copyright (C) 2005, 2006, 2007 Roman Schindlauer
 * Copyright (C) 2006, 2007, 2008, 2009, 2010 Thomas Krennwallner
 * Copyright (C) 2009, 2010 Peter Schüller
 * Copyright (C) 2011, 2012, 2013 Christoph Redl
 * 
 * This file is part of dlvhex.
 *
 * dlvhex is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * dlvhex is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with dlvhex; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

/**
 * @file   Set.hpp
 * @author Christoph Redl <redl@kr.tuwien.ac.at>
 * 
 * @brief  Set data structures implemented as dynamically allocated arrays.
 */

#ifndef SET_HPP_INCLUDED__09122011
#define SET_HPP_INCLUDED__09122011

#include <iterator>
#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>
#include "dlvhex2/DynamicVector.h"

template<typename T>
class Set;

template<typename T>
class insert_set_iterator : public std::iterator<std::output_iterator_tag, void, void, void, void>{
protected:
	Set<T>& set;
public:
	insert_set_iterator(Set<T>& s) : set(s){
	}

	insert_set_iterator& operator=(const typename Set<T>::value_type& v){
		set.insert(v);
		return *this;
	}

	insert_set_iterator& operator*(){
		return *this;
	}

	insert_set_iterator& operator++(){
		return *this;
	}

	insert_set_iterator operator++(int){
		return *this;
	}
};

template<typename T>
class const_set_iterator : public std::iterator<std::input_iterator_tag, T, ptrdiff_t, const T*, const T&>{
protected:
	const Set<T>& set;
	int loc;
public:
	const_set_iterator(const Set<T>& s, int l = 0) : set(s), loc(l){
	}

	inline const T& operator*() const{
		return set.getData()[loc];
	}

	inline const T* operator->() const{
		return &(set.getData()[loc]);
	}

	inline const_set_iterator& operator++(){
		++loc;
		return *this;
	}

	inline const_set_iterator operator++(int){
		const_set_iterator<T> old(*this);
		operator++();
		return old;
	}
	
	inline const_set_iterator& operator--(){
		--loc;
		return *this;
	}

	inline const_set_iterator operator--(int){
		const_set_iterator<T> old(*this);
		operator--();
		return old;
	}

	inline const_set_iterator operator+(int i) const{
		return const_set_iterator<T>(set, loc + 1);
	}
	
	inline const_set_iterator operator+(const_set_iterator& it) const{
		return const_set_iterator<T>(set, loc + it.loc);
	}

	inline const_set_iterator operator-(int i) const{
		return const_set_iterator<T>(set, loc - i);
	}
	
	inline const_set_iterator operator-(const_set_iterator& it) const{
		return const_set_iterator<T>(set, loc - it.loc);
	}

	inline operator const int() const{
		return loc;
	}

	inline bool operator==(const_set_iterator const& sit2) const{
		return loc == sit2.loc;
	}

	inline bool operator!=(const_set_iterator const& sit2) const{
		return loc != sit2.loc;
	}
};

template<typename T>
class set_iterator : public std::iterator<std::input_iterator_tag, T, ptrdiff_t, T*, T&>{
protected:
	Set<T>& set;
	int loc;
public:
	set_iterator(Set<T>& s, int l = 0) : set(s), loc(l){
	}

	inline T& operator*(){
		return set.getData()[loc];
	}

	inline T* operator->(){
		return &(set.getData()[loc]);
	}

	inline set_iterator& operator++(){
		++loc;
		return *this;
	}

	inline set_iterator operator++(int){
		set_iterator<T> old(*this);
		operator++();
		return old;
	}
	
	inline set_iterator& operator--(){
		--loc;
		return *this;
	}

	inline set_iterator operator--(int){
		set_iterator<T> old(*this);
		operator--();
		return old;
	}

	inline set_iterator operator+(int i) const{
		return set_iterator<T>(set, loc + i);
	}
	
	inline set_iterator operator+(set_iterator& it) const{
		return set_iterator<T>(set, loc + it.loc);
	}

	inline set_iterator operator-(int i) const{
		return set_iterator<T>(set, loc - i);
	}
	
	inline set_iterator operator-(set_iterator& it) const{
		return set_iterator<T>(set, loc - it.loc);
	}

	inline operator const int() const{
		return loc;
	}

	inline bool operator==(set_iterator const& sit2) const{
		return loc == sit2.loc;
	}

	inline bool operator!=(set_iterator const& sit2) const{
		return loc != sit2.loc;
	}
};

template<typename T>
class Set{
private:
	T* data;

	int allocSize;
	int rsize;
	int increase;

	void grow(){
		allocSize += increase;
		data = (T*)realloc(data, sizeof(T) * allocSize);
	}

	void grow(int minSize){
		allocSize = minSize % increase == 0
					?	minSize
					: (minSize / increase + 1) * increase;
		data = (T*)realloc(data, sizeof(T) * allocSize);
	}

	int binarySearch(T e) const{
		if (rsize == 0) return 0;

		int lower = 0;
		int upper = rsize - 1;
		int pos = (lower + upper) / 2;

		while (data[pos] != e){
			if (e < data[pos]){
				if (pos > lower){
					upper = pos - 1;
				}else{
					// element is not contained: return insert location
					return pos;
				}
			}else{
				if (pos < upper){
					lower = pos + 1;
				}else{
					// element is not contained: return insert location
					return pos + 1;
				}
			}
			pos = (lower + upper) / 2;
		}
		// element is contained
		return pos;
	}

public:
	typedef set_iterator<T> iterator;
	typedef const_set_iterator<T> const_iterator;
	typedef T value_type;
	typedef const T& const_reference;

	Set(int initialSize = 0, int inc = 10) : increase(inc), rsize(0){
		if (initialSize == 0){
			data = 0;
		}else{
			data = (T*)realloc(0, sizeof(T) * initialSize);
		}
		allocSize = initialSize;
	}

	Set(const Set<T>& s2){
		data = (T*)realloc(0, sizeof(T) * s2.allocSize);
		allocSize = s2.allocSize;
		increase = s2.increase;
		rsize = s2.rsize;
		memcpy(data, s2.data, sizeof(T) * rsize);
	}

	virtual ~Set(){
		if (data) free(data);
		data = 0;
	}

	inline bool contains(T e) const{
		int p =  binarySearch(e);
		return (p < rsize) && (data[p] == e);
	}

	inline int count(T e) const{		// for compatibility with std::set
		return contains(e) ? 1 : 0;
	}

	inline void insert(T e){
		// find location for insertion
		int p = binarySearch(e);
		if (p < rsize && data[p] == e) return;	// already contained

		if (rsize == allocSize){
			grow();
		}

		// shift
		memmove(&(data[p + 1]), &(data[p]), sizeof(T) * (rsize - p));

		// insert
		data[p] = e;
		rsize++;
	}

	//	template<typename _Iter>
	template<typename _Iter>
	inline void insert(_Iter begin, _Iter end){
		grow(size() + (end - begin));
		for (_Iter it = begin; it != end; ++it){
			insert(*it);
		}
	}

	inline void erase(T e){
		// find element
		int p = binarySearch(e);
		if (p >= rsize) return;		// not contained
		if (data[p] != e) return;	// not contained

		// shift
		memmove(&(data[p]), &(data[p + 1]), sizeof(T) * (rsize - (p + 1)));

		rsize--;
	}

	inline set_iterator<T> find(T e){
		// find element
		int p = binarySearch(e);
		if (p >= rsize) return end();		// not contained
		if (data[p] != e) return end();		// not contained
		return set_iterator<T>(*this, p);
	}

	inline const_set_iterator<T> find(T e) const{
		// find element
		int p = binarySearch(e);
		if (p >= rsize) return ((const Set<T>*)this)->end();		// not contained
		if (data[p] != e) return ((const Set<T>*)this)->end();		// not contained
		return const_set_iterator<T>(*this, p);
	}

	bool empty() const{
		return rsize == 0;
	}

	void clear(){
		rsize = 0;
	}

	set_iterator<T> begin(){
		return set_iterator<T>(*this, 0);
	}

	set_iterator<T> end(){
		return set_iterator<T>(*this, rsize);
	}

	const_set_iterator<T> begin() const{
		return const_set_iterator<T>(*this, 0);
	}

	const_set_iterator<T> end() const{
		return const_set_iterator<T>(*this, rsize);
	}

	inline T& operator[](int i){
		return data[i];
	}

	inline const T& operator[](int i) const{
		return data[i];
	}

	inline int size() const{
		return rsize;
	}

	T* getData(){
		return data;
	}

	const T* getData() const{
		return data;
	}

	Set<T>& operator=(const Set<T>& s2){
		clear();
		grow(s2.size());
		rsize = s2.rsize;
		memcpy(data, s2.data, sizeof(T) * rsize);
		return *this;
	}
};

/*
// alternative implementation based on Set<T>

template<typename T>
struct SetElement{
	T element;
	long index;

	SetElement(){
	}

	SetElement(T el, long i) : element(el), index(i){
	}

	bool operator<(const SetElement<T>& el2) const{
		return element < el2.element;
	}

	bool operator>(const SetElement<T>& el2) const{
		return element > el2.element;
	}
	
	bool operator==(const SetElement<T>& el2) const{
		return element == el2.element;
	}

	bool operator!=(const SetElement<T>& el2) const{
		return element != el2.element;
	}
};
*/

template<typename T>
struct SortElement{
	long index;
	T elem;

	SortElement(){
	}

	SortElement(int i, T el) : index(i), elem(el){
	}

	bool operator<(const SortElement<T>& el2) const{
		return index < el2.index;
	}

	bool operator>(const SortElement<T>& el2) const{
		return index > el2.index;
	}
	
	bool operator==(const SortElement<T>& el2) const{
		return index == el2.index;
	}

	bool operator!=(const SortElement<T>& el2) const{
		return index != el2.index;
	}
};

// implementation based on boost::unordered_map
template<typename T, typename H>
class OrderedSet{
private:
	DynamicVector<T, long> os;
//	boost::unordered_map<T, long, H> os;
	long c;

	void renumber(){
		typedef SortElement<T> SortEl;
		std::vector<SortElement<T> > sorted;
		sorted.reserve(os.size());

//		typedef std::pair<T, long> Pair;
//		BOOST_FOREACH (Pair p, os){
//			sorted.push_back(SortEl(p.second, p.first));
//		}
		for (T i = 0; i < os.size(); ++i){
			if (os.find(i) != os.end()){
				sorted.push_back(SortEl(os[i], i));
			}
		}

		std::sort(sorted.begin(), sorted.end());

		c = 0;
		BOOST_FOREACH (SortEl se, sorted){
			os[se.elem] = c++;
		}
	}

public:
	OrderedSet() : c(0){
	}
	
	inline void insert(T el){
		if (c >= 1000000000){
			renumber();
		}
		os[el] = c++;
	}

	inline void erase(T el){
		os.erase(el);
	}

	long getInsertionIndex(T el){
		return os[el];
	}

	inline int compare(T el1, T el2){
		if (getInsertionIndex(el1) < getInsertionIndex(el2)) return -1;
		else if (getInsertionIndex(el1) > getInsertionIndex(el2)) return +1;
		else return 0;
	}

	void resize(int s){
//		os.resize(s);
	}


/*
// alternative implementation based on Set<T>

private:
	Set<SetElement<T> > os;
	long c;

	// reusable element (to avoid reallocation on stack of insert, erase and getInsertionIndex)
	SetElement<T> tmpEl;

	void renumber(){
		typedef SetElement<T> SetEl;
		typedef SortElement<T> SortEl;
		std::vector<SortElement<T> > sorted;
		sorted.reserve(os.size());
		
		BOOST_FOREACH (SetEl se, os){
			sorted.push_back(SortEl(se.index, se.element));
		}

		std::sort(sorted.begin(), sorted.end());

		os.clear();
		c = 0;
		BOOST_FOREACH (SortEl se, sorted){
			os.insert(SetEl(se.elem, c++));
		}
	}
public:
	OrderedSet() : c(0){
	}
	
	inline void insert(T el){
		if (c >= 10000000){
			renumber();
		}
		tmpEl.element = el;
		tmpEl.index = c++;
		os.insert(tmpEl);
	}

	inline void erase(T el){
		tmpEl.element = el;
		os.erase(tmpEl);
	}

	inline long getInsertionIndex(T el){
		tmpEl.element = el;
		if (os.count(tmpEl) > 0){
			return (os.find(tmpEl))->index;
		}else{
			return -1;
		}
	}

	inline int compare(T el1, T el2){
		if (getInsertionIndex(el1) < getInsertionIndex(el2)) return -1;
		else if (getInsertionIndex(el1) > getInsertionIndex(el2)) return +1;
		else return 0;
	}
*/
};

// compatibility with BOOST_FOREACH
namespace boost{
	template<typename T>
	struct range_mutable_iterator< Set<T> >{
		typedef set_iterator<T> type;
	};

	template<typename T>
	struct range_const_iterator< Set<T> >{
		typedef const_set_iterator<T> type;
	};
}

#endif

