// kernel.c - the C part of the kernel
/* Copyright (C) 1999  Free Software Foundation, Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/**@file startup.cpp
 * This file starts up the kernel and gets everything setup including
 * the global descriptor tables, and the interrupt descriptor tables.
 */

#include <constants.h>
#include <types.h>
#include <i386.h>
#include <multiboot.h>
#include <screen_utils.h>
#include <mem_utils.h>
#include <Debug.h>
#include <PhysicalMemManager.h>
#include <MemoryManager.h>
#include <InterruptManager.h>
#include <SerialDriver.h>
#include <KeyboardDriver.h>

// Check if the bit in flags is set
#define CHECK_FLAG(flags,bit)   	((flags) & (1 << (bit)))

#define VIRTUAL_OFFSET_IN_DIR_SIZE	(VIRTUAL_OFFSET/sizeof(PageDirectoryEntry))
#define VIRTUAL_OFFSET_IN_TABLE_SIZE	VIRTUAL_OFFSET_IN_DIR_SIZE

// setup by the linker for us
extern ulong _code;	///< The end of the code segment, setup by the linker
extern ulong _end;	///< The end of the kernel, setup by the linker

// for the global descriptor table
extern SystemTableRegister		gdtr;
extern CodeDataSegmentDescriptor	gdt[];

// for the virtual memory that maps to the page directories and the page tables
extern PageDirectoryEntry	pageDir[];
extern PageTableEntry		vPgTable[];

// globals
TaskStateSegment		theTSS;	///< The task state segment, used by the ProcessManager
	
// prototype
void __do_global_ctors();
void SetupDrivers(void*);
void PrintMultiBootInfo(MultibootInfo *multibootInfo);
bool GetRAMStartAndEnd(MultibootInfo *multibootInfo, ulong &ramStart, ulong &ramEnd);


/**
 * Initilizes all of the memory management classes and the interrupt manager.
 * @param magic The multiboot magic number.
 * @param addr The address of the multiboot information.
 */
extern "C" void InitMemAndInts (ulong magic, ulong addr)
{
	// Clear the screen.
	ClearScreen();
	
	// Am I booted by a Multiboot-compliant boot loader?
	if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
	{
		PANIC("Invalid magic number: 0x%x\n", magic);
		return;
	}
	
	// Set MBI to the address of the Multiboot information structure.
	MultibootInfo *multibootInfo = (MultibootInfo *) addr;

	// Print out some information
	PrintMultiBootInfo(multibootInfo);

	//
	// TODO: Add a class that parses the command line
	// have it load after the memory manager, and allows it to take any
	// number of option classes. Each class will search the string looking
	// for it's option, then record what the option is.
	// 

	// get the addresses of usable memory
	// return if the Multiboot Info struct isn't setup correctly
	ulong	ramStart, ramEnd;
	
	if(!GetRAMStartAndEnd(multibootInfo, ramStart, ramEnd))
	{
		PANIC("Couldn't get RAM\n");
		return;
	}
	
	if(ramEnd <= NUM_PAGE_TABLE_ENTRIES * PAGE_SIZE)
		PANIC("NOT ENOUGH RAM", false);
	

	vga_printf("RAM START: 0x%x\n", ramStart);
	vga_printf("  RAM END: 0x%x\n", ramEnd);
	
	uint	kernelEnd = uint(reinterpret_cast<uint>(&_end)) - VIRTUAL_OFFSET;
	
	vga_printf("    __END: 0x%x\n", kernelEnd);
	vga_printf("    VIDEO: 0x%x\n", VGA_VIDEO_ADDR - VIRTUAL_OFFSET);
	
	// This should really be wrapped into a class or at least a function to set the values.
	// Setup the Global Descriptor Table
	// 0 = NULL	(required)
	// 1 = kernel code segment
	// 2 = kernel data segment
	// 3 = user code segment
	// 4 = user data segment
	// 5 = TSS
	
	MemSet(gdt, 0x00, SEG_DESC_SIZE*NUM_SEG_DESC);	// zero them all out
	
	for(int i=1; i < NUM_SEG_DESC; ++i)	// all entries are setup almost the same way
	{
		gdt[i].segLimit      = 0xFFFF;
		gdt[i].lowBaseAddr   = 0x0000;
		gdt[i].midBaseAddr   = 0x00;
		gdt[i].rwEnable      = 1;
		gdt[i].expandConform = 0;
		gdt[i].desType       = 1;
		gdt[i].present       = 1;
		gdt[i].hiLimit       = 0xF;
		gdt[i].opSize        = 1;
		gdt[i].granularity   = 1;
		gdt[i].hiBaseAddr    = 0x0;
	}
	
	// make the kernel's code and data
	gdt[1].desPrivLevel = gdt[2].desPrivLevel = 0;	// ring 0
	gdt[1].isCode = 1;		// kernel code segment
	gdt[2].isCode = 0;		// kernel data segment
	
	// make the user's code and data
	gdt[3].desPrivLevel = gdt[4].desPrivLevel = 3;	// ring 3
	gdt[3].isCode = 1;		// user code segment
	gdt[4].isCode = 0;		// user data segment
	
	
	TaskSwitchingSegmentDescriptor	*tss = reinterpret_cast<TaskSwitchingSegmentDescriptor*>(&gdt[5]);
	
	tss->segLimit    =  sizeof(TaskStateSegment) & 0xFFFF;
	tss->lowBaseAddr =  reinterpret_cast<uint>(&theTSS) & 0xFFFF;
	tss->midBaseAddr = (reinterpret_cast<uint>(&theTSS) >> 16 ) & 0xFF;
	tss->type    = 9;
	tss->zero    = 0;
	tss->desPrivLevel = 0;
	tss->present = 1;
	tss->hiLimit     = (sizeof(TaskStateSegment) >> 16) & 0xFF;
	tss->available = 0;
	tss->zero2 = 0;
	tss->granularity = 1;
	tss->hiBaseAddr  =  reinterpret_cast<uint>(&theTSS) >> 24;
	
	asm __volatile__ ("lgdt %0" : "=m"(gdtr));
	
	//
	// Setup the actual segment
	//	
	MemSet(&theTSS, 0, sizeof(TaskStateSegment));
	
	theTSS.ss0       = 0x10;
	theTSS.iomapbase = sizeof(theTSS);
	
	asm __volatile__ ("ltr %0": : "r"(static_cast<ushort>(0x28)));
	
	
	// create the one and only memory manager for the kernel
	// this takes care of the heap allowing for new and delete to be used
	// this must be installed first before any drivers and/or the interrupt mananger can be used
	vga_printf("Creating the memory manager...");
	static MemManager	theMemManager(&_end);
	vga_printf("DONE\n");
	
	// Setup the interrupt manager
	vga_printf("Creating interrupt manager...");
	InterruptManager	&theInterruptManager = InterruptManager::GetInstance();
	vga_printf("DONE\n");

	// Install the physical memory manager so we can handle page faults
	// we've already mapped the first 4 MB (1 page dir entry) of memory, so our physical memory starts there
	vga_printf("Creating physical memory manager...");
 	static PhysicalMemManager thePhysMemManager(NUM_PAGE_TABLE_ENTRIES * PAGE_SIZE, ramEnd, vPgTable);
	vga_printf("DONE\n");
	
	// install the page fault handler
	theInterruptManager.InstallHandler(&thePhysMemManager, INT_PAGE_FAULT);
	
	//
	// Before we can do anything with static objects we have to init them
	//
	__do_global_ctors();
}

/**
 * Setup all of the "trusted" drivers and manager.
 * 
 * After this function completes the kernel should be 100% running and working. However we still
 * need to setup other drivers so we can do things like read/write to the hard disk and start the
 * init program. These are done by a task in the SetupDrivers function. This way if something goes
 * we are still running and able to print information to the serial port for logging.
 */
extern "C" void InitTasks(void)
{
	asm("cli");	// disable all INTs
	
	InterruptManager	&theInterruptManager = InterruptManager::GetInstance();

	//
	// Setup the console manager, this is needed for the process manager.
	// It is actually the first call to DEBUG that sets up the VirtualConsoleManager
	//
	DEBUG("Creating the virtual console manager...");
	VirtualConsoleManager	&theConsoleManager = VirtualConsoleManager::GetInstance();
	DEBUG_NL("DONE\n");

	// create a serial device on COM1 so we can write debug information
	SerialDriver	*com1 = new SerialDriver(SerialDriver::COM1);

// 	vga_printf("Installing Serial driver...");
	DEBUG("Installing Serial driver...");
	theInterruptManager.InstallHandler(com1, IRQ_4);
	DEBUG_NL("DONE\n");

	//
	// Setup the debug printer to print on the serial port
	//
	Debug::GetInstance().SetSerial(com1);

	//
	// Setup the keyboard so we know what video mode the user wants
	//
	DEBUG("Installing keyboard driver...");
	theInterruptManager.InstallHandler(&KeyboardDriver::GetInstance(), IRQ_1);
	DEBUG_NL("DONE\n");

	//
	// Setup the one and only process manager.
	//
	DEBUG("Creating the process manager...");
	ProcessManager		&theProcessManager = ProcessManager::GetInstance();
	DEBUG_NL("DONE\n");
	
	// with the process manager setup and ready, start installing the drivers.
	theProcessManager.CreateThread(SetupDrivers, NULL, Thread::KERNEL, 0);
	
	// switch to our created thread
	theProcessManager.PerformTaskSwitch();
	
	//
	// We should never get back here, our created thread will run and then the NULL thread.
	//
	PANIC("GOT BACK TO InitTasks!!!");
}


bool GetRAMStartAndEnd(MultibootInfo *multibootInfo, ulong &ramStart, ulong &ramEnd)
{
	ulong	size=0;	// this will hold the size of each segment
	
	if (!CHECK_FLAG (multibootInfo->flags, 6))
		return FALSE;
	
	
	MemoryMap	*pMMap = (MemoryMap *) multibootInfo->mmap_addr;

	// loop through all the memory segments until we find the biggest RAM segment
	for ( ;	(ulong) pMMap < multibootInfo->mmap_addr + multibootInfo->mmap_length;
			pMMap = (MemoryMap *) ((ulong) pMMap + pMMap->size + sizeof (pMMap->size)))
	{
		if(pMMap->type == 0x1)	// we found RAM
		{
			if(pMMap->length_low > size)	// found a bigger section
			{
				ramStart = pMMap->base_addr_low;
				ramEnd = pMMap->base_addr_low + pMMap->length_low;
				size = pMMap->length_low;
			}
		}
	}

	return TRUE;
}

void PrintMultiBootInfo(MultibootInfo *multibootInfo)
{
	vga_printf("Multi Boot Information\n");
	
	// Print out the flags. 
	vga_printf("flags = 0x%x\n", (unsigned) multibootInfo->flags);

	// Are mem_* valid? 
	if (CHECK_FLAG (multibootInfo->flags, 0))
		vga_printf("mem_lower = %uKB, mem_upper = %uKB\n",
			   (unsigned) multibootInfo->mem_lower,
			   (unsigned) multibootInfo->mem_upper);
	
	// Is boot_device valid?
	if (CHECK_FLAG (multibootInfo->flags, 1))
		vga_printf("boot_device = 0x%x\n", (unsigned) multibootInfo->boot_device);
	
	// Is the command line passed? 
	if (CHECK_FLAG (multibootInfo->flags, 2))
		vga_printf("cmdline = %s\n", (char *) multibootInfo->cmdline);

	// Are mods_* valid? 
	if (CHECK_FLAG (multibootInfo->flags, 3))
	{
		ModuleInfo	*pModuleInfo = (ModuleInfo *)multibootInfo->mods_addr;

		vga_printf("mods_count = %d, mods_addr = 0x%x\n",
			   (int) multibootInfo->mods_count,
			   (int) multibootInfo->mods_addr);

		for (unsigned int i=0; i < multibootInfo->mods_count; ++i)
			vga_printf("\tmod_start = 0x%x, mod_end = 0x%x, string = %s\n",
				   (unsigned) pModuleInfo[i].mod_start,
				   (unsigned) pModuleInfo[i].mod_end,
				   (char *) pModuleInfo[i].string);
	}
	
	// Bits 4 and 5 are mutually exclusive! 
	if (CHECK_FLAG (multibootInfo->flags, 4) && CHECK_FLAG (multibootInfo->flags, 5))
	{
		vga_printf("Both bits 4 and 5 are set.\n");
		return;
	}
	
//	// Is the symbol table of a.out valid? 
	if (CHECK_FLAG (multibootInfo->flags, 4))
	{
		AOutSymbolTable *pAOutSym = &(multibootInfo->u.aout_sym);
	
		vga_printf("aout_symbol_table: tabsize = 0x%0x, "
			"strsize = 0x%x, addr = 0x%x\n",
			(unsigned) pAOutSym->tabsize,
			(unsigned) pAOutSym->strsize,
			(unsigned) pAOutSym->addr);
	}
	
	// Is the section header table of ELF valid? 
	if (CHECK_FLAG (multibootInfo->flags, 5))
	{
		ELFSectionHeaderTable *pELFSec = &(multibootInfo->u.elf_sec);
	
		vga_printf("elf_sec: num = %u, size = 0x%x,"
			" addr = 0x%x, shndx = 0x%x\n",
			(unsigned) pELFSec->num, (unsigned) pELFSec->size,
			(unsigned) pELFSec->addr, (unsigned) pELFSec->shndx);
	}
	
	// Are mmap_* valid? 
	if (CHECK_FLAG (multibootInfo->flags, 6))
	{
		MemoryMap	*pMMap = (MemoryMap *) multibootInfo->mmap_addr;
	
		vga_printf("mmap_addr = 0x%x, mmap_length = 0x%x\n",(unsigned) multibootInfo->mmap_addr,
								 (unsigned) multibootInfo->mmap_length);

		for ( ;	(ulong) pMMap < multibootInfo->mmap_addr + multibootInfo->mmap_length;
				pMMap = (MemoryMap *) ((ulong) pMMap + pMMap->size + sizeof (pMMap->size)))

			vga_printf(" size = 0x%x, base_addr = 0x%x%x, length = 0x%x%x, type = 0x%x\n",
				(unsigned) pMMap->size,
				(unsigned) pMMap->base_addr_high,
				(unsigned) pMMap->base_addr_low,
				(unsigned) pMMap->length_high,
				(unsigned) pMMap->length_low,
				(unsigned) pMMap->type);
	}
}


