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

/** @file vector.h
 * This file contains the vector implementation for the kernel.
 */

#ifndef _K_VECTOR_H
#define _K_VECTOR_H

#define DEFAULT_MULTIPLIER	2

#include <types.h>
#include <constants.h>
#include <comparitors.h>
#include <mem_utils.h>

#include <screen_utils.h>

/** @namespace k_std
 */
namespace k_std
{


/** @class vector
 * A templated class that is an implementation of the STL vector, built for the kernel.
 */
template<typename T>
class vector
{
public:
	// prototypes 
	class iterator;
	friend class iterator;

	// constructors
	vector();
	explicit vector(uint n);					// create n objects
	explicit vector(uint n, const T &t);				// make n copies of t
	vector(const vector<T> &arg);				// copy constructor
	vector(const iterator &f, const iterator &l);			// copy a range of another list
	~vector();	// destructor

	inline void set_multiplier(uint mul) { multiplier = mul; }	// GOT TO BE CAREFUL WITH THIS!!!

	void push_back(const T &arg)  { insert(end(), arg); }
	void push_front(const T &arg) { insert(begin(), arg); }
	void pop_back()  { erase(--end()); }
	void pop_front() { erase(begin()); }
	iterator insert(iterator pos, const T &arg);
	void insert(iterator pos, const uint n, const T &arg);
	void insert(iterator pos, iterator f, iterator l);
	iterator erase(iterator pos);
	iterator erase(iterator f, iterator l);
	
	void clear(void) { vSize = 0; }
	inline uint size(void) const { return vSize; }
	inline bool empty(void) const { return(vSize == 0); }
	void reserve(uint n);

	T &operator[](const uint n) { return(objs[n]); }
	vector<T> &operator=(const vector<T> &right);

	void swap(iterator r, iterator l);	// swap right & left
	void sort(void);
	template<typename Comparitor>
	void sort(Comparitor cmp);


	/** @class vector<T>::iterator
	 * An iterator implementation for the vector class
	 */
	class iterator
	{
	public:
		friend class vector;
		iterator() { objPtr = NULL; }
		iterator(const iterator &it) { objPtr = it.objPtr; }
		explicit iterator(const T *ptr) { objPtr = const_cast<T*>(ptr); }
		T &operator*() const { return(*objPtr); }	// return reference to the object

		iterator operator++()		// preincrement
		{ ++objPtr; return(*this); }		
		
		iterator operator++(int)	// postincrement
		{ iterator tmp(*this); ++*this; return(tmp); }
		
		iterator operator--()		//predecriment
		{ --objPtr; return(*this); }
		
		iterator operator--(int)	// postdecriment
		{ iterator tmp(*this); --*this; return(tmp); }

		iterator operator+(uint amt) const	// addition
		{ return(iterator(objPtr + amt)); }

		uint operator+(const iterator &right) const
		{ return(objPtr + right.objPtr); }

		iterator operator-(uint amt) const	// subtraction
		{ return(iterator(objPtr - amt)); }

		uint operator-(const iterator &right) const
		{ return(objPtr - right.objPtr); }

		bool operator==(const iterator &right) const	// comparison
		{ return(objPtr == right.objPtr); }
		
		bool operator!=(const iterator &right) const
		{ return(objPtr != right.objPtr); }

		bool operator<(const iterator &right) const
		{ return(objPtr < right.objPtr); }

		bool operator>(const iterator &right) const
		{ return(objPtr > right.objPtr); }

	protected:
		T	*objPtr;
	};

	iterator begin() const	{ return(iterator(objs)); }
	iterator end() const	{ return(iterator(objs + size())); }	// double check this

protected:
	template<typename Comparitor>
	iterator InternalMedian(iterator &l, iterator &r, Comparitor);
	template<typename Comparitor>
	void InternalQuickSort(iterator l, iterator r, Comparitor cmp);
	template<typename Comparitor>
	void InternalInsertionSort(iterator l, iterator r, Comparitor cmp);

private:
	T	*objs;
	uint	vSize, vCapacity;
	uint	multiplier;	// bochs was having a problem with this being a float :-(
};

template<typename T>
vector<T>::vector()
{
	vSize = vCapacity = 0;
	objs = NULL;
	multiplier = DEFAULT_MULTIPLIER;
}
 
template<typename T>
vector<T>::vector(uint n)
{
	objs = new T[n];	// create n objects
	vCapacity = n;		// update the capacity
	vSize = n;		// update the size
	multiplier = DEFAULT_MULTIPLIER;
}

template<typename T>
vector<T>::vector(uint n, const T &t)
{
	objs = new T[n];

	for(uint i=0; i < n; ++i)
		objs[i] = t;	// copy over the object to each one

	vCapacity = n;
	vSize = n;
	multiplier = DEFAULT_MULTIPLIER;
}

template<typename T>
vector<T>::vector(const vector<T> &arg)
{
	insert(begin(), arg.begin(), arg.end());
	multiplier = DEFAULT_MULTIPLIER;
	
/*	objs = new T[arg.vSize];	// make just enough objects
	
	for(uint i=0; i < arg.vSize; ++i)
		objs[i] = arg.objs[i];	// copy them all over

	vCapacity = arg.vSize;	// update the capacity
	vSize = arg.vSize;	// update the size
*/
}

template<typename T>
vector<T>::vector(const iterator &f, const iterator &l)
{
// 	objs = new T[l.objPtr - f.objPtr];	// SHOULD DO A CHECK HERE
	insert(begin(), f, l);
	multiplier = DEFAULT_MULTIPLIER;
}

template<typename T>
vector<T>::~vector()
{
	// delete the objects
	delete [] objs;
}

template<typename T>
typename vector<T>::iterator vector<T>::insert(iterator pos, const T &arg)	// insert before pos
{
	// see if we need to make more memory
	if(vCapacity == 0)
	{
		reserve(1);		// initially only make one
		pos = begin();		// update that this points to the start
	}

	else if(vSize == vCapacity)
	{
		uint	offset = uint(pos.objPtr - objs);	// save this position

		reserve(uint(vCapacity*multiplier));
		pos.objPtr = objs + offset;
	}

	for(T *i=objs+vSize; i != pos.objPtr; --i)	// could have size issues if we get 2^32 items in the list
		*i = *(i-1);
		
	*pos.objPtr = arg;
	
	vSize++;

	return(pos);
}

template<typename T>
void vector<T>::insert(iterator pos, const uint n, const T &arg)
{
	// make sure we get all the memory up-front
	if(vCapacity == 0)
	{
		reserve(n);
		pos = begin();
	}
	
	else if(vSize + n > vCapacity)
	{
		uint	offset = uint(pos.objPtr - objs);	// save this position
		
		reserve(uint(vCapacity + n*multiplier));
		pos.objPtr = objs + offset;			// reset this position
	}

	for(uint i=0; i < n; ++i)
		pos = insert(pos, arg);
}

template<typename T>
void vector<T>::insert(iterator pos, iterator f, iterator l)
{
	if(l < f)
		return;

	uint	numToInsert = uint(l.objPtr - f.objPtr);
	
	// make memory if needed
	if(vCapacity == 0)
	{
		reserve(numToInsert);
		pos = begin();
	}
	
	else if(vSize + numToInsert > vCapacity)
	{
		uint	offset = uint(pos.objPtr - objs);	// save this position
		
		reserve(uint((vCapacity + numToInsert)*multiplier));
		pos.objPtr = objs + offset;			// reset this position
	}

	for(iterator tmp = f; tmp != l; ++tmp)
		pos = insert(pos, *tmp);
}

template<typename T>
typename vector<T>::iterator vector<T>::erase(iterator pos)
{
	return(erase(pos, iterator(pos.objPtr+1)));	// just call the range with a single object
}

template<typename T>
typename vector<T>::iterator vector<T>::erase(iterator f, iterator l)
{
	// we're asked to erase all the items
	if(f == begin() && l == end())
	{
		delete [] objs;
		
		objs = NULL;
		vSize = vCapacity = 0;
		
		return(begin());	
	}
	
	iterator tmp = f;

	// erase the items by simply copying over them
	if(l != end())
		for(iterator it = l; it != end(); ++it, ++f)
			*f = *it;

	vSize -= l.objPtr - f.objPtr;	// just update the size

	return(tmp);
}

template<typename T>
void vector<T>::reserve(uint n)	// any time this is called all iterators & pointers are invalidated!!!
{
	if(n < vCapacity)
		return;

	T	*tmp = new T[n];	// make n new ones
	
	for(uint i=0; i < vSize; ++i)
		tmp[i] = objs[i];	// copy over
		
	if(vSize != 0)
		delete [] objs;

	objs = tmp;
	vCapacity = n;	// update the capcity
}

template<typename T>
vector<T> &vector<T>::operator=(const vector<T> &right)
{
	delete [] objs;
	
	vSize = vCapacity = 0;
	
	insert(begin(), right.begin(), right.end());
	
	return *this;
}

	
template<typename T>
void vector<T>::swap(iterator r, iterator l)
{
	T	tmp = *r.objPtr;	// set tmp = r
	*r.objPtr = *l.objPtr;	// set r = l
	*l.objPtr = tmp;		// set l = tmp
}

template<typename T>
void vector<T>::sort(void)
{
	InternalQuickSort(begin(), --end(), less_then<T>());
}

template<typename T>
template<typename Comparitor>
void vector<T>::sort(Comparitor cmp)	// quick sort
{
	InternalQuickSort(begin(), --end(), cmp);
}

template<typename T>
template<typename Comparitor>
typename vector<T>::iterator vector<T>::InternalMedian(iterator &l, iterator &r, Comparitor cmp)
{
	iterator c(l.objPtr + uint((r.objPtr - l.objPtr) / 2));
	
	if(cmp(*c, *l))
		swap(l, c);
	if(cmp(*r, *l))
		swap(l, r);
	if(cmp(*r, *c))
		swap(c, r);

	swap(c, r - 1);

	return (r - 1);
}

template<typename T>
template<typename Comparitor>
void vector<T>::InternalQuickSort(iterator l, iterator r, Comparitor cmp)
{
	if(r.objPtr - l.objPtr <= 5)
		InternalInsertionSort(l, r+1, cmp);

	else
	{
		iterator	pivot = InternalMedian(l, r, cmp);
		iterator	i = l;
		iterator	j = r - 1;

		while(1)
		{
			while(cmp(*(++i), *pivot)) ;
			while(cmp(*pivot, *(--j))) ;

			if(i < j)
				swap(i, j);
			else
				break;
		}

		swap(i, r - 1);

		InternalQuickSort(l, i-1, cmp);
		InternalQuickSort(i+1, r, cmp);
	}
}

template<typename T>
template<typename Comparitor>
void vector<T>::InternalInsertionSort(iterator l, iterator r, Comparitor cmp)
{
	iterator j;

	for(iterator p = l + 1; p != r; ++p)
	{
		T	tmp = *p;
		
		for(j = p; j != l && cmp(tmp, *(j.objPtr - 1)); --j)
			*j = *(j.objPtr - 1);

		*j = tmp;
	}
}


//
// This is for the specialization for vector<bool>
//

#include "bitvector.h"

}

#endif
