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

/** @file mem_manage.h
 *
 */

#ifndef MEM_MANAGE_H
#define MEM_MANAGE_H

#define MAGIC_VALUE	0xDEADC0DE
#define HEAP_SIZE	0x4000000	// 64 MB heap
//#define HEAP_SIZE	(1024 * 1024)

#include <types.h>
#include <constants.h>


// Memory blocks will have the following form
/*
|-------------------|
|   0xDEADC0DE      |
|-------------------|
|   Size (in bytes) | LSB: 0 = free, 1 = used
|-------------------|
|   NextFreeBlk     |
|-------------------|
|                   |
|                   |
|    Allocated      |
|     Memory        |
|   (rounded up     |
|    to 32 bits)    |
|                   |
|                   |
|-------------------|
*/

/** @class MemManager
 *
 * @brief This is the memory manager for the heap of the kernel.
 *
 */
class MemManager
{
	class MemBlock;

public:
	/**
	 * The only constructor for the MemManager.
	 * @param startHeap A pointer to the the start of the heap. The end is determined by a constant HEAP_SIZE.
	 */
	MemManager(void *startHeap);

	/**
	 * The function that allocates memory on the kernel's heap.
	 * The global <b>new</b> operator calls this function.
	 * @param bytes The amount of memory to be allocated.
	 * @return A void pointer to the newly allocated memory.
	 */
	void *AllocateMemory(const ulong bytes);
	
	/**
	 * The function that frees memory from the kernel's heap.
	 * The global <b>delete</b> operator calls this function.
	 * @param theMemory A pointer to the memory to free.
	 */
	void FreeMemory(void *theMemory);

	/**
	 * Returns a pointer to the instance of the memory manager.
	 * Can't use Singleton for this because it there is no default constructor.
	 * @return A pointer to the MemManager.
	 */
	static inline MemManager *GetInstancePtr()
	{ return myself; }

	/**
	 * A <b>DEBUG</b> function to walk through the kernel's heap.
	 * @param printInfo If you want information printed to the screen or not. Prints 1 line per block.
	 */
	void WalkHeap(bool printInfo = false);
	
	/**
	 * Reports how much memory is used by the kernel's heap.
	 * @return The amount of memory used.
	 */
	inline ulong GetUsedMemory() { return(usedMemory); }

private:
	/**
	 * The copy constructor.
	 * Defined as private so no one can use it.
	 */
	MemManager(const MemManager &m)
	{ (void)m; }
	
	/**
	 * The assignment operator.
	 * Defined as private so no one can use it.
	 */
	MemManager& operator=(const MemManager &m)
	{ (void)m; return(*this); }
	
	
	/** @class MemBlock
	* @brief This is the representation of a memory block
	* It doesn't have any member functions, only variables.
	*/
	class MemBlock
	{
		friend class MemManager;
	protected:
		ulong		magicWord;	// a value to make sure nothing bad has happened
		ulong		size;		// this holds the size of the block
		MemBlock	*nextFree;	// this is a pointer to the next free block
	} __attribute__((packed));		// pack this structure

	
	/**
	 * Rounds a ulong up to a 32-bit boundary.
	 * @param arg The ulong to be rounded up to a 32-bit boundary.
	 * @return arg rounded up to a 32-bit boundary.
	 */
	ulong RoundUpTo32Bits(const ulong arg) const;
	
	/**
	 * Rounds a ulong down to a 32-bit boundary.
	 * @param arg The ulong to be rounded down to a 32-bit boundary.
	 * @return arg rounded down to a 32-bit boundary.
	 */
	ulong RoundDownTo32Bits(const ulong arg) const;
	
	/**
	 * Rounds a ulong pointer up to a 32-bit boundary.
	 * @param arg The ulong pointer to be rounded up to a 32-bit boundary.
	 * @return arg rounded up to a 32-bit boundary.
	 */
	inline ulong *RoundUpTo32Bits(const ulong *arg) const
	{ return reinterpret_cast<ulong*>(RoundUpTo32Bits(reinterpret_cast<ulong>(arg))); }

	/**
	 * Rounds a ulong pointer down to a 32-bit boundary.
	 * @param arg The ulong pointer to be rounded down to a 32-bit boundary.
	 * @return arg rounded down to a 32-bit boundary.
	 */
	inline ulong *RoundDownTo32Bits(const ulong *arg) const
	{ return reinterpret_cast<ulong*>(RoundUpTo32Bits(reinterpret_cast<ulong>(arg))); }

	/**
	 * Checks a block to see if it is used or not.
	 * @param theBlock The block to check.
	 * @return True if the block is free, false if it is used.
	 */
	inline bool IsFreeBlock(const MemManager::MemBlock &theBlock) const
	{ return(!(theBlock.size & 0x1)); }

	/**
	 * Marks a block as used.
	 * @param theBlock The block to mark.
	 */
	inline void MarkAsUsed(MemManager::MemBlock &theBlock) const
	{ theBlock.size |= 0x1; }

	/**
	 * Marks a block as unused.
	 * @param theBlock The block to mark.
	 */
	inline void MarkAsUnused(MemManager::MemBlock &theBlock) const
	{ theBlock.size &= ~0x1; }
	
	/**
	 * Returns the size of a block without modifying the used bit.
	 * @param theBlock The block to get the size of.
	 */
	inline ulong GetSize(const MemManager::MemBlock &theBlock) const
	{ return theBlock.size & 0xFFFFFFFE; }
	

	static MemBlock *start;	// the start of memory
	static MemBlock *end;	// the end of memory

	ulong	usedMemory;	// amount of memory used
	ulong	usedBlocks;	// number of blocks used

	MemBlock	*freeList;	// points to the start of the free list
	
	static MemManager	*myself;	// self reference
	static bool		created;	// check if it has already been created or not
};

#endif
