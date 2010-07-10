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


/** @file PhysicalMemManager.h
 *
 */

#ifndef PHYSICALMEMMANAGER_H
#define PHYSICALMEMMANAGER_H

#include <constants.h>
#include <types.h>
#include <i386.h>
#include <Handler.h>
#include <list.h>
#include <vector.h>
#include <algorithms.h>
#include <ProcessManager.h>
#include <FileSystemManager.h>

using k_std::list;
using k_std::vector;
using k_std::find_if;
using k_std::find_if_backwards;

#define PHYS2STRUCTADDR(x) ( (x) >> 12)

/** @class PhysicalMemManager
 *
 * @brief This is the physical memory manager to handle paging.
 *
 * Virtual Memory in MOOSE looks like this
 *
 * <table border=0>
 * <tr><td><hr>			</td><td>4 GB			</td></tr>
 * <tr><td>Mapped Page Tables	</td><td>			</td></tr>
 * <tr><td><hr>			</td><td>4 GB - 4 MB		</td></tr>
 * <tr><td>Mapped Page Directory</td><td>			</td></tr>
 * <tr><td><hr>			</td><td>4 GB - 4 MB - 4096	</td></tr>
 * <tr><td>Kernel's stack, heap and data</td><td>		</td></tr>
 * <tr><td><hr>			</td><td>3 GB			</td></tr>
 * <tr><td>User Land Memory	</td><td>			</td></tr>
 * <tr><td><hr>			</td><td>0 GB			</td></tr>
 * </table>
 */

class PhysicalMemManager : public Handler
{
public:
	/**
	 * The only constructor for the PhysicalMemManager.
	 * It requires the start and end of RAM which will be rounded accordingly and a pointer to the virtual page table mapping
	 * @param startRAM The first address of physical memory.
	 * @param endRAM The last address of physical memory.
 	 * @param vPgTbl The pointer to the virtual page tables.
	 */
	PhysicalMemManager(ulong startRAM, ulong endRAM, PageTableEntry *vPgTbl);

	/**
	 * The startup function used by the Handler.
	 * This is unused.
	 */
	int Startup() { return(0); }
	
	/**
	 * The shutdown function used by the Handler.
	 * This is unused.
	 */
	int Shutdown() { return(0); }
	
	/**
	 * The handle routine for the Handler.
	 * This function grabs the fault address, rounds it down, and does some error checking.
	 * If everything's OK then HandlePageFault is called.
	 * @param regs The Registers structure passed by the Handler. The error code is checked.
	 * @return Returns whatever HandlePageFault returns, or -1 on an error.
	 */
	int Handle(Registers *regs);
	
	/**
	 * Creates a page dir in physical memory, sets it up like the base page dir, returns its address.
	 * @param procID The process ID to assign the page to.
	 * @return The address of the physical page.
	 */
	ulong CreatePageDirectory(ulong procID);
	
	/**
	 * Creates a copy of a page directroy, setting all user-land entries to NOT writable.
	 * @param procID The process ID to assign the page to.
	 * @param srcPageDir The physical address of the page directory to be copied.
	 * @return The address of the physical page.
	 */
	ulong CopyPageDirectory(ulong procID, ulong srcPageDir);
	
	/**
	 * Sets the page directory.
	 * @param pageDirPhysAddress The physical address of the new page directory.
	 */
	void SetPageDirectory(ulong pageDirPhysAddress);
	
	/**
	 * Sets the page directory to the kernel's page directory.
	 */
	static void SetToKernelPageDirectory();
	
	/**
	 * Copy page.
	 * This is for use with copy-on-write.
	 * @param virtualAddress The virtual address (rounded down) of the page to copy.
	 * @param pageDirPhysAddress The physical address of the page directory.
	 */
	void CopyPage(ulong virtualAddress, ulong pageDirPhysAddress);
	
	/**
	 * Free all of the physical pages that a process has allocated.
	 * @param procID The ID of the process.
	 */
	void FreeProcPages(ulong procID);
	
	/**
	 * Searches through the page table for a free page and returns the physical address.
	 * @return The physical address of a free page.
	 */
	ulong FindFreePage();
	
	/**
	 * Maps part of a file to a memory address or a process.
	 * @param fd The file descriptor of the file to read from, should be opened.
	 * @param offset The offset into the file to start reading from.
	 * @param amount The amount of data to read from the file.
	 * @param addr The address to map the data to.
	 * @param pageDir The page directory to map this data into.
	 */
	inline void MapMemoryFromFile(int fd, int offset, uint amount, ulong addr, ulong pageDir)
	{ MapMemoryFromFile(ProcessManager::GetInstance().GetFileDescriptor(fd), offset, amount, addr, pageDir); }
	
	/**
	 * Maps part of a file to a memory address or a process.
	 * @param fd A file descriptor pointer to the file to read from, should be opened.
	 * @param offset The offset into the file to start reading from.
	 * @param amount The amount of data to read from the file.
	 * @param addr The address to map the data to.
	 * @param pageDir The page directory to map this data into.
	 */
	void MapMemoryFromFile(FileDescriptorBase *fd, int offset, uint amount, ulong addr, ulong pageDir);
	
	/**
	 * Maps a physical page into the virtual address space.
	 * @param virtualAddress The virtual address to map the physical page to.
	 * @param physicalPage The physical page to map into virtual address space.
	 * @param pageDirPhysAddress The page directory address to use. Default is the current proc.
	 * @param procID The ID of the proc to be associated with this page.
	 */
	void MapPage(ulong virtualAddress, ulong physicalPage, ulong pageDirPhysAddress = 0, ulong procID = 0);
	
	/**
	 * Acquires a new physical page and maps it into memory.
	 * @param virtualAddress The virtual address to map the acquired physical page to.
	 * @param pageDirPhysAddress The page directory address to use. Default is the current proc.
	 */
	void AcquireAndMapPage(ulong virtualAddress, ulong pageDirPhysAddress = 0);

	/**
	 * Return statistics about how many physical pages have been used in the system.
	 * @param usedPages Fills in the number of pages used.
	 * @param freePages Fills in the number of pages free.
	 * @brief The number of pages used + number free might not be the total pages. All pages allocated by the kernel are thrown away.
	 */
	void GetStats(ulong &usedPageCount, ulong &freePageCount);
	
	/**
	 * Returns the physical address of the current page directory.
	 * @return The physical address of the current page directory.
	 */
	inline ulong GetCurrentPageDirectory()
	{ return currentPageDirectory; }
	
	/**
	 * Returns an instance to this class.
	 * It's a Singleton even though it doesn't inherit from Singleton
	 * @return A pointer to this class.
	 */
	static inline PhysicalMemManager* GetInstancePtr()
	{ return myself; }

private:
	/**
	 * Rounds an address up to a page boundry.
	 * @param arg The address to round.
	 * @return The rounded address.
	 */
	ulong RoundUpAPage(ulong arg);
	
	/**
	 * Rounds an address down to a page boundry.
	 * @param arg The address to round.
	 * @return The rounded address.
	 */
	ulong RoundDownAPage(ulong arg);
	
	/**
	 * Converts and address to a page number.
	 * @param arg The address to convert.
	 * @return The page number.
	 */
	inline ulong AddressToPageNumber(ulong addr) { return(addr/PAGE_SIZE);	}
	
	/**
	 * Converts and address to a directory index.
	 * @param arg The address to convert.
	 * @return The directory index.
	 */
	inline ulong AddressToDirIndex(ulong addr)   { return(addr/(PAGE_SIZE*NUM_PAGE_TABLE_ENTRIES));	}

	/**
	 * Flushes the entire TLB.
	 * Used only as needed.
	 */
	inline void FlushTLB()
	{ asm __volatile__ ("movl %cr3, %eax;\nmovl %eax, %cr3\n"); }
	
	/** @class PageInfo
	 * Stores information about a used page.
	 */
	class PageInfo
	{
		friend class PhysicalMemManager;
	public:
		/**
		 * Default constructor so we can make a list of these.
		 */
		PageInfo() { ; }
		
		/**
		 * The only usable constructor that sets the internal values.
		 * @param p The process ID.
		 * @param pa The physical address.
		 * @param va The virtual address.
		 */
		PageInfo(ulong p, ulong pa, ulong va) : procID(p), physcialAddr(pa), virtualAddr(va)
		{ ; }
		
		/**
		 * The less than operator that compares process IDs.
		 * @param right The right PageInfo struct.
		 * @return True if this->procID &lt; right.procID
		*/
		bool operator<(const PageInfo &right) const
		{ return(procID < right.procID); }
		
		/**
		 * The equal operator that compares process IDs.
		 * @param right The process ID to test against.
		 * @return True if this->procID == right
		*/
		bool operator==(const ulong &right)
		{ return(procID == right); }
			
	private:
		ulong	procID;		///< the ID of the process using the page
		ulong	physcialAddr;	///< the base address of the page (MUST BE ON A PAGE BOUNDARY)
		ulong	virtualAddr;	///< the base address of the page in virtual memory (NOT SURE WHY WE'D NEED THIS)
	};
	
	ulong		startAddr;	///< The lowest address of usable physical RAM
	ulong		endAddr;	///< The highest address of usable physical RAM
	
	typedef	list<PageInfo>::iterator	usedPageIt_t;	///< Used list iterator
	typedef	vector<bool>::iterator		freePageIt_t;	///< Free list iterator

	list<PageInfo>	usedPages;	///< A list of the pages that are used
	vector<bool>	freePages;	///< A bitmap of the pages that are free
		
	PageDirectoryEntry	*thePageDir;		///< A pointer to the page table that maps the page tables
	PageTableEntry		*theVirtualPageTable;	///< A pointer to where the page tables are mapped
	PageTableEntry		*thePageTables;		///< A pointer to where the page directory is mapped
	
	ulong			currentPageDirectory;	///< The physic address of the current page directory
	
	static PhysicalMemManager	*myself;	///< A pointer to the static object
	static bool			created;	///< Boolean indicating if the object was created
};


#endif // PhysicalMemManager.h


