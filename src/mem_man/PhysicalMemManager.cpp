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


/** @file PhysicalMemManager.cpp
 *
 */

#include <PhysicalMemManager.h>
#include <Debug.h>
#include <mem_utils.h>
#include <AutoDisable.h>

PhysicalMemManager *PhysicalMemManager::myself;
bool PhysicalMemManager::created = false;

// this is the kernel's page dir, we use it as a reference for all new page dirs
extern PageDirectoryEntry	pageDir[];

PhysicalMemManager::PhysicalMemManager(ulong startRAM, ulong endRAM, PageTableEntry *vPgTbl)
	: startAddr(RoundUpAPage(startRAM)),
	  endAddr(RoundDownAPage(endRAM)), 
	  freePages((endAddr - startAddr) / PAGE_SIZE, true),
	  currentPageDirectory(ulong(pageDir) - VIRTUAL_OFFSET)
{
	if(created)
		PANIC("Tried to create two PhysicalMemManager\n");
	
	myself = this;
	created = true;
	
	vga_printf(" (%d PAGES) ", (endAddr - startAddr) / PAGE_SIZE);
	
	// Setup the pointers to the page directories and tables
	theVirtualPageTable = vPgTbl;
	thePageTables = reinterpret_cast<PageTableEntry*>    (PAGE_TABLES_MAPPING);
	thePageDir    = reinterpret_cast<PageDirectoryEntry*>(PAGE_DIRECTORY_MAPPING);
	
	// Setup virtual page tables so that they maps to the same
	// page tables pointed to by the page directory
	
	// Zero out the virtual pages
	MemSet(theVirtualPageTable, 0, sizeof(PageTableEntry)*NUM_PAGE_TABLE_ENTRIES);
	
	// Copy over the entries from the page dir so they point to the same physical pages
	// We can do this because they are basically the same
	MemCopy(theVirtualPageTable, pageDir, sizeof(PageTableEntry)*NUM_PAGE_TABLE_ENTRIES);
	
	// Set the last (top) entry to the page dir itself
	// it will never be used and we need it to access the page dir
 	MemSet(&theVirtualPageTable[NUM_PAGE_TABLE_ENTRIES-1], 0, sizeof(PageTableEntry));
	
 	theVirtualPageTable[NUM_PAGE_TABLE_ENTRIES-1].present   = 1;
 	theVirtualPageTable[NUM_PAGE_TABLE_ENTRIES-1].readWrite = 1;
 	theVirtualPageTable[NUM_PAGE_TABLE_ENTRIES-1].pageAddr  = PHYS2STRUCTADDR(uint(pageDir) - VIRTUAL_OFFSET);
	
	// Setup the virtual mapping of the page tables in the page directory
	// zero out it's slot
	MemSet(&pageDir[NUM_PAGE_DIR_ENTRIES-1], 0, sizeof(PageDirectoryEntry));
	
	// set the page directory
	pageDir[NUM_PAGE_DIR_ENTRIES-1].present       = 1;
	pageDir[NUM_PAGE_DIR_ENTRIES-1].readWrite     = 1;
	pageDir[NUM_PAGE_DIR_ENTRIES-1].pageTableAddr = PHYS2STRUCTADDR(uint(theVirtualPageTable) - VIRTUAL_OFFSET);	
	
	
	//
	// at some point might need to map in more kernel pages
	//
}

int PhysicalMemManager::Handle(Registers *regs)
{
	ulong	addr;
		
	// get the address of the page fault
	asm __volatile__ ("movl %%cr2, %%eax;\n movl %%eax, %0": "=r" (addr) : : "%eax");
		
	PANIC("FAULT ADDR: 0x%x  ERROR CODE: 0x%x\n", addr, regs->err_code);
// 	DEBUG("CURRENT PAGE DIR: 0x%x   KERNEL PAGE DIR: 0x%x\n", currentPageDirectory, ulong(pageDir) - VIRTUAL_OFFSET);
	
	addr = RoundDownAPage(addr);	// round down to a page boundry
	
//	printf("ROUNDED FAULT ADDR: 0x%x\n", addr);
	
	ProcessManager &procMan = ProcessManager::GetInstance();

	//
	// For error codes see Intel Vol 3 5-45
	//
	
	// reserved bit set
	if(regs->err_code & 0x8)
		PANIC("Reserved bit set\n");	
	
	// user proc is trying to access memory that they shouldn't be
	if(regs->err_code & 0x5)
	{
		DEBUG("PROC ID: %d\n", procMan.GetCurrentProcID());
		PANIC("Ring 3 process attempting to access Ring 0 memory\n", false);
	}
		
	// check for a fault on the first page of memory
	if(addr < PAGE_SIZE)
	{
		PANIC("PAGE FAULT ON FIRST PAGE\n");
		return(-1);
	}
	
	// check for a fault on the top 4 MB
	if(addr >= PAGE_TABLES_MAPPING)
	{
		PANIC("PAGE FAULT ON LAST 4 MB\n");
		return(-1);
	}
	
	// copy-on-write needed
	if(regs->err_code & 0x7)
	{
		CopyPage(addr, procMan.GetProcPageDir(procMan.GetCurrentProcID()));
		return(0);
	}
	
	// map a new page into the proper virtual address
	MapPage(addr,
		FindFreePage(), 
		procMan.GetProcPageDir(procMan.GetCurrentProcID()),
		procMan.GetCurrentProcID());
	
	return(0);
}

void PhysicalMemManager::MapPage(ulong virtualAddress, ulong physicalPage, ulong pageDirPhysAddress, ulong procID)
{
	if(currentPageDirectory == ulong(pageDir) || pageDirPhysAddress == ulong(pageDir))
		PANIC("WRONG PAGE DIR");
	
	ulong	oldPageDir = currentPageDirectory;
	
	// see if we need to map a new page directory in
	if(pageDirPhysAddress != currentPageDirectory)
		SetPageDirectory(pageDirPhysAddress);
	
	uint	pageDirIndex = AddressToDirIndex(virtualAddress);
	uint	pageNumber = AddressToPageNumber(virtualAddress);
	uchar	priv = pageNumber < AddressToPageNumber(KERNEL_BASE_ADDR) ? 1 : 0;
	
// 	printf("DIR INDEX: %d   PAGE NUMBER: %d\n", pageDirIndex, pageNumber);
	
	// check if there is a page table in the page directory
	if(thePageDir[pageDirIndex].present != 1)	// need to get a page for the page table
	{
		// search through the table for a free page
		uint	physAddr = FindFreePage();
		
		DEBUG("USING 0x%x FOR NEW PAGE TABLE\n", physAddr);
		
		// add to the used list if not for the kernel
		if(virtualAddress < KERNEL_BASE_ADDR)
			usedPages.push_back(PageInfo(procID, physAddr, virtualAddress));
		
		thePageDir[pageDirIndex].pageTableAddr = theVirtualPageTable[pageDirIndex].pageAddr  = PHYS2STRUCTADDR(physAddr);
		thePageDir[pageDirIndex].present       = theVirtualPageTable[pageDirIndex].present   = 1;
		thePageDir[pageDirIndex].readWrite     = theVirtualPageTable[pageDirIndex].readWrite = 1;
		thePageDir[pageDirIndex].userSuper     = theVirtualPageTable[pageDirIndex].userSuper = priv;
		
		// zero out the new page table
		MemSet(&thePageTables[pageDirIndex*NUM_PAGE_TABLE_ENTRIES], 0, PAGE_SIZE);
	}
		
	// add it to the page table
	thePageTables[pageNumber].pageAddr = PHYS2STRUCTADDR(physicalPage);
	thePageTables[pageNumber].present = 1;		// set the present bit
	thePageTables[pageNumber].readWrite = 1;	// set the read/write bit
	thePageTables[pageNumber].userSuper = priv;	// set the privledge level

	if(pageDirPhysAddress != currentPageDirectory)	// reset to the oldPageDir if needed
		SetPageDirectory(oldPageDir);
	
	else
		FlushTLB();
}

ulong PhysicalMemManager::CreatePageDirectory(ulong procID)
{
	AutoDisable	lock;
	
	ulong	retPhysAddr = FindFreePage();
	
	if(procID != KERNEL_PID)	// keep only if NOT kernel
	{
		// setup the info struct
		PageInfo	tmp(procID, retPhysAddr, 0);
		
		usedPages.push_back(tmp);	// add to the used list
	}
	
	// save off the last entry, the page directory
	PageTableEntry	tmpEntry = theVirtualPageTable[NUM_PAGE_TABLE_ENTRIES-1];
	
	// zero it out the entry
	MemSet(&theVirtualPageTable[NUM_PAGE_TABLE_ENTRIES-1], 0, sizeof(PageTableEntry));
	
	// map us in
	theVirtualPageTable[NUM_PAGE_TABLE_ENTRIES-1].pageAddr  = PHYS2STRUCTADDR(retPhysAddr);	// put in the new address
	theVirtualPageTable[NUM_PAGE_TABLE_ENTRIES-1].present   = 1;	// set to present
	theVirtualPageTable[NUM_PAGE_TABLE_ENTRIES-1].readWrite = 1;	// set so we can write to it
	
	FlushTLB();	// do a flush here so we're sure we're copying to the right spot
	
	// make this page dir just like the kernel's page dir
	MemCopy(thePageDir, pageDir, sizeof(PageTableEntry) * NUM_PAGE_TABLE_ENTRIES);
	
	// restore the entry to what it was before we got here
	theVirtualPageTable[NUM_PAGE_TABLE_ENTRIES-1] = tmpEntry;
	
	FlushTLB();	// do another flush here so that we're back to where we were

	return(retPhysAddr);
}

ulong PhysicalMemManager::CopyPageDirectory(ulong procID, ulong srcPageDir)
{
	AutoDisable	lock;
	
	ulong	retPhysAddr = FindFreePage();
	
	if(procID != KERNEL_PID)	// keep only if NOT kernel
	{
		// setup the info struct
		PageInfo	tmp(procID, retPhysAddr, 0);
		
		usedPages.push_back(tmp);	// add to the used list
	}
	
	// save off the last 2 entries
	PageTableEntry	tmpEntry1 = theVirtualPageTable[NUM_PAGE_TABLE_ENTRIES-1];
	PageTableEntry	tmpEntry2 = theVirtualPageTable[NUM_PAGE_TABLE_ENTRIES-2];
	
	// zero it out the entries
	MemSet(&theVirtualPageTable[NUM_PAGE_TABLE_ENTRIES-2], 0, sizeof(PageTableEntry)*2);
	
	// map us in the source page
	theVirtualPageTable[NUM_PAGE_TABLE_ENTRIES-1].pageAddr  = PHYS2STRUCTADDR(srcPageDir);
	theVirtualPageTable[NUM_PAGE_TABLE_ENTRIES-1].present   = 1;	// set to present
	theVirtualPageTable[NUM_PAGE_TABLE_ENTRIES-1].readWrite = 1;	// set so we can write to it
	
	// map in the new page
	theVirtualPageTable[NUM_PAGE_TABLE_ENTRIES-2].pageAddr  = PHYS2STRUCTADDR(retPhysAddr);
	theVirtualPageTable[NUM_PAGE_TABLE_ENTRIES-2].present   = 1;	// set to present
	theVirtualPageTable[NUM_PAGE_TABLE_ENTRIES-2].readWrite = 1;	// set so we can write to it
	
	FlushTLB();	// do a flush here so we're sure we're copying to the right spot
	
	// copy the page dir
	PageDirectoryEntry	*pDirEntry = reinterpret_cast<PageDirectoryEntry*>(PAGE_DIRECTORY_MAPPING - PAGE_SIZE);
	
	DEBUG("pDirEntry: 0x%x\n", pDirEntry);
	DEBUG("thePageDir: 0x%x\n", thePageDir);
	
	MemCopy(pDirEntry, thePageDir, sizeof(PageTableEntry) * NUM_PAGE_TABLE_ENTRIES);
	
	// go through each page table entry in user-land and make those that are present read-only
	for(int i=0; i < NUM_PAGE_TABLE_ENTRIES; ++i)
	{
		// only set those that are user-land and present
		if(pDirEntry[i].userSuper && pDirEntry[i].present)
		{
			DEBUG("SETTING ENTRY: %d\n", i);
			pDirEntry[i].readWrite = 0;	// make read-only
		}
	}
	
	// restore the entries
	theVirtualPageTable[NUM_PAGE_TABLE_ENTRIES-1] = tmpEntry1;
	theVirtualPageTable[NUM_PAGE_TABLE_ENTRIES-2] = tmpEntry2;
	
	FlushTLB();	// do another flush here so that we're back to where we were

	return(retPhysAddr);
}

void PhysicalMemManager::CopyPage(ulong virtualAddress, ulong pageDirPhysAddress)
{
	AutoDisable lock;
	
	ulong	newPage = FindFreePage();	// get a new page
	ulong	oldPageDir = currentPageDirectory;
	
	
	// see if we need to map a new page directory in
	if(pageDirPhysAddress != currentPageDirectory)
		SetPageDirectory(pageDirPhysAddress);
	
	uint	pageNumber = AddressToPageNumber(virtualAddress);

	DEBUG("PAGE NUMBER: %d\n", pageNumber);
	DEBUG("%s\n", theVirtualPageTable[pageNumber].readWrite == 0 ? "READ" : "WRITE");
			
	// save off the last entry, the page directory
	PageTableEntry	tmpEntry = theVirtualPageTable[NUM_PAGE_TABLE_ENTRIES-1];
	
	// zero it out the entry
	MemSet(&theVirtualPageTable[NUM_PAGE_TABLE_ENTRIES-1], 0, sizeof(PageTableEntry));
	
	// map in the new page
	theVirtualPageTable[NUM_PAGE_TABLE_ENTRIES-1].pageAddr  = PHYS2STRUCTADDR(newPage);	// put in the new address
	theVirtualPageTable[NUM_PAGE_TABLE_ENTRIES-1].present   = 1;	// set to present
	theVirtualPageTable[NUM_PAGE_TABLE_ENTRIES-1].readWrite = 1;	// set so we can write to it
	
	FlushTLB();	// do a flush here so we're sure we're copying to the right spot
	
	// copy the page
	MemCopy(thePageDir, reinterpret_cast<uchar*>(virtualAddress), PAGE_SIZE);
	
	// restore the entry to what it was before we got here
	theVirtualPageTable[NUM_PAGE_TABLE_ENTRIES-1] = tmpEntry;

	// map in the new page
	theVirtualPageTable[pageNumber].pageAddr = PHYS2STRUCTADDR(newPage);
	theVirtualPageTable[pageNumber].readWrite = 1;	// allow writes
	
	// reset to the oldPageDir if needed
	if(pageDirPhysAddress != currentPageDirectory)
		SetPageDirectory(oldPageDir);
	
	else
		FlushTLB();
}

void PhysicalMemManager::SetToKernelPageDirectory()
{
	PhysicalMemManager::GetInstancePtr()->SetPageDirectory(ulong(pageDir) - VIRTUAL_OFFSET);
}

void PhysicalMemManager::SetPageDirectory(ulong pageDirPhysAddress)
{
	AutoDisable lock;
	
	DEBUG("SET PAGE DIR: 0x%x -> 0x%x %s\n",
	      currentPageDirectory,
	      pageDirPhysAddress,
	      pageDirPhysAddress == ulong(pageDir) - VIRTUAL_OFFSET ? "(KERNEL)" : "");
	
	// check to see if they're the same, if so just return
	if(currentPageDirectory == pageDirPhysAddress)
		return;
	
	// check to make sure it is page aligned
	if(pageDirPhysAddress % PAGE_SIZE != 0)
	{
		PANIC("Attempting to load a non-page aligned page directory\n");
		return;
	}
	
	// zero it out the entry
	MemSet(&theVirtualPageTable[NUM_PAGE_TABLE_ENTRIES-1], 0, sizeof(PageTableEntry));
	
	// map us in
	theVirtualPageTable[NUM_PAGE_TABLE_ENTRIES-1].pageAddr  = PHYS2STRUCTADDR(pageDirPhysAddress);	// put in the new address
	theVirtualPageTable[NUM_PAGE_TABLE_ENTRIES-1].present   = 1;	// set to present
	theVirtualPageTable[NUM_PAGE_TABLE_ENTRIES-1].readWrite = 1;	// set so we can write to it
	
	FlushTLB();	// do a flush here so we're sure we're copying to the right spot
	
	// update the virtual page mapping to correspond to the new directory
	// *** EXCEPT don't copy over the last entry... that's us!!!
	MemCopy(theVirtualPageTable, thePageDir, sizeof(PageTableEntry)*(NUM_PAGE_TABLE_ENTRIES-1));
	
	// load this page in as our new page directory
	asm __volatile__ ("movl %0, %%eax;\n movl %%eax, %%cr3;" : : "r" (pageDirPhysAddress) : "%eax");
	
	currentPageDirectory = pageDirPhysAddress;
}

ulong PhysicalMemManager::FindFreePage()
{
	static freePageIt_t lastFree = freePages.begin();
	
	// search through the table for a free page
	lastFree = find(lastFree, freePages.end(), true);
	
	// did find one after the last time we found a free page
	// so start from the begining and make sure nothing is free
	if(lastFree == freePages.end())
	{
		// search from the start
		lastFree = find(freePages.begin(), freePages.end(), true);
	
		// if we still can't find one, then we're out of memory :-(
		if(lastFree == freePages.end())
			PANIC("Out of memory");
	}
	
	*lastFree = false;	// mark the page as used
	
	return (lastFree - freePages.begin()) * PAGE_SIZE + startAddr;
}

void PhysicalMemManager::FreeProcPages(ulong procID)
{
	AutoDisable	lock;
	
	// sort the used list by procID
	usedPages.sort();

	// find the first used page to delete
	usedPageIt_t first = find(usedPages.begin(), usedPages.end(), procID);
	
	if(first == usedPages.end())
		return;	// there were no pages allocated for this proc
	
	// get the last page to delete
	usedPageIt_t last = find_backwards(usedPages.begin(), usedPages.end(), procID);

	if(last != usedPages.end())
		++last;	// both insert and erase work from [first, last)

	// go through the list getting the physical addresses and freeing them in the free table
	for(usedPageIt_t it = first; it != last; ++it)
		freePages[((*it).physcialAddr - startAddr)/PAGE_SIZE] = true;

	// remove them from the used list
	usedPages.erase(first, last);
}

void PhysicalMemManager::MapMemoryFromFile(FileDescriptorBase *fd, int offset, uint amount, ulong addr, ulong pageDir)
{
	AutoDisable		lock;
	int			ret;
	
	// seek to the proper offset
	ret = fd->GetFileSystem()->Seek(fd, offset, FileSystemBase::SEEK_SET);
	
	if(ret < 0)
	{
		DEBUG("SEEK RETURNED: %d\n", ret);
		return;
	}
	
	// save the old page directory
	ProcessManager	&procMan = ProcessManager::GetInstance();
	ulong		oldPageDir = GetCurrentPageDirectory();
	
	// load the page directory
	SetPageDirectory(pageDir);
	
	// make a pointer to the spot in memory to write the data
	uchar	*memAddr = reinterpret_cast<uchar*>(addr);
	
	// read the data into that memory location
	ret = fd->GetFileSystem()->Read(fd, memAddr, amount);
	
	if(ret < 0)
	{
		DEBUG("READ ERROR: %d\n", ret);
		return;	
	}
	
	else if(uint(ret) < amount)
	{
		DEBUG("READ: %d of %d\n", ret, amount);
		return;
	}
	
	// the page fault handler will take care of everything for us here...
	
	// restore page directory
	SetPageDirectory(oldPageDir);
}


void PhysicalMemManager::AcquireAndMapPage(ulong virtualAddress, ulong pageDirPhysAddress)
{
	MapPage(virtualAddress, FindFreePage(), pageDirPhysAddress);
}


void PhysicalMemManager::GetStats(ulong &usedPageCount, ulong &freePageCount)
{
	AutoDisable	lock;
	
	usedPageCount = usedPages.size();
	freePageCount = freePages.size() - usedPageCount;
}



ulong PhysicalMemManager::RoundDownAPage(ulong arg)
{
	if(arg % PAGE_SIZE == 0)
		return(arg);

	else
		return(arg - (arg%PAGE_SIZE));
}

ulong PhysicalMemManager::RoundUpAPage(ulong arg)
{
	if(arg % PAGE_SIZE == 0)
		return(arg);

	else
		return(arg + (PAGE_SIZE - arg%PAGE_SIZE));
}
