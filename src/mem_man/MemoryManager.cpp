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

/** @file MemoryManager.cpp
 *
 */
#include <types.h>
#include <constants.h>
#include <MemoryManager.h>

#ifdef UNIT_TEST

#include "harness_code.h"

#else

#include <screen_utils.h>
#include <AutoDisable.h>
#include <Debug.h>

#endif


//
// Static defs
//
MemManager::MemBlock*	MemManager::start;
MemManager*		MemManager::myself;
bool			MemManager::created = false;

//
// Overloading of new & delete for the kernel
//
void *operator new(size_t size)
{
	static MemManager	*heap = MemManager::GetInstancePtr();

	return heap->AllocateMemory(size);
}

void *operator new[](size_t size)
{
	static MemManager	*heap = MemManager::GetInstancePtr();

	return heap->AllocateMemory(size);
}

void operator delete(void *ptr)
{
	if(ptr == NULL)
		return;

	static MemManager	*heap = MemManager::GetInstancePtr();

	heap->FreeMemory(ptr);
}

void operator delete[](void *ptr)
{
	if(ptr == NULL)
		return;

	static MemManager	*heap = MemManager::GetInstancePtr();

	heap->FreeMemory(ptr);
}


//
// Constructors
//

// default constructor
MemManager::MemManager(void *startHeap)	
	: usedMemory(0), usedBlocks(0)
{
	if(created)
	{
		vga_printf("Tried to create a second MemManager\n");
		while(1);
	}
	
	// setup the singelton stuff
	created = true;
	myself = this;
	
	start = reinterpret_cast<MemBlock*>(RoundUpTo32Bits(reinterpret_cast<ulong*>(startHeap)));

	vga_printf("START HEAP: 0x%x\n", start);
	vga_printf("  END HEAP: 0x%x\n", reinterpret_cast<uchar*>(start) + HEAP_SIZE);
	vga_printf("      SIZE: 0x%x\n", HEAP_SIZE);

	MemBlock	*tmpBlk = reinterpret_cast<MemBlock*>(start);

	tmpBlk->magicWord = MAGIC_VALUE;
	tmpBlk->size = HEAP_SIZE - sizeof(MemBlock);
	tmpBlk->nextFree = NULL;
	
	freeList = tmpBlk;	// point to the start of the free list
}


void *MemManager::AllocateMemory(const ulong bytes)
{
	//
	// A first fit algorithm is used to allocate memory
	//

	AutoDisable	lock;
	MemBlock	*curBlk = freeList;	// get the start of the free list
	MemBlock	*prevBlk = freeList;	// trailing pointer

	ulong	sizeRequested = RoundUpTo32Bits(bytes);
	
// 	DEBUG("ASKED: %u GETTING: %u\n", bytes, sizeRequested);

	// make sure we have memory
	if(curBlk == NULL)
	{
		DEBUG("SIZE REQUESTED: %d\n", sizeRequested);

		WalkHeap(true);
					
		PANIC("Out of memory");
	}
	
	// make sure we have the magic word in our first block
	if(curBlk->magicWord != MAGIC_VALUE)
	{
		DEBUG("Memory used: %d\n", usedMemory);
		PANIC("MemManager: First memory block doesn't have proper magic word\n");
	}

	// search through for the first free block that fits the requirements
	// 1) Must be a free block (always will be because in the free list)
	// 2) Must be the exact same size OR big enough to break into 2 blocks
	while(curBlk->size != sizeRequested &&
	      curBlk->size < sizeRequested + sizeof(MemBlock) + 4)
	{
		// this block won't work, and there are no more free ones
		if(curBlk->nextFree == NULL)
		{
			DEBUG("SIZE REQUESTED: %d\n", sizeRequested);

			WalkHeap(true);
			
			PANIC("Out of memory");
		}

		// check to make sure this is a valid block
		if(!IsFreeBlock(*curBlk) || curBlk->magicWord != MAGIC_VALUE)
		{
			if(!IsFreeBlock(*curBlk))
				DEBUG("NOT FREE BLOCK: 0x%x\n", curBlk->size);
			if(curBlk->magicWord != MAGIC_VALUE)
				DEBUG("BAD MAGIC WORD: 0x%x\n", curBlk->magicWord);
			
			DEBUG("BLOCK: %x\n", (reinterpret_cast<char*>(curBlk)) + sizeof(MemBlock));
			PANIC("Corrupted Memory");
		}
		
		prevBlk = curBlk;			// update our pointers
		curBlk = curBlk->nextFree;
	}

// 	DEBUG("FOUND BLOCK AT: 0x%x\n", curBlk);

	// we are now pointing to a block that is free
	// it is either the exact size we need or big enough to break into 2 blocks
	if(curBlk->size == sizeRequested)
	{
		MarkAsUsed(*curBlk);	// mark this block as used

		if(prevBlk == curBlk)	// both point to the first block
			freeList = curBlk->nextFree;		// update the start of the free list
		else	
			prevBlk->nextFree = curBlk->nextFree;	// otherwise link into the list
	}

	else	// size is big enough to break into 2 blocks
	{
		// make a new memory block
		MemBlock *newBlk = reinterpret_cast<MemBlock*>(reinterpret_cast<uchar *>(curBlk) + sizeRequested + sizeof(MemBlock));

// 		DEBUG("MAKING NEW BLOCK AT: 0x%x\n", newBlk);
		
		// setup the new block
		newBlk->magicWord = MAGIC_VALUE;
		newBlk->size = curBlk->size - sizeRequested - sizeof(MemBlock);	// set the size of the new block
		newBlk->nextFree = curBlk->nextFree;	// link into the free list
		
// 		DEBUG("CUR SIZE: %u NEW SIZE: %u\n", curBlk->size, newBlk->size);
		
		if(prevBlk == curBlk)	// both point to the first block
			freeList = newBlk;
		else
			prevBlk->nextFree = newBlk;

		// update the old block
		curBlk->size = sizeRequested;
		MarkAsUsed(*curBlk);
		curBlk->nextFree = NULL;	// just for safety
	}

	usedMemory += sizeRequested + sizeof(MemBlock);	// rough estimate of used memory
	usedBlocks++;
	
// 	DEBUG("NEW RET PTR: 0x%x\n", reinterpret_cast<uchar *>(curBlk) + sizeof(MemBlock));
	return(reinterpret_cast<uchar *>(curBlk) + sizeof(MemBlock));
}


void MemManager::FreeMemory(void *theMemory)
{
	AutoDisable	lock;	
	MemBlock	*returnedBlk = reinterpret_cast<MemBlock*>(reinterpret_cast<uchar*>(theMemory) - sizeof(MemBlock));
	MemBlock	*prevBlk = freeList, *curBlk = freeList;

	if(IsFreeBlock(*returnedBlk))
	{
		DEBUG("ABOUT TO FREE: 0x%x\n", theMemory);
		PANIC("ALREADY FREE!!!\n");
		return;
	}
	
	// check to see that we have our magic word
	if(returnedBlk->magicWord != MAGIC_VALUE)
	{
		PANIC("MemManager: No magic value found in block to free\n");
	}
	
	MarkAsUnused(*returnedBlk);	// mark as unused

	// update the stats
	usedMemory -= returnedBlk->size + sizeof(MemBlock);
	usedBlocks--;

	// check to see if it is before the start of the free list
	if(returnedBlk < freeList)
	{
		returnedBlk->nextFree = freeList;

		// check to see if we can coalesce with the block above it
		if(reinterpret_cast<char*>(returnedBlk) + sizeof(MemBlock) + returnedBlk->size ==
		   reinterpret_cast<char*>(freeList))
		{
			returnedBlk->size += freeList->size + sizeof(MemBlock);	// update the size
			returnedBlk->nextFree = freeList->nextFree;	// link into the free list
		}

		freeList = returnedBlk;
		return;
	}

	// search through the free list that sandwiches the returned block
	while(curBlk < returnedBlk)
	{
		prevBlk = curBlk;
		curBlk = curBlk->nextFree;

		if(curBlk == NULL)
			PANIC("Couldn't find returned block");
	}

	// link it in as a free block
	// (if we coalesce this is a waste of time...)
	prevBlk->nextFree = returnedBlk;
	returnedBlk->nextFree = curBlk;

	// first check to see if we can coalesce prevBlk and returnedBlk
	if(reinterpret_cast<char*>(prevBlk) + sizeof(MemBlock) + prevBlk->size ==
	   reinterpret_cast<char*>(returnedBlk))
	{
		prevBlk->size += returnedBlk->size + sizeof(MemBlock);	// update the size
		prevBlk->nextFree = curBlk;	// link into the free list
		returnedBlk = prevBlk;		// update the pointer for the next check
	}

	// now check to see if we can coalesce returnedBlk and curBlk
	if(reinterpret_cast<char*>(returnedBlk) + sizeof(MemBlock) + returnedBlk->size ==
	   reinterpret_cast<char*>(curBlk))
	{
		returnedBlk->size += curBlk->size + sizeof(MemBlock);	// update the size
		returnedBlk->nextFree = curBlk->nextFree;	// link into the free list
	}
}

void MemManager::WalkHeap(bool printInfo)
{
	AutoDisable	lock;
	MemBlock	*curBlk = start;
	MemBlock	*lastFree = NULL;
 	ulong		memBlocksTotal = 0;
	
	DEBUG("BYTES: %u  BLOCKS: %u\n", usedMemory, usedBlocks);
	
	while(reinterpret_cast<uchar*>(curBlk) < reinterpret_cast<uchar*>(start)+HEAP_SIZE)
	{
		memBlocksTotal += (GetSize(*curBlk) + sizeof(MemBlock));
		
		if(IsFreeBlock(*curBlk))	// found a free block
		{
			if(lastFree != NULL && lastFree->nextFree != curBlk)
				DEBUG("* LAST FREE DOESN'T POINT TO THIS BLOCK\n");
			lastFree = curBlk;
		}

		if(curBlk->magicWord != 0xDEADC0DE)
		{
			DEBUG("   BAD BLOCK @ 0x%x\n", curBlk);
			return;
		}
		
		if(printInfo)
			DEBUG("%s: SIZE: %u (%u)  ADDR: 0x%x (0x%x)\n",
			       IsFreeBlock(*curBlk) ? "FREE" : "USED",
			       GetSize(*curBlk),
			       GetSize(*curBlk) + sizeof(MemBlock),
			       reinterpret_cast<uchar*>(curBlk) + sizeof(MemBlock),
			       curBlk);
		
		curBlk = reinterpret_cast<MemBlock*>(reinterpret_cast<uchar*>(curBlk) + GetSize(*curBlk) + sizeof(MemBlock));
	}
	
	if(memBlocksTotal == HEAP_SIZE)
		DEBUG("MEMORY LOOKS GOOD\n");
	else
	{
		DEBUG("%u != %u\n", memBlocksTotal, HEAP_SIZE);
		PANIC("MEMORY CORRUPT\n");
	}
}

ulong MemManager::RoundUpTo32Bits(const ulong arg) const
{
	if(arg % 4 == 0)
		return(arg);

	else
		return(arg + (4 - arg%4));
}

ulong MemManager::RoundDownTo32Bits(const ulong arg) const
{
	if(arg % 4 == 0)
		return(arg);

	else
		return(arg - arg%4);
}


