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

/**@file k_vector.h
 * This file contains the specialization of vector for bool.
 */

#ifdef _K_VECTOR_H		// only want this if not already included in k_vector.h

#define BASESIZE	(sizeof(ulong) * 8)

// helper functions
inline unsigned int BitsToBlocks(const ulong &numOfBits)
{ return(numOfBits%BASESIZE == 0 ? numOfBits/BASESIZE : (numOfBits/BASESIZE)+1); }

inline unsigned int BitsToBytes(const ulong &numOfBits)
{ return(BitsToBlocks(numOfBits)*4); }

inline ulong BitToMask(const ulong &index)
{ return(0x1 << (index%BASESIZE)); }

/**@class BitReference
 * @brief The BitReference class is needed for things like a[1] = 1; when dealing with k_vector<bool>.
 */
class BitReference
{
public:
	BitReference(const ulong *theBlock, const uint &pos) 
		: pBlock(const_cast<ulong*>(theBlock)), theBit(pos)
	{ ; }
	
	BitReference(const BitReference &right)
	{ *this = right; }

	~BitReference() {}	// destructor

	bool operator=(const bool &right) // assignment for a[i] = i;
	{
		if(right)	{ *pBlock |= BitToMask(theBit); } 	// set to 1
		else 		{ *pBlock &= ~BitToMask(theBit); }	// set to 0
		
		return(right);
	}

	BitReference &operator=(const BitReference &right) // assignment for a[i] = b[i];
	{
		if(*(right.pBlock) & BitToMask(right.theBit))	{ *pBlock |= BitToMask(theBit); } // set to 1
		else 						{ *pBlock &= ~BitToMask(theBit); }// set to 0
		
		return(*this);
	}

	bool operator==(const bool &right)
	{ return((*pBlock & BitToMask(theBit)) > 0 ? true : false == right); }

	operator bool() const
	{ return(*pBlock & (BitToMask(theBit)) ? true : false);	}

private:
	ulong	*pBlock;
	ulong	theBit;	// should range from 0 to 32
};

/**@class k_vector<bool>
 * @brief Specialization of the k_vector class.
 */
template <>
class vector <bool>
{
public:
	class iterator;			// prototype
	friend class iterator;
	friend class BitReference;	// make BitReference a friend class

	// default constructor
	vector() : blocks(NULL), numOfBits(0), numOfBlocks(0)
	{ ; }
	
	// create n objects
	explicit vector(uint n)
	{
		numOfBlocks = BitsToBlocks(n);
		blocks = new ulong[numOfBlocks];	// don't call reserve because blocks != NULL right now
		numOfBits = n;
	
		for(ulong i=0; i < numOfBlocks; ++i)
			blocks[i] = 0;	// set to all zero
	}
	
	// make n copies of t
	explicit vector(uint n, const bool &t)
	{
		numOfBlocks = BitsToBlocks(n);
		blocks = new ulong[numOfBlocks];	// don't call reserve because blocks != NULL right now
		numOfBits = n;
		
		for(ulong i=0; i < numOfBlocks; ++i)
			blocks[i] = t ? 0xFFFFFFFF : 0;
	}
		
	// copy constructor
	vector(const vector<bool> &arg)
		: blocks(new ulong[arg.numOfBlocks]), numOfBits(arg.numOfBits), numOfBlocks(arg.numOfBlocks)
	{
		memcpy(blocks, arg.blocks, sizeof(ulong) * numOfBlocks);
	}
			
	// copy a range of another list
	vector(const iterator &f, const iterator &l)
	{
		numOfBits = l - f;
		numOfBlocks = BitsToBlocks(numOfBits);
		blocks = new ulong[numOfBlocks];
		
		for(iterator it = f, mine = begin(); it != l; ++it, ++mine)
			*mine = *it;
	}

	// create a bit vector from a pointer
	vector(const ulong *ptr, const ulong bits)
	{
		reserve(bits);	// make the memory
		
		// could change this to a memcopy
		for(uint i=0; i < BitsToBlocks(bits); ++i)
			blocks[i] = ptr[i];
			
	}
	
	// destructor
	~vector()
	{
		if(blocks != NULL)
			delete [] blocks;
	}

	void push_back(const bool &arg)
	{ insert(end(), arg); }
	
	void push_front(const bool &arg)
	{ insert(begin(), arg); }
	
	void pop_back()
	{ erase(--end()); }
	
	void pop_front()
	{ erase(begin()); }
	
	iterator insert(iterator pos, const bool arg)
	{
		if(numOfBits == 0)	// have no memory
		{
			reserve(1);
			pos = begin();
		}
		
		else if(numOfBlocks*BASESIZE == numOfBits)	// at capacity
		{
			
			uint 	offset = pos - begin();
			
			reserve(numOfBits + 1);
			
			pos = begin() + offset;
		}
		
		iterator	it = end();
		iterator	it2 = it; --it2;	// set to the one before
		iterator	tmp = begin();
		
		for( ; it != pos; --it, --it2)
			*it = *it2;
		
		*pos = arg;

		numOfBits++;
		return(pos);
	}
	
	void insert(iterator pos, const uint n, const bool arg)
	{
		uint	offset = pos - begin();
		
		reserve(numOfBits + n);	// reserve all the memory we need
		
		// reset the pos in case we made some memory
		pos = begin() + offset;
	
		for(uint i=0; i < n; ++i)
			pos = insert(pos, arg);
	}
			
	void insert(iterator pos, iterator f, iterator l)
	{
		uint	offset = pos - begin();
		
		reserve(numOfBits + (l - f));	// reserve all the memory we need
		
		// reset the pos in case we made some memory
		pos = begin() + offset;
		
		for( ; f != l; ++f)
			pos = insert(pos, *f);
	}
			
	iterator erase(iterator pos)
	{ return(erase(pos, pos+1)); }

	iterator erase(iterator f, iterator l)
	{
		if(numOfBits == 0)
			return(end());
		
		iterator tmp = f;	// save this for the return value

		if(l != end())
			for(iterator it = l; it != end(); ++it, ++f)
				*f = *it;

		numOfBits -= (l - f);	// just update the size

		return(tmp);
	}

	void clear(void);
	
	inline uint size(void) const
	{ return numOfBits; }
	
	inline bool empty(void) const
	{ return(numOfBits == 0); }
	
	void reserve(uint n)
	{
		if(BitsToBlocks(n) <= numOfBlocks)	// we have enough memory
			return;	// nothing to do
	
		ulong	*tmp = new ulong[BitsToBlocks(n)];	// make new memory
	
		if(blocks == NULL)
			*tmp = 0;
		else
			memcpy(tmp, blocks, sizeof(ulong) * numOfBlocks);
		
		numOfBlocks = BitsToBlocks(n);		// set the number of blocks we have now
	
		if(blocks != NULL)
			delete [] blocks;
	
		blocks = tmp;	// swap the pointers
	}

	// array notation for testing
	bool operator[](const uint item) const
	{
		if(BitReference(blocks + (BitsToBlocks(item)-(item%BASESIZE == 0 ? 0 : 1)), item%BASESIZE))
			return true;
		else
			return false;
	}

	// array notation for assignment
	BitReference operator[](const uint item)
	{
		return BitReference(blocks + (BitsToBlocks(item)-(item%BASESIZE == 0 ? 0 : 1)), item%BASESIZE);
	}

	// swap right & left
	void swap(iterator r, iterator l)
	{
		bool	t = *r;		// set tmp = r
		*r = *l;	// set r = l
		*l = t;		// set l = tmp
	}

	// these aren't defined by STL, but easy to code and useful
//	uint numSet();
//	uint numClear();

	/**@class vector<bool>::iterator
	 * An iterator implementation for the vector<bool> class
	 */
	class iterator
	{
	public:
		friend class vector<bool>;

		iterator() : blockPtr(NULL), curBit(0)
		{ ; }
		
		iterator(const iterator &it) : blockPtr(it.blockPtr), curBit(it.curBit)
		{ ; }
		
		explicit iterator(const ulong *ptr, const ulong bit) : blockPtr(const_cast<ulong*>(ptr)), curBit(bit)
		{ ; }
		
		BitReference operator*() const 
		{ return(BitReference(blockPtr, curBit)); }	// return reference to the object

		iterator operator++()		// preincrement
		{
			if(curBit + 1 == 32)
			{
				++blockPtr;
				curBit = -1;
			}
			curBit += 1;
			
			return(*this);
		}		
		
		iterator operator++(int)	// postincrement
		{ iterator tmp(*this); ++*this; return(tmp); }
		
		iterator& operator--()		//predecriment
		{
			if(curBit - 1 == -1)
			{
				--blockPtr;
				curBit = BASESIZE;
			}
			curBit -= 1;
			return(*this);
		}
		
		iterator operator--(int)	// postdecriment
		{ iterator tmp(*this); --*this; return(tmp); }

		iterator operator+(uint amt) const	// addition
		{
			ulong	*newPtr = blockPtr + BitsToBlocks(curBit+amt) - ((curBit+amt)%BASESIZE == 0 ? 0 : 1);
			uint	 newBit = (curBit + amt)%BASESIZE;

			if(newBit > BASESIZE)
			{
				++newPtr;
				newBit %= BASESIZE;
			}

			return iterator(newPtr, newBit);
		}

		iterator operator-(uint amt)	// subtraction
		{
			ulong	*newPtr = blockPtr - int(amt/BASESIZE);
			int	 newBit = curBit - amt%BASESIZE;

			if(newBit < 0)
			{
				--newPtr;
				newBit += BASESIZE;
			}

			return iterator(newPtr, newBit);
		}

		bool operator==(const iterator &right) const
		{ return(blockPtr == right.blockPtr && curBit == right.curBit); }
		
		bool operator!=(const iterator &right) const
		{ return(blockPtr != right.blockPtr || curBit != right.curBit); }

		bool operator<(const iterator &right) const
		{ return(blockPtr < right.blockPtr && curBit < right.curBit); }

		bool operator>(const iterator &right) const
		{ return(blockPtr > right.blockPtr && curBit > right.curBit); }
		
		ulong operator-(const iterator right) const
		{
			if(blockPtr - right.blockPtr == 0)	// just need the bits
				return(curBit - right.curBit);
			else
				return( (BASESIZE * ((blockPtr - right.blockPtr)-1)) + (BASESIZE - right.curBit) + curBit );
		}

	protected:
		ulong	*blockPtr;
		int	curBit;		// this should range from 0 to 31, NEVER 32!
	};

	iterator begin()	{ return(iterator(blocks, 0)); }
	iterator end()
	{
		if(numOfBits == 0)
			return(iterator(blocks, 0));
		
		if(numOfBits%BASESIZE == 0)
			return(iterator(blocks + BitsToBlocks(numOfBits), 0));		
		
		// we want 1 beyond the end
		return(iterator(blocks + (BitsToBlocks(numOfBits) == 0 ? 0 : BitsToBlocks(numOfBits)-1), numOfBits%BASESIZE));
	}

private:
	ulong	*blocks;	// an array of bitbase_t that holds the actual data
	ulong	numOfBits;	// the number of bits used in that array, can calculate how many bitbase_t from this
	ulong	numOfBlocks;	// this is the number of blocks allocated already
};


#endif 
