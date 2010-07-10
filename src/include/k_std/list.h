#ifndef _K_LIST_H
#define _K_LIST_H

#include <types.h>
#include <constants.h>
#include <comparitors.h>

#ifdef UNIT_TEST
#include <stdio.h>
#endif

// define the namespace
namespace k_std
{

// this is the typename for the objects in the list
template<typename T>
class list
{
protected:
	struct ListObj;
public:

	// prototypes
	class iterator;
	friend class iterator;

	list();					// default constructor
	explicit list(uint n);			// make n objects of T()
	explicit list(uint n, const T &t);		// make n copies of t
	list(const list<T> &arg);			// copy constructor
	list(const iterator &f, const iterator &l);	// copy a range of another list
	~list();	// destructor
	
	void push_back(const T &arg)  { insert(end(), arg); }
	void push_front(const T &arg) { insert(begin(), arg); }
	void pop_back()  { erase(--end()); }
	void pop_front() { erase(begin()); }
	iterator insert(iterator pos, const T &arg);
	void insert(iterator pos, const uint n, const T &arg);
	void insert(iterator pos, const iterator f, const iterator l);
	iterator erase(iterator pos);
	void erase(iterator f, iterator l);
	T &front()	{ return(head->obj); }
	T &back()	{ return(tail->prev->obj); }
	
	void clear(void);
	inline uint size(void) const { return lSize; }
	inline bool empty(void) const { return(lSize == 0); }

	list<T> &operator=(const list<T> &right);

	void swap(iterator r, iterator l);	// swap right & left
	void sort(void);
	template<typename Comparitor>
	void sort(Comparitor cmp);

	void DebugPrint()
	{
		ListObj	*tmp = head;
		
		printf("HEAD: 0x%x  TAIL: 0x%x\n", head, tail);

		for( ; tmp != tail; tmp = tmp->next)
			printf("0x%x <- 0x%x -> 0x%x\n", tmp->prev, tmp, tmp->next);
		printf("0x%x <- TAIL: 0x%x -> 0x%x\n", tail->prev, tail, tail->next);
	}

	//
	// Iterator
	//
	class iterator
	{
	public:
		friend class list;
		iterator() : ptr(NULL) { ; }	// constructor
		iterator(const iterator &it) : ptr(it.ptr) { ; }
		T &operator*() const { return(ptr->obj); }	// return reference to the object
		
		iterator& operator++()// preincrement
		{ ptr = ptr->next; return(*this); }		
		
		iterator operator++(int)	// postincrement
		{ iterator tmp(*this); ++*this; return(tmp); }
		
		iterator& operator--()	//predecriment
		{ ptr = ptr->prev; return(*this); }
		
		iterator operator--(int)	// postdecriment
		{ iterator tmp(*this); --*this; return(tmp); }

		bool operator==(const iterator &right) const
		{ return(ptr == right.ptr); }
		
		bool operator!=(const iterator &right) const
		{ return(ptr != right.ptr); }

		iterator &operator=(const iterator &right)
		{ ptr = right.ptr; return(*this); }

	protected:
		iterator(ListObj* p) : ptr(p) { ; }
		ListObj*	ptr;
	};

	iterator begin() const	{ return(iterator(head)); }
	iterator end() const	{ return(iterator(tail)); }	// specific value for the end of the list

protected:
	struct	ListObj
	{
		ListObj	*next;
		ListObj	*prev;
		T	obj;
	};

	template<typename Comparitor>
	void InternalSort(ListObj **h, ListObj **t, uint s, Comparitor cmp);	// used internally to help the sort
	void SetupBlankList();	// just like the default constructor, but can be called by others

private:
	ListObj		*head;	// this is the head of our list
	ListObj		*tail;	// this is the tail of our list
	uint		lSize;
	
};


//
// Constructors
//
template<typename T>
list<T>::list() 	// default constructor
{
	SetupBlankList();
}

template<typename T>
void list<T>::SetupBlankList()
{
	lSize = 0;
	tail = new ListObj;
	tail->next = NULL;
	tail->prev = NULL;	
	head = tail;
}

template<typename T>
list<T>::list(uint n) // make n objects of T()
{
	SetupBlankList();
	for(uint i=0; i < n; i++)
		push_back(T());	// make the items
	lSize = n;			// set the size
}

template<typename T>
list<T>::list(uint n, const T &t) // make n copies of t
{
	SetupBlankList();
	for(uint i=0; i < n; i++)
		push_back(t);	// copy the item on the list
	lSize = n;			// set the size
}

template<typename T>
list<T>::list(const list<T> &arg) // copy constructor
{
	SetupBlankList();
	insert(begin(), arg.begin(), arg.end());
}

template<typename T>
list<T>::list(const iterator &f, const iterator &l) // copy a range of another list
{
	SetupBlankList();
	lSize = 0;
	for(iterator it = f; it != l; ++it)
	{
		push_back(*it);	// copy over the items in the range
        	lSize++;
	}
}

template<typename T>
list<T>::~list()
{
	ListObj	*tmp = head->next;

//	DebugPrint();

	if(head == tail)	// list is empty
	{
		delete head;
		return;
	}
	
	else if(tmp == tail)	// list is only 2 long
	{
		delete head;
		delete tail;
		return;
	}

	else
	{
		while(tmp != tail)	// go through and delete everyone
		{
			delete tmp->prev;
			tmp = tmp->next;
		}
		
		delete tmp->prev;
		
		delete tail;	// delete the last guy
	}
}

template<typename T>	// insert before pos
typename list<T>::iterator list<T>::insert(iterator pos, const T &arg)
{
	ListObj		*tmp = new ListObj;
	
	tmp->obj = arg;	// problems with complex types if we don't call operator=

//	memcpy(&tmp->obj, &arg, sizeof(T));

	if(head == tail)	// empty list
	{
		head = tail->prev = tmp;	// update head & tail
		tmp->next = tail;		// setup tmp
		tmp->prev = NULL;
		pos = tail;
	}

	else if(pos == begin())	// insert at head
	{
		head = tmp;		// update head
		tmp->prev = NULL;	// setup tmp
		tmp->next = pos.ptr;
		pos.ptr->prev = tmp;	// link in tmp
	}

	else
	{
		tmp->next = pos.ptr;		// setup object's links
		tmp->prev = pos.ptr->prev;

		pos.ptr->prev->next = tmp;	// link in the object
		pos.ptr->prev = tmp;
	}

	lSize++;	// update the size

	return(pos);
}

template<typename T>
void list<T>::insert(iterator pos, const uint n, const T &arg)
{
	for(uint i=0; i < n; i++)
		pos = insert(pos, arg);
}

template<typename T>
void list<T>::insert(iterator pos, const iterator f, const iterator l)
{
	for(iterator it=f; it != l; ++it)
		pos = insert(pos, *it);
}

template<typename T>
typename list<T>::iterator list<T>::erase(iterator pos)
{
	iterator	newPos = pos;
	newPos++;

	if(head == tail)	// empty list
		return(head);			// not a good way to deal with this, but OK
	
	if(pos == begin())	// delete the head element
	{
		head = head->next;	// update the head
		head->prev = NULL;

		delete pos.ptr;	// delete the object
		lSize--;
	}

	else if(pos == end())	// delete the tail element
		return(tail->prev);	// not a good way to deal with this

	else	// some place in the middle
	{
		pos.ptr->next->prev = pos.ptr->prev;	// link around the object
		pos.ptr->prev->next = pos.ptr->next;
		
		delete pos.ptr;	// delete the object
		lSize--;
	}

	return(newPos);
}

template<typename T>
void list<T>::erase(iterator f, iterator l)
{
	if(f == begin() && l == end())	// want the whole list
	{
		clear();	// much faster
		f = l = end();
	}
	
	else	// call it one at a time
	{
		for(iterator it=f; it != l; )
			it = erase(it);
	}
}

template<typename T>
void list<T>::clear(void)
{
	if(lSize == 0) return;

	ListObj	*tmp = head->next;

	while(tmp != tail)	// go through and delete everyone
	{
		delete tmp->prev;
		tmp = tmp->next;
	}

	delete tail;	// delete the last guy

	SetupBlankList();	// update to fresh
}

template<typename T>
list<T> &list<T>::operator=(const list<T> &right)
{
	clear();	// clear out anything we might already have

	insert(begin(), right.begin(), right.end());

	return(*this);
}


template<typename T>
void list<T>::swap(iterator r, iterator l)
{
	if(r == l) return;

	T	tmp;

	//
	// This is certainly NOT the fastest way to swap... but certainly the easiest
	// This has an impact on the speed of sort
	// might consider chaning ListObj's obj to a T* to make this work faster
	//
	if(r == end())
	{
		tmp = tail->obj;
		tail->obj = l.ptr->obj;
		l.ptr->obj = tmp;
	}

	else if(l == end())
	{
		tmp = tail->obj;
		tail->obj = r.ptr->obj;
		r.ptr->obj = tmp;
	}

	else
	{
		tmp = r.ptr->obj;
		r.ptr->obj = l.ptr->obj;
		l.ptr->obj = tmp;
	}
}

template<typename T>
void list<T>::sort()	// perform merge sort
{
	// simple cases first
	if(lSize < 2) return;

	this->sort(less_then<T>());
}

template<typename T>
template<typename Comparitor>
void list<T>::sort(Comparitor cmp)
{
	// simple case first
	if(lSize < 2) return;

	ListObj	*tmp = tail->prev;
	tmp->next = NULL;

	// call the internal sort function
	this->InternalSort(&head, &tmp, lSize, cmp);

	tmp->next = tail;
	tail->prev = tmp;
}


template<typename T>
template<typename Comparitor>
void list<T>::InternalSort(ListObj **h, ListObj **t, uint s, Comparitor cmp)
{
	// base cases
	if(s < 2)	return;
	if(s == 2)
	{
		if(cmp((*t)->obj, (*h)->obj))	// swap these
			swap(iterator(*h), iterator(*t));
		return;
	}

	ListObj	*t1, *h2;
	ListObj *tmp = *h;	// set tmp to the head of the list

	// split the list into 2 pieces
	for(uint i=1; i < s/2; ++i)
		tmp = tmp->next;

	t1 = tmp;			// make a new end
	h2 = tmp->next;			// make a new start

	t1->next = NULL;		// unlink the two lists
	h2->prev = NULL;

	// make the recursive call on each list
	this->InternalSort(h, &t1, s/2, cmp);
	this->InternalSort(&h2, t, s/2 + (s%2==0?0:1), cmp);

	// now we have sorted lists... merge them back together
	ListObj *h1 = (*h);

	// set the new head if we need to
	if(cmp(h1->obj, h2->obj))
		h1 = h1->next;
		// no need to chang the head *h = h1 already

	else
	{
		*h = h2;
		h2 = h2->next;
	}

	tmp = (*h);	// this is the current one in the merged list

	while(h1 != NULL && h2 != NULL)	// go through picking the smallest from each list
	{
		if(cmp(h1->obj,h2->obj))
		{
			tmp->next = h1;
			h1->prev = tmp;
			h1 = h1->next;
		}
		else
		{
			tmp->next = h2;
			h2->prev = tmp;
			h2 = h2->next;
		}
		tmp = tmp->next;
	}

	if(h1 == NULL)	// need to link in the rest of h2
	{
		tmp->next = h2;
		h2->prev = tmp;
//		while(tmp->next != NULL)
//			tmp = tmp->next;
//		(*t)->prev = tmp;
	}

	else	// need to link in the rest of h1
	{
		tmp->next = h1;
		h1->prev = tmp;
		*t = t1;	// update the tail
//		while(tmp->next != NULL)
//			tmp = tmp->next;
//		(*t)->prev = tmp;
	}
}

}
#endif
