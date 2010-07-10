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


/** @file ATADriver.cpp
 *
 */

#include <constants.h>
#include <types.h>
#include <AutoDisable.h>
#include <ATADriver.h>
#include <io_utils.h>
#include <mem_utils.h>
#include <Debug.h>


ATADriver::ATADriver(ushort controller, uchar device, ulong baseAddress)
	: requestSemaphore(1), ide(controller), dev(device), lbaBase(baseAddress)
{ ; }

ATADriver::ATADriver(const ATADriver &driver)
	: BlockDevice(), requestSemaphore(1), ide(driver.ide), dev(driver.dev), lbaBase(driver.lbaBase)
{ ; }

int ATADriver::ReadBlocks(ulong address, int blockCount, void *dest)
{
	requestSemaphore.Wait();	// wait until the device is ready
	
	AutoDisable	lock;
	int		totalBlocksRead = 0;
	uchar		statusReg;
	
// 	DEBUG("ASKED FOR: %u\n", address);
// 	DEBUG("OFFSET: %u\n", lbaBase);
	
	// update the address
	address += lbaBase;
	
// 	DEBUG("READING BLOCK: %u\n", address);

	// we can only read 255 blocks at a time
	for(int curBlockCount = MIN(blockCount, MAX_BLOCK_COUNT);
		   totalBlocksRead < blockCount;
		   totalBlocksRead += curBlockCount)
	{
		curBlockCount = MIN(blockCount - totalBlocksRead, MAX_BLOCK_COUNT);
		
		do { statusReg = inb(ide | ALT_STATUS_REG); } while(statusReg & BUSY);
		
		asm ("cli");	// clear interrupts
		
		// wait until the drive is ready
		do { statusReg = inb(ide | ALT_STATUS_REG); } while(!(statusReg & DRIVE_READY));
		
		// set the number of blocks to read
		outb(ide | SECTOR_COUNT_REG, curBlockCount);
		
		// set the 1st byte of the LBA
		outb(ide | SECTOR_NUMBER_REG, (address+totalBlocksRead) & 0xFF);
		
		// set the 2nd byte of the LBA
		outb(ide | CYLINDER_LOW_REG, ((address+totalBlocksRead) >> 8) & 0xFF);
		
		// set the 3rd byte of the LBA
		outb(ide | CYLINDER_HIGH_REG, ((address+totalBlocksRead) >> 16) & 0xFF);
		
		// select the device and the high 4 bits of the LBA
		outb(ide | DRIVE_HEAD_REG, dev | (((address+totalBlocksRead) >> 24) & 0x0F));
		
		// send the READ SECTOR WITH RETRIES command
		outb(ide | COMMAND_REG, READ_SECTOR_RETRY);
		
		asm("sti");	// enable interrupts
		
		for(int i=0; i < curBlockCount; ++i)
		{
			// INT HAPPENS HERE
			
			asm("cli");	// clear interrupts
			
			// read the ALT_STATUS_REG so we don't mess up the INT
			do { statusReg = inb(ide | ALT_STATUS_REG); } while(statusReg & BUSY);
			
			// poll to see if ready
			do { statusReg = inb(ide | ALT_STATUS_REG); }
			while(!(statusReg & DRIVE_READY) &&
			       (statusReg & DATA_READY));
			
			if(statusReg & 0x01)	// we have an error
			{
				statusReg = inb(ide | ERROR_REG);
				PANIC("ATA READ ERROR: %x\n", statusReg);
			}
			
			// read in 1 (ONE) SECTOR of data, 1 block = 256 words or 512 bytes
			insw(ide | DATA_REG,
			     reinterpret_cast<uchar*>(dest) + (i*512) + (totalBlocksRead*512),
			     256);
			
			// read the primary status register ONCE PER INT
			statusReg = inb(ide | STATUS_REG);
			
			if(statusReg & 0x01)	// we have an error
			{
				statusReg = inb(ide | ERROR_REG);
				PANIC("ATA READ ERROR: %x\n", statusReg);
			}
			
			asm("sti");	// endable interrupts
			
			// need to delay for 400ns
			Delay400ns(ide);
		}
	}

	
	requestSemaphore.Signal();	// let other commands be issued

	return(blockCount);
}

int ATADriver::WriteBlocks(ulong address, int blockCount, void *src)
{
	(void) address;
	(void) blockCount;
	(void) src;
	
	PANIC("ATA Write support not done.\n");
	
	return(0);
}

void ATADriver::Delay400ns(ushort ide)
{
	inb(ide | ALT_STATUS_REG);
	inb(ide | ALT_STATUS_REG);
	inb(ide | ALT_STATUS_REG);
	inb(ide | ALT_STATUS_REG);
}

void ATADriver::ResetController(ushort ide)
{
	// set the rest bit
	outb(DEV_CONTROL_REG, SOFTWARE_RESET);
		
	// delay for 400ns
	ATADriver::Delay400ns(ide);
	
	// set it to a zero now
	outb(DEV_CONTROL_REG, 0x00);
   
	// delay for 400ns
	ATADriver::Delay400ns(ide);
}


