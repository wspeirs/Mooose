/*
 * Copyright (c) 2005
 * William R. Speirs
 *
 * Permission to use, copy, distribute, or modify this software for
 * the purpose of education is herby granted without fee. Permission
 * to sell this software or its documentation is hereby denied without
 * first obtaining the written consent of the author. In all cases, the 
 * above copyright notice must appear and this permission notice must 
 * appear in the supporting documentation. William R. Speirs makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 */


/** @file map.h
 *
 */

#ifndef MAP_H
#define MAP_H

#include <constants.h>
#include <types.h>
#include <list.h>
#include <algorithms.h>

namespace k_std
{

/**
 * The pair class.
 */
template<typename T1, typename T2>
class pair
{
public:
	/**
	 * Default constructor.
	 */
	pair()
	{ ; }
	
	/**
	 * Creates a pair from two objects.
	 * @param t1 The first item in the pair.
	 * @param t2 The second item in the pair.
	 */
	pair(T1 t1, T2 t2) : first(t1), second(t2)
	{ ; }
	
	/**
	 * Copy constructor.
	 * @param p The pair to copy.
	 */
	pair(const pair &p) : first(p.first), second(p.second)
	{ ; }
	
	/**
	 * Assignment operator.
	 * @param p The pair to copy.
	 * @return The newly copied pair.
	 */
	pair &operator=(const pair &p)
	{
		first = p.first;
		second = p.second;
		
		return *this;
	}
			
	/**
	 * The equality operator.
	 * @param right The pair to test quality of.
	 * @return The equality.
	 */
	bool operator==(const pair &right)
	{ return first == right.first && second == right.second; }
	
	/**
	 * The less than operator.
	 * If first &lt right.first = true
	 * else if first &gt right.first = false
	 * else second &lt right.second
	 * @param right The pair to test.
	 * @return The result.
	 */
	bool operator<(const pair &right)
	{
		if(first < right.first) return true;
		else if(first > right.first) return false;
		
		return second < right.second;
	}
	
	
	T1	first;	///< The first item in the pair
	T2	second;	///< The second item in the pair
};

// for right now we just use a list... not O(lg n) but such is life
template<typename KEY_T, typename VALUE_T, typename Comparitor = k_std::less_then<KEY_T> >
class map
{
private:
	typedef k_std::list< pair<KEY_T, VALUE_T> >	contain_t;
	typedef typename contain_t::iterator		contain_it;
	
	struct InternalComparitor
	{
		InternalComparitor() { ; }
		InternalComparitor(const pair<KEY_T, VALUE_T> &arg) : item(arg) { ; }
		
		bool operator()(const pair<KEY_T, VALUE_T> &left, const pair<KEY_T, VALUE_T> &right)
		{ return Comparitor()(left.first, right.first);	}
		
		bool operator()(const pair<KEY_T, VALUE_T> &right)
		{ return !Comparitor()(item.first, right.first) && !Comparitor()(right.first, item.first); }
		
		pair<KEY_T, VALUE_T>	item;
	};
	
	struct InternalKeyComparitor
	{
		InternalKeyComparitor(const KEY_T &arg) : item(arg) { ; }
		
		bool operator()(const pair<KEY_T, VALUE_T> &right)
		{ return item == right.first; }
		
		KEY_T item;
	};
public:
	/**
	 * Default constructor
	 */
	map()
	{ ; }
	
	/**
	 * Take a range of elements and inserts them into the map
	 * @param first The first element to insert
	 * @param last The one beyond the last element to insert
	 */
	template<typename InputIterator>
	map(InputIterator first, InputIterator last)
	{ insert(first, last); }
	
	/**
	 * The copy constructor
	 * @param m The map to copy
	 */
	map(const map &m)
	{ *this = m; }
	
	/**
	 * The destructor
	 */
	~map()
	{ ; }
	
	/**
	 * The assignment operator
	 * @param m The map to copy
	 * @return A reference to the map
	 */
	map &operator=(const map &m)
	{
		container = m.container;
		iCmp = m.iCmp;
	}
	
	class iterator
	{
		friend class map;
	public:
		iterator() { ; }
		iterator(const contain_it &c) : cur(c) { ; }
		
		inline iterator operator++()	// prefix
		{ ++cur; return *this; }
		
		inline iterator operator++(int)	// postfix
		{ iterator tmp(*this); ++*this; return(tmp); }
		
		inline iterator operator--()	// prefix
		{ --cur; return *this; }
		
		inline iterator operator--(int)	// postfix
		{ iterator tmp(*this); --*this;	return(tmp); }
		
		inline bool operator!=(const iterator &right)
		{ return cur != right.cur; }
		
		inline bool operator==(const iterator &right)
		{ return cur == right.cur; }
		
		inline pair<KEY_T, VALUE_T> operator*()
		{ return(*cur); }
		
	private:
		contain_it	cur;
	};
	
	inline iterator begin()
	{ return iterator(container.begin()); }
	
	inline iterator end()
	{ return iterator(container.end()); }
	
	/**
	 * Inserts an element
	 * @param arg The element to insert
	 * @return An iterator that points to the element
	 */
	iterator insert(const pair<KEY_T, VALUE_T> &arg);

	/**
	 * Inserts a range of elements
	 * @param first The first element to insert
	 * @param last The one beyond the last element to insert
	 */
	template<typename InputIterator>
	void insert(InputIterator first, InputIterator last);

	/**
	 * Erases an element
	 * @param arg The element to erase
	 */
	void erase(const pair<KEY_T, VALUE_T> &arg);

	/**
	 * Erases an element
	 * @param pos An iterator to the element to erase
	 */
	void erase(iterator pos);

	/**
	 * Erases a range of elements
	 * @param first The first element to erase
	 * @param last The one beyond the last element to erase
	 */
	void erase(iterator first, iterator last);
	
	/**
	 * Finds an element in the map.
	 * @param key The key to search for
	 * @return An iterator to the pair
	 */
	iterator find(const KEY_T &key)
	{
		contain_it it = k_std::find_if(container.begin(), container.end(), InternalKeyComparitor(key));
	
		return iterator(it);
	}

	/**
	 * Returns the number of items in the map.
	 * @return Number of items in the map.
	 */
	int size()
	{ return container.size(); }
	
	/**
	 * Removes everything from the map.
	 */
	void clear()
	{ container.clear(); }


private:
	contain_t		container;
	InternalComparitor	iCmp;
};


template<typename KEY_T, typename VALUE_T, typename Comparitor>
typename map<KEY_T, VALUE_T, Comparitor>::iterator map<KEY_T, VALUE_T, Comparitor>::insert(const pair<KEY_T, VALUE_T> &arg)
{
	// see if it is already in the list
	iterator it = find(arg.first);
	
	// if it is, just return
	if(it != end())
		return it;
	
	iterator ret(container.insert(container.begin(), arg));
	container.sort(iCmp);
	
 	return ret;
}

template<typename KEY_T, typename VALUE_T, typename Comparitor>
template<typename InputIterator>
void map<KEY_T, VALUE_T, Comparitor>::insert(InputIterator first, InputIterator last)
{
	InputIterator	tmp = first;
	
	while(tmp != last)
		insert(*tmp++);
}

template<typename KEY_T, typename VALUE_T, typename Comparitor>
void map<KEY_T, VALUE_T, Comparitor>::erase(const pair<KEY_T, VALUE_T> &arg)
{
	contain_it it = k_std::find_if(container.begin(), container.end(), InternalComparitor(arg));
	
	container.erase(it);
}

template<typename KEY_T, typename VALUE_T, typename Comparitor>
void map<KEY_T, VALUE_T, Comparitor>::erase(iterator pos)
{
	container.erase(pos.cur);
}

template<typename KEY_T, typename VALUE_T, typename Comparitor>
void map<KEY_T, VALUE_T, Comparitor>::erase(iterator first, iterator last)
{
	iterator tmp = first;
	
	while(tmp != last)
		erase(tmp++);	
}


}

#endif // MAP_H


