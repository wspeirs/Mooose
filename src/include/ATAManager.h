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


/** @file ATAManager.h
 *
 */

#ifndef ATAMANAGER_H
#define ATAMANAGER_H


#include <constants.h>
#include <types.h>
#include <map.h>
#include <list.h>
#include <string.h>
#include <Handler.h>
#include <Singleton.h>
#include <ATADriver.h>

using k_std::map;
using k_std::list;
using k_std::pair;
using k_std::string;

/** @class ATAManager
 *
 * @brief The manager of the ATA devices.
 *
 **/

class ATAManager : public Driver, public Singleton<ATAManager>
{
public:
	/**
	 * This runs when the manager is installed.
	 * 
	 * It polls the hardware and figures out what ATA devies are attached
	 * and then creates drivers for each one.
	 */
	int Startup();
	
	/**
	 * This handles all the ATA interrupts.
	 * 
	 * Currently this function does nothing.
	 */
	int IRQSignaled(Registers *regs)
	{
		(void)regs;
// 		printf("ATA INT\n");
		return(0);
	}
	
	/**
	 * This runs when the manager is uninstalled.
	 * 
	 * It cleans up all the memory made for the drivers.
	 */
	int Shutdown();
	
	/**
	 * Returns the device give a string.
	 * @param deviceName The string for the device: /dev/hda, /dev/hdb, etc
	 * @return A pointer to the device or NULL if it doesn't exist.
	 */
	ATADriver *GetDeviceByName(string deviceName);
	
private:
	struct DeviceInfo
	{
		bool	ATAPI_dev;		// if its ATA or ATAPI
		ushort	numLogCylinders;	// number of logical cylinders
		ushort	numLogHeads;
		ushort	numSecPreTrack;
		uchar	serialNumber[20];
		uchar	firmwareVersion[8];
		uchar	modelNumber[40];
		uchar	maxSectorsForMultiple;	// max sectors read or written for MULTIPLE
		ushort	numCurLogCylinders;
		ushort	numCurLogHeads;
		ushort	numCurLogSectorsPerTrack;
		ulong	curCapacityInSectors;
		uchar	curNumSectorsForMultiple;	// current value of number of sectors per MULTIPLE
		ulong	totalAddressableSectors;	// LBA
		uchar	ATAVersion;
	};
	
	struct PartitionDescriptor
	{
		uchar	bootIndicator;
		uchar	chsPartitionStart[3];
		uchar	partitionType;
		uchar	chsPartitionEnd[3];
		ulong	lbaPartitionStart;
		ulong	partitionSize;	
	} __attribute__((packed));

	/**
	 * Polls the given controller and device to see if it is there or not.
	 */
	bool GetDeviceInformation(ushort controller, uchar device, DeviceInfo &devInfo);
	
	/**
	 * Fills in a list of start addresses for all the partitions on the device.
	 * 
	 * Assumes the partition table is <b>1024</b> bytes into the block device.
	 * @param blockDevice The block device to read the partition table from.
	 * @param addrList The list of addresses that will be returned.
	 * @return Error status.
	 */
	int GetPartitionAddresses(ATADriver *blockDevice, list<ulong> &addrList);

	/**
	 * Prints the information of an ATA device. (DEBUG ONLY)
	 * 
	 * @param info The ATA info.
	 */
	void PrintATAInfo(const DeviceInfo &info);
	
	map<string, ATADriver*>	ATADrivers;	///< A map of all the ATA drivers and their names
	map<string, DeviceInfo>	ATAInformation;	///< A map of all the ATA names and their information

	// Devices
	static const uchar DEVICE_0	= 0xE0; //0xA0;
	static const uchar DEVICE_1	= 0xF0; //0xB0;
	// Controllers
	static const ushort PRIMARY_IDE		= 0x00F0;
	static const ushort SECONDARY_IDE	= 0x0070;

};


#endif // ATAManager.h


