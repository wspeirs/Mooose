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


/** @file string.h
 *	Contains an implementation of a string class.
 */

#ifndef _K_STRING_H
#define _K_STRING_H

#include <constants.h>
#include <types.h>

#ifdef UNIT_TEST
#include <stdio.h>
#endif

/** @namespace k_std
 * @brief The kernel standard template library namespace.
 */
namespace k_std
{

/** @class string
 *	An implementation of a string class.
 */
class string
{
public:
	// prototypes
	class iterator;
	friend class iterator;
	
	string()					// default constructor
	{
		buff = new char[1];
		buff[0] = '\0';	// need to set to null
		buffLength = 0;
		buffCapacity = 1;
	}

	string(const string &str)	// copy constructor
	{
		buff = new char[str.buffCapacity];	// make memory for the string
	
		// could change to memcopy
		for(uint i=0; i < str.buffCapacity; ++i)
			buff[i] = str.buff[i];

		buffLength = str.buffLength;
		buffCapacity = str.buffCapacity;
	}
	
	explicit string(const char* str)		// make a string from a char *
	{
		buffCapacity = 0;

		// first find the length (bad way to do this)
		for( ; str[buffCapacity] != '\0'; ++buffCapacity);
		++buffCapacity;

		buff = new char[buffCapacity];

		for(uint i=0; i < buffCapacity; ++i)
			buff[i] = str[i];

		buffLength = buffCapacity - 1;
	}

	explicit string(const char* str, uint n)	// make a string from a char * taking only n chars
	{
		buff = new char[n+1];

		for(uint i=0; i < n; ++i)
			buff[i] = str[i];
		buff[n] = '\0';

		buffLength = n;
		buffCapacity = n+1;
	}
	
	explicit string(uint n, char c)		// make n copies of c
	{
		buff = new char[n+1];

		for(uint i=0; i < n; ++i)
			buff[i] = c;
		buff[n] = '\0';

		buffLength = n;
		buffCapacity = n+1;
	}
	
	string(iterator first, iterator last)		// make a string from a range of characters
	{
		buffLength = uint(last.charPtr - first.charPtr);
		buffCapacity = buffLength + 1;

		buff = new char[buffLength+1];

		for(uint i=0; i < buffLength; ++i)
			buff[i] = *(first.charPtr + i);
		buff[buffLength] = '\0';
	}

	~string()
	{
		delete [] buff;
	}


	void push_back(const char &c)  { insert(end(), c); }
	void push_front(const char &c) { insert(begin(), c); }
	void pop_back()  { erase(--end()); }
	void pop_front() { erase(begin()); }
	
	iterator insert(iterator pos, const char &c)
	{
		// see if we need to make more memory
		if(buffCapacity == 0)
		{
			reserve(2);             // initially only make one
			pos = begin();          // update that this points to the start
		}
	
		// see if there is enough room even with the \0 at the end
		else if(buffLength+1 >= buffCapacity)
		{
			uint    offset = uint(pos.charPtr - buff);     // save this position
	
			reserve(uint(buffCapacity*2)+1);
			pos.charPtr = buff + offset;
		}
	
		for(char *i=buff+buffLength; i > pos.charPtr; --i)
			*i = *(i-1);
	
		*pos.charPtr = c;
	
		buffLength++;
		buff[buffLength] = '\0';	// set the NULL
	
		return(pos);
	}
	
	void insert(iterator pos, const uint n, const char &c)
	{
		// make sure we get all the memory up-front
		if(buffCapacity == 0)
		{
			reserve(n+1);
			pos = begin();
		}

		else if(buffLength + n > buffCapacity)
		{
			uint    offset = uint(pos.charPtr - buff);     // save this position
	
			reserve(uint(buffCapacity + n*2)+1);	// COULD BE DYNAMIC
			pos.charPtr = buff + offset;                     // reset this position
		}
	
		for(uint i=0; i < n; ++i)
		{
			pos = insert(pos, c);
			pos++;
		}
	}

	void insert(iterator pos, iterator f, iterator l)
	{
		if(l < f)
			return;
	
		uint    numToInsert = uint(l.charPtr - f.charPtr);
	
		// make memory if needed
		if(buffCapacity == 0)
		{
			reserve(numToInsert+1);
			pos = begin();
		}
	
		else if(buffLength + numToInsert > buffCapacity)
		{
			uint    offset = uint(pos.charPtr - buff);     // save this position
	
			reserve(uint((buffCapacity + numToInsert)*2)+1);
			pos.charPtr = buff + offset;                     // reset this position
		}
	
		for(iterator tmp = f; tmp != l; ++tmp)
		{
			pos = insert(pos, *tmp);
			pos++;
		}
	}
	
	iterator erase(iterator pos)
	{
		return(erase(pos, iterator(pos.charPtr+1)));     // just call the range with a single object
	}
	
	iterator erase(iterator f, iterator l)
	{
		if(buffLength == 0)
			return(end());
		
		if(f == begin() && l == end())
		{
			if(buff != NULL)
				delete [] buff;
	
			buff = NULL;
			buffLength = buffCapacity = 0;
	
			return(begin());        
		}
	
		iterator tmp = f;
	
	
		if(l != end())
			for(iterator it = l; it != end(); ++it, ++tmp)
				*tmp = *it;
	
		buffLength -= uint(l.charPtr - f.charPtr);   // update the size
		buff[buffLength] = '\0';	// update the NULL
	
		return(f);
	}
	
	void clear(void) { buffCapacity = 0; }
	inline uint size(void) const { return buffLength; }
	inline bool empty(void) const { return(buffLength == 0); }
	const char *c_str(void) { return buff; }
	
	void reserve(uint n) // all iterators are invalid after this call
	{
		if(n < buffCapacity)
			return;

		char	*tmp = new char[n];        // make n new ones

		for(uint i=0; i < buffLength; ++i)
			tmp[i] = buff[i];       // copy over
		tmp[buffLength] = '\0';

		delete [] buff;

		buff = tmp;
		buffCapacity = n;  // update the capcity
	}
	
	char &operator[](const uint n) { return(buff[n]); }
	
	string &operator=(const string &right)
	{
		delete [] buff;
		
		buff = new char[right.buffCapacity];	// make memory for the string
	
		// could change to memcopy
		for(uint i=0; i < right.buffCapacity; ++i)
			buff[i] = right.buff[i];

		buffLength = right.buffLength;
		buffCapacity = right.buffCapacity;

		return(*this);
	}

	string &operator=(char *right)
	{
		return(*this = string(right));
	}

	string &operator+=(string &right)
	{
		insert(end(), right.begin(), right.end());

		return(*this);
	}

	string &operator+=(char *str)
	{
		string tmp(str);
	
		insert(end(), tmp.begin(), tmp.end());
	
		return(*this);
	}

	string operator+(string &right)
	{
		string tmp(*this);

		tmp.insert(tmp.end(), right.begin(), right.end());

		return(tmp);
	}
	
	string operator+(char *str)
	{
		string tmp(*this);
		string tmp2(str);

		tmp.insert(tmp.end(), tmp2.begin(), tmp2.end());

		return(tmp);
	}

	bool operator==(const char *right) const
	{
		return(*this == string(right));
	}
	
	bool operator==(const string &right) const
	{
		if(buffLength != right.buffLength)
			return(false);
	
		for(unsigned int i=0; i < buffLength; ++i)
		{
			if(buff[i] != right.buff[i])
				return(false);
		}
		
		return(true);
	}
	
	bool operator!=(const string &right) const
	{
		if(*this == right)
			return(false);
		
		return(true);
	}
	
	bool operator<(const char *right) const
	{
		return(*this < string(right));
	}
	
	bool operator<(const string &right) const
	{
		int length = this->buffLength < right.buffLength ? this->buffLength : right.buffLength;
		
		for(int i=0; i < length; ++i)
		{
			if(this->buff[i] >= right.buff[i])
				return(false);
		}
		
		return(true);		
	}
	
	bool BeginsWith(const char *arg) const
	{
		return(this->BeginsWith(string(arg)));	
	}
	
	bool BeginsWith(const string &arg) const
	{
		if(buffLength < arg.buffLength)
			return(false);
		
		for(unsigned int i=0; i < arg.buffLength; ++i)
		{
			if(buff[i] != arg.buff[i])
				return(false);
		}
		
		return(true);
	}
	
	// helpful functions
	template<typename Container>
	void Tokenize(char token, Container &theContainer)
	{
		char	*ptr1 = buff, *ptr2 = buff;
	
		for(uint i=0; i < buffLength; ++i)
		{
			if(buff[i] == token)	// found the token
			{
				ptr2 = &buff[i];	// set the pointer
				
				if(ptr1 == ptr2)	// on that character
				{
					++ptr2;
					ptr1 = ptr2;
					continue;
				}
			
				iterator tmp(ptr1);	// not sure why I have to do this...
				iterator tmp2(ptr2);
				
				string	tmpString(tmp, tmp2); //(iterator(ptr1), iterator(ptr2));
			
				theContainer.push_back(tmpString);	// add to our container
			
				++ptr2;
				ptr1 = ptr2;
			}
		}
		
		if(ptr1 != &buff[buffLength])
		{
			// need to add the last item
			ptr2 = &buff[buffLength];
		
			iterator tmp(ptr1), tmp2(ptr2);
		
			string	tmpString(tmp, tmp2);
		
			theContainer.push_back(tmpString);
		}
	}
	
	/** @class string::iterator
	 * An iterator implementation for the string class
	 */
	class iterator
	{
	public:
		friend class string;
		iterator() { charPtr = NULL; }
		iterator(const iterator &it) { charPtr = it.charPtr; }
		explicit iterator(const char *ptr) { charPtr = const_cast<char*>(ptr); }
		char &operator*() const { return(*charPtr); }	// return reference to the character

		iterator operator++()		// preincrement
		{ ++charPtr; return(*this); }		
	
		iterator operator++(int)	// postincrement
		{ iterator tmp(*this); ++*this; return(tmp); }
	
		iterator operator--()		//predecriment
		{ --charPtr; return(*this); }
	
		iterator operator--(int)	// postdecriment
		{ iterator tmp(*this); --*this; return(tmp); }

		iterator operator+(uint amt) const	// addition
		{ return(iterator(charPtr + amt)); }

//		uint operator+(const iterator &right) const
//		{ return(charPtr + right.charPtr); }

		iterator operator-(uint amt) const	// subtraction
		{ return(iterator(charPtr - amt)); }

		uint operator-(const iterator &right) const
		{ return(uint(charPtr - right.charPtr)); }

		bool operator==(const iterator &right) const	// comparison
		{ return(charPtr == right.charPtr); }
	
		bool operator!=(const iterator &right) const
		{ return(charPtr != right.charPtr); }

		bool operator<(const iterator &right) const
		{ return(charPtr < right.charPtr); }

		bool operator>(const iterator &right) const
		{ return(charPtr > right.charPtr); }

	protected:
		char	*charPtr;
	};

	iterator begin() const	{ return iterator(buff); }
	iterator end() const	{ return iterator(buff + size()); }
	
private:
	char	*buff;
	ulong	buffLength;
	ulong	buffCapacity;
};

}

#endif
