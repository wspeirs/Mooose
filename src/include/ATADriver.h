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


/** @file ATADriver.h
 *
 */

#ifndef ATADRIVER_H
#define ATADRIVER_H

#include <constants.h>
#include <types.h>
#include <list.h>
#include <Semaphore.h>
#include <Devices.h>

using k_std::list;

/** @class ATADriver
 *
 * @brief This is the device driver for the ATA hard disk controller.
 * 
 * This driver actually does not inherit from driver because it's manager handles everything.
 *
 **/

class ATADriver : public BlockDevice
{
	friend class ATAManager;	// so the manager can access the constants
public:
	/**
	 * The only constructor for an ATADriver.
	 * @param controller The IDE controller to use: primary, secondary.
	 * @param device The device on that IDE controller.
	 */
	ATADriver(ushort controller, uchar device, ulong baseAddress = 0);
	
	/**
	 * The copy constructor.
	 * 
	 * Everything is copied except the semaphore.
	 * @param driver The drive to construct this one from.
	 */
	ATADriver(const ATADriver &driver);
	
	/**
	 * Changes the lbaBase.
	 * 
	 * Use with care...
	 * @param newBase The new base address.
	 */
	inline void ChangeBaseAddress(ulong newBase)
	{ lbaBase = newBase; }
	
	/**
	 * Reads blocks from the device.
	 * @param address The address of the first block to read.
	 * @param blockCount The number of blocks to read.
	 * @param dest The memory location to write the blocks to.
	 * @return The number of blocks read.
	 */
	int ReadBlocks(ulong address, int blockCount, void *dest);
	
	/**
	 * Writes blocks to the device.
	 * @param address The address of the first block to write.
	 * @param blockCount The number of blocks to write.
	 * @param src The memory location to read the blocks from.
	 * @return The number of blocks written.
	 */
	int WriteBlocks(ulong address, int blockCount, void *src);
	
	/**
	 * Returns the size of a block for the device.
	 * @return The number of blocks read.
	 */
	ulong GetBlockSize()
	{ return 512; }

private:
	Semaphore	requestSemaphore;	// only 1 command on the wire at a time
	ushort		ide;
	uchar		dev;
	ulong		lbaBase;
	
	/**
	 * Calls the alt status register 4 times to delay for 400ns.
	 * @param ide The IDE controller.
	 */
	static void Delay400ns(ushort ide);
	
	/**
	 * Resets a controller.
	 * @param ide The IDE controller.
	 */
	static void ResetController(ushort ide);
			
	
	// Registers
	static const ushort DATA_REG		= 0x0100;
	static const ushort ERROR_REG		= 0x0101;
	static const ushort FEATURES_REG	= 0x0101;
	static const ushort SECTOR_COUNT_REG	= 0x0102;
	static const ushort SECTOR_NUMBER_REG	= 0x0103;	///< Low LBA
	static const ushort CYLINDER_LOW_REG	= 0x0104;	///< Mid LBA
	static const ushort CYLINDER_HIGH_REG	= 0x0105;	///< High LBA
	static const ushort DRIVE_HEAD_REG	= 0x0106;
	static const ushort STATUS_REG		= 0x0107;
	static const ushort COMMAND_REG		= 0x0107;
	static const ushort ALT_STATUS_REG	= 0x0306;
	static const ushort DEV_CONTROL_REG	= 0x0306;
	
	// Commands (manditory)
	static const uchar EXEC_DIAG		= 0x90;
	static const uchar IDENT_DEV		= 0xEC;
	static const uchar INIT_DEV_PARAMS	= 0x91;
	static const uchar READ_LONG_RETRY	= 0x22;
	static const uchar READ_LONG_NO_RETRY	= 0x23;
	static const uchar READ_MULTIPLE	= 0xC4;
	static const uchar READ_SECTOR_RETRY	= 0x20;
	static const uchar READ_SECTOR_NO_RETRY	= 0x21;
	static const uchar READ_VERIFY_SEC_RETRY	= 0x40;
	static const uchar READ_VERIFY_SEC_NO_RETRY	= 0x41;
	static const uchar SEEK			= 0x70;
	static const uchar SET_MULTIPLE_MODE	= 0xC6;
	static const uchar WRITE_BUFFER		= 0xE8;
	static const uchar WRITE_LONG_RETRY	= 0x32;
	static const uchar WRITE_LONG_NO_RETRY	= 0x33;
	static const uchar WRITE_MULTIPLE	= 0xC5;
	static const uchar WRITE_SECTOR_RETRY	= 0x30;
	static const uchar WRITE_SECTOR_NO_RETRY= 0x31;
	static const uchar WRITE_VERIFY		= 0x3C;
	static const uchar RESET_DEVICE		= 0x08;
	
	// Common values
	static const uchar BUSY			= 0x80;
	static const uchar DRIVE_READY		= 0x40;
	static const uchar DRIVE_FAULT		= 0x20;
	static const uchar SEEK_FINISH		= 0x10;
	static const uchar DATA_READY		= 0x08;
	static const uchar CORRECT_DATA 	= 0x04;
	static const uchar INDEX		= 0x02;
	static const uchar ERROR		= 0x01;
	static const uchar SOFTWARE_RESET	= 0x04;
	static const int   MAX_BLOCK_COUNT	= 255;	///< This should really be read from the info
};


#endif // ATADriver.h


