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


/** @file BinaryTree.h
 *
 */

#ifndef BINARYTREE_H
#define BINARYTREE_H

#ifdef UNIT_TEST

typedef unsigned char	uchar;
typedef unsigned short	ushort;
typedef unsigned int	uint;
typedef unsigned long	ulong;

#include "comparitors.h"

#else

#include <constants.h>
#include <types.h>
#include <comparitors.h>

#endif

/*
This is a simple binary tree... to be replaced by a blanced binary tree
*/
template<typename T, typename Comparitor = k_std::less_then<T> >
class BinaryTree
{
public:
	class Node
	{
	public:
		Node()
		{ parent = rightChild = leftChild = NULL; }
		~Node()
		{
			if(rightChild != NULL) delete rightChild;
			if(leftChild != NULL) delete leftChild;
		}
		
		T	value;
		Node	*parent;
		Node	*rightChild;
		Node	*leftChild;
	};
	
	class Iterator
	{
	public:
		Iterator()
		{ cur = last = NULL; }
		
		Iterator(const Iterator &i)
			: cur(i.cur), last(i.last)
		{ ; }
		
		Iterator(Node *c, Node *l)
			: cur(c), last(l)
		{ ; }
		
		Iterator &operator++()	// prefix
		{
			if(cur->rightChild == NULL && cur->leftChild == NULL)
			{
				Node	*tmp = last;
				last = cur;
				cur = tmp;
			}
			
			else if(cur->leftChild != NULL && cur->leftChild != last)
			{ last = cur; cur = cur->leftChild; }
			
			else if(cur->leftChild == last && cur->rightChild != NULL)
			{ last = cur; cur = cur->rightChild; }
			
			return(*this);
		}
		
		Iterator& operator++(int)	// postfix
		{
			Iterator tmp(*this);
			++(*this);
			return (tmp);
		}

		Iterator &operator--()	// prefix
		{
			return(*this);
		}
		
		Iterator& operator--(int)	// postfix
		{
			Iterator tmp(*this);
			++(*this);
			return (tmp);
		}

	private:
		Node	*cur;
		Node	*last;
	};
	
	BinaryTree();	// default constructor
	BinaryTree(const T &t);
	BinaryTree(const BinaryTree &bt);	// copy constructor
	~BinaryTree();	// destructor
	
	Node *Insert(const T &t);
	void Erase(BinaryTree::Node *pos);
	Node *Find(const T &t);
	
	Iterator begin()
	{
		Node *last, *cur = root;
		
		while(cur != NULL)
		{
			last = cur;
			cur = cur->leftChild;
		}
		
		return(Iterator(cur, last));
	}
	
	Iterator end()
	{
		Node *last, *cur = root;
		
		while(cur != NULL)
		{
			last = cur;
			cur = cur->rightChild;
		}
		
		return(Iterator(cur, last));
	}

	inline void Clear()
	{
		if(root != NULL)
			delete root;
		
		root = NULL;
		nodeCount = 0;
	}
	
	void WalkTree()
	{
		WalkTree(root);
	}
	
	void WalkTree(Node *p)
	{
		if(p == NULL)
			return;
		
		WalkTree(p->leftChild);
		cout << p->value << endl;
		WalkTree(p->rightChild);
	}
	
	BinaryTree &operator=(const BinaryTree &bt);	// assignment operator
	inline uint size()
	{ return nodeCount; }
	
private:
	Node	*root;
	uint	nodeCount;
};


template<typename T, typename Comparitor>
BinaryTree<T, Comparitor>::BinaryTree()
{
	root = NULL;
	nodeCount = 0;
}

template<typename T, typename Comparitor>
BinaryTree<T, Comparitor>::BinaryTree(const T &t)
{
	root = new Node;
	root->value = t;
	nodeCount = 0;
}

template<typename T, typename Comparitor>
BinaryTree<T, Comparitor>::BinaryTree(const BinaryTree &bt)
{
	*this = bt;
}

template<typename T, typename Comparitor>
BinaryTree<T, Comparitor>::~BinaryTree()
{
	if(root != NULL)
		delete root;
}

template<typename T, typename Comparitor>
BinaryTree<T, Comparitor> &BinaryTree<T, Comparitor>::operator=(const BinaryTree<T, Comparitor> &bt)
{
	Clear();
	 			
	// need to fill out this tree...
}

template<typename T, typename Comparitor>
typename BinaryTree<T, Comparitor>::Node *BinaryTree<T, Comparitor>::Insert(const T &t)
{
	// empty tree
	if(root == NULL)
	{
		root = new Node;
		root->value = t;
		nodeCount = 1;
		return root;
	}
	
	// walk through the tree looking for a home for t
	Node	*cur = root, *last = NULL;
	
	while(cur != NULL)
	{
		last = cur;
		
		// *cur < t, move to the right
		if(Comparitor()(cur->value, t))
			cur = cur->rightChild;
		
		// *cur > t, move to the left
		else if(Comparitor()(t, cur->value))
			cur = cur->leftChild;
		
		else
			return(NULL);
	}
	
	// we found a home for t
	// make a new node, and link in
	cur = new Node;
	cur->parent = last;
	cur->value = t;
	
	if(last != NULL)
	{
		if(Comparitor()(last->value, t))	// went to the right
			last->rightChild = cur;
		else
			last->leftChild = cur;
	}
	
	++nodeCount;
	
	return(cur);
}

template<typename T, typename Comparitor>
void BinaryTree<T, Comparitor>::Erase(BinaryTree<T, Comparitor>::Node *pos)
{
	if(root == NULL || pos == NULL)
		return;
	
	// check if we're the only node
	if(nodeCount == 1 && pos == root)
	{
		Clear();
		return;
	}
	
	// check if it's a leaf
	if(pos->rightChild == NULL && pos->leftChild == NULL)
	{
		// find out if we're a right or left, setup the parent accordingly
		if(pos->parent->rightChild == pos)
			pos->parent->rightChild = NULL;
		else
			pos->parent->leftChild = NULL;
	}
	
	// if we have no rightChild then just have leftChild take my place
	else if(pos->rightChild == NULL)
	{
		pos->leftChild->parent = pos->parent;
		
		if(pos->parent != NULL)
		{
			if(pos->parent->rightChild == pos)
				pos->parent->rightChild = pos->leftChild;
			else
				pos->parent->leftChild = pos->leftChild;
		}
		
		else
		{
			root = pos->leftChild;
			
			pos->leftChild = NULL;
		}
	}
	
	// if we have no leftChild then just have rightChild take my place
	else if(pos->leftChild == NULL)
	{
		pos->rightChild->parent = pos->parent;
		
		if(pos->parent != NULL)
		{
			if(pos->parent->rightChild == pos)
				pos->parent->rightChild = pos->rightChild;
			else
				pos->parent->leftChild = pos->rightChild;
		}
		
		else
		{
			root = pos->rightChild;
			
			pos->rightChild = NULL;
		}
	}

	// we have both of our children, so just pick the right most of the left child
	else
	{
		Node *last, *cur = pos->leftChild;
		
		while(cur != NULL)
		{
			last = cur;
			cur = cur->rightChild;
		}
		
		if(last->parent != pos)
		{
			last->parent->rightChild = NULL;	// the right link is now gone
			last->leftChild = pos->leftChild;
		}
		
		last->parent = pos->parent;
		last->rightChild = pos->rightChild;
		
		pos->rightChild->parent = last;
		pos->leftChild->parent = last;
		
		if(pos->parent != NULL)
		{
			if(pos->parent->rightChild == pos)
				pos->parent->rightChild = last;
			else
				pos->parent->leftChild = last;
		}
		
		if(root == pos)
			root = last;
	}
	
	--nodeCount;
	
	pos->rightChild = NULL;
	pos->leftChild = NULL;
	
	delete pos;
}

template<typename T, typename Comparitor>
typename BinaryTree<T, Comparitor>::Node *BinaryTree<T, Comparitor>::Find(const T &t)
{
	Node	*cur = root;
	
	if(root == NULL)
		return NULL;
	
	// while *cur != t
	while(cur != NULL && (Comparitor()(cur->value, t) || Comparitor()(t, cur->value)))
	{
		// *cur < t
		if(Comparitor()(cur->value, t))
			cur = cur->rightChild;
		
		// *cur > t
		else if(Comparitor()(t, cur->value))
			cur = cur->leftChild;
		
		else
			return(NULL);
	}
	
	return cur;
}


#endif // BINARYTREE_H


