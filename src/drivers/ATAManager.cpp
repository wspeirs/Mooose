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


/** @file ATAManager.cpp
 *
 */

#include <constants.h>
#include <types.h>
#include <ATAManager.h>
#include <AutoDisable.h>
#include <io_utils.h>
#include <mem_utils.h>
#include <Debug.h>

int ATAManager::Startup()
{
	DeviceInfo	tmpInfo;
	
	// poll the hardware and insert each device into the map
	if(GetDeviceInformation(PRIMARY_IDE, DEVICE_0, tmpInfo))
	{
		ATADrivers.insert(pair<string, ATADriver*>(string("/dev/hda"), new ATADriver(PRIMARY_IDE, DEVICE_0)));
		ATAInformation.insert(pair<string, DeviceInfo>(string("/dev/hda"), tmpInfo));
// 		PrintATAInfo(tmpInfo);
	}
	
	if(GetDeviceInformation(PRIMARY_IDE, DEVICE_1, tmpInfo))
	{
		ATADrivers.insert(pair<string, ATADriver*>(string("/dev/hdb"), new ATADriver(PRIMARY_IDE, DEVICE_1)));
		ATAInformation.insert(pair<string, DeviceInfo>(string("/dev/hdb"), tmpInfo));
// 		PrintATAInfo(tmpInfo);
	}
	
	if(GetDeviceInformation(SECONDARY_IDE, DEVICE_0, tmpInfo))
	{
		ATADrivers.insert(pair<string, ATADriver*>(string("/dev/hdc"), new ATADriver(SECONDARY_IDE, DEVICE_0)));
		ATAInformation.insert(pair<string, DeviceInfo>(string("/dev/hdc"), tmpInfo));
// 		PrintATAInfo(tmpInfo);
	}
	
	if(GetDeviceInformation(SECONDARY_IDE, DEVICE_1, tmpInfo))
	{
		ATADrivers.insert(pair<string, ATADriver*>(string("/dev/hdd"), new ATADriver(SECONDARY_IDE, DEVICE_1)));
		ATAInformation.insert(pair<string, DeviceInfo>(string("/dev/hdd"), tmpInfo));
// 		PrintATAInfo(tmpInfo);
	}
	
	// once we have all the drives installed, go through and partition them
	list<ulong>	addrList;
	
	for(map<string, ATADriver*>::iterator it = ATADrivers.begin(), end = ATADrivers.end(); it != end; ++it)
	{
		addrList.clear();	// clear out the list first
		GetPartitionAddresses((*it).second, addrList);	// get the addresses
	
		char	charCounter = '1';
	
		// go through the list and make devices for each of these
		for(list<ulong>::iterator jt = addrList.begin(); jt != addrList.end(); ++jt)
		{
			ATADriver	*tmp = new ATADriver(*((*it).second));	// copy this device
			string		tmpStr(1, charCounter);
			
			tmp->ChangeBaseAddress(*jt);	// change it's base address
			
			// insert the device with it's new name into the manager
			ATADrivers.insert(pair<string, ATADriver*>((*it).first + tmpStr, tmp));
			
			++charCounter;
		}
	}
	
	return(0);
}

int ATAManager::GetPartitionAddresses(ATADriver *blockDevice, list<ulong> &addrList)
{
	uchar			*buff = new uchar[1024];
	PartitionDescriptor	*partDes;
	
	// assumes 512 blocks
	blockDevice->ReadBlocks(0, 2, buff);
// 	blockDevice->ReadBlocks(116, 2, buff);
	
	// sanity check
	if(buff[510] != 0x55 || buff[511] != 0xAA)
	{
		PANIC("BAD PARTITION\n", false);
		return(-1);
	}

	for(int i=0; i < 4; ++i)
	{
		partDes = reinterpret_cast<PartitionDescriptor*>(&buff[446+(i*16)]);
		
		if(partDes->lbaPartitionStart != 0)
		{
// 			DEBUG("GOT %d\n", partDes->lbaPartitionStart);
// 			DEBUG("MAKING IT: %d\n", partDes->lbaPartitionStart + 116);
			
			addrList.push_back(partDes->lbaPartitionStart);
		}
	}
	
	
	delete [] buff;
	
	return 0;
}


int ATAManager::Shutdown()
{
	// go through the map and delete all the devices
	for(map<string, ATADriver*>::iterator it = ATADrivers.begin(); it != ATADrivers.end(); ++it)
		delete (*it).second;
	
	ATADrivers.clear();	// clear the map
	
	return(0);
}

ATADriver *ATAManager::GetDeviceByName(string deviceName)
{
	map<string, ATADriver*>::iterator it = ATADrivers.find(deviceName);
	
	if(it == ATADrivers.end())
		return NULL;
	
	return (*it).second;
}


/// Helper function to byte swap things
void ByteSwap(uchar *addr, ulong size)
{
	uchar	tmp;

	for(ulong i=0; i < size; i += 2)
	{
		tmp = addr[i];
		addr[i] = addr[i+1];
		addr[i+1] = tmp;
	}
}

bool ATAManager::GetDeviceInformation(ushort controller, uchar device, DeviceInfo &devInfo)
{
	AutoDisable	lock;	// disable ints
	uchar		statusReg;
	
	MemSet(&devInfo, 0, sizeof(DeviceInfo)); // just zero it out
		
	// delay for 400ns
	ATADriver::Delay400ns(controller);
	
	// check the status register
	statusReg = inb(controller | ATADriver::ALT_STATUS_REG);
	
	// if we get 0xFF from the status register then no controller :-(
	if(statusReg == 0xFF)
		return false;
	
// 	DEBUG("*** START (%x) ***\n", statusReg);
	
	// wait for the drive to not be busy
	do { statusReg = inb(controller | ATADriver::ALT_STATUS_REG); }
	while(statusReg & ATADriver::BUSY);
	
// 	DEBUG("STATUS REG: %x\n", statusReg);
	
	asm("cli");	// clear interrupts
	
	// wait for the drive to be ready
	do { statusReg = inb(controller | ATADriver::ALT_STATUS_REG); }
	while(!(statusReg & ATADriver::DRIVE_READY));
	
// 	DEBUG("STATUS REG: %x\n", statusReg);
	
	// select the device
	outb(controller | ATADriver::DRIVE_HEAD_REG, device);
	
	// send the "IDENTIFY DRIVE" command
	outb(controller | ATADriver::COMMAND_REG, ATADriver::IDENT_DEV);
	
	asm("sti");	// enable interrupts
	
// 	DEBUG("SENT COMMAND\n");
	
	// INT HAPPENS HERE
	
	// wait for the driver to not be busy
	do { statusReg = inb(controller | ATADriver::ALT_STATUS_REG); }
	while(statusReg & ATADriver::BUSY);
	
// 	DEBUG("STATUS REG: %x\n", statusReg);
	
	asm("cli");	// clear interrupts
	
	// poll to see if the drive is ready
	do { statusReg = inb(controller | ATADriver::ALT_STATUS_REG); }
	while(!(statusReg & ATADriver::DRIVE_READY) && (statusReg & ATADriver::DATA_READY));
	
// 	DEBUG("STATUS REG: %x\n", statusReg);
	
	ushort	tmpBuf[256];
	
	// getting an error probably means we're dealing with ATAPI
	if(statusReg & ATADriver::ERROR)
	{
// 		DEBUG("GOT AN ERROR\n");
		
		uchar cl, ch;
		
		// read in cylinder high & low to check for ATAPI
		cl = inb(controller | ATADriver::CYLINDER_LOW_REG);
		ch = inb(controller | ATADriver::CYLINDER_HIGH_REG);
		
// 		DEBUG("LOW 0x%x == 0x14 && HIGH 0x%x == 0xeb\n", cl, ch);
		
 		if(cl == 0x14 && ch == 0xeb)	// we have a packet device
		{
			// perform a software reset
			ATADriver::ResetController(controller);
		}
			
		else	// some other error
			PANIC("Unknown error identifying drives", false);
		
		//
		// Reset this device
		//
			
		// wait for the drive to not be busy
/*		do { statusReg = inb(controller | ATADriver::ALT_STATUS_REG); }
		while(statusReg & ATADriver::BUSY);

		asm("cli");
		
		// wait for the drive to be ready
		do { statusReg = inb(controller | ATADriver::ALT_STATUS_REG); }
		while(!(statusReg & ATADriver::DRIVE_READY));

		// select the device
		outb(controller | ATADriver::DRIVE_HEAD_REG, device);
		
		// send the "RESET DEVICE" command
		outb(controller | ATADriver::COMMAND_REG, ATADriver::RESET_DEVICE);
		
		asm("sti");
		
		// wait for the drive to not be busy
		do { statusReg = inb(controller | ATADriver::ALT_STATUS_REG); }
		while(statusReg & ATADriver::BUSY);
		
		statusReg = inb(controller | ATADriver::STATUS_REG);
		
		printf("STATUS: %x\n", statusReg);		
*/	
		return(false);
	}
	
// 	DEBUG("READING DATA\n");
	
	// it is 256 words (ushort) or 512 bytes (uchar)
	insw(controller | ATADriver::DATA_REG, tmpBuf, 256);
	
	// read the primary status register once
	inb(controller | ATADriver::STATUS_REG);
	
	// fill in the info for the drive
	devInfo.ATAPI_dev = tmpBuf[0] & 0x8000;
	devInfo.numLogCylinders = tmpBuf[1];
	devInfo.numLogHeads = tmpBuf[3];
	devInfo.numSecPreTrack = tmpBuf[6];
	
	MemCopy(devInfo.serialNumber, &tmpBuf[10], sizeof(devInfo.serialNumber));
	MemCopy(devInfo.firmwareVersion, &tmpBuf[23], sizeof(devInfo.firmwareVersion));
	MemCopy(devInfo.modelNumber, &tmpBuf[27], sizeof(devInfo.modelNumber));

	// need to byte swap the model number
	ByteSwap(devInfo.modelNumber, sizeof(devInfo.modelNumber));
	
	devInfo.maxSectorsForMultiple = tmpBuf[47] & 0x00FF;
	devInfo.numCurLogCylinders = tmpBuf[54];
	devInfo.numCurLogHeads = tmpBuf[55];
	devInfo.numCurLogSectorsPerTrack = tmpBuf[56];
	
	MemCopy(&devInfo.curCapacityInSectors, &tmpBuf[57], sizeof(devInfo.curCapacityInSectors));
	
	devInfo.curNumSectorsForMultiple = tmpBuf[59] & 0x00FF;
	
	MemCopy(&devInfo.totalAddressableSectors, &tmpBuf[60], sizeof(devInfo.totalAddressableSectors));

	devInfo.ATAVersion = tmpBuf[80];
	
	return(true);	
}

void ATAManager::PrintATAInfo(const DeviceInfo &info)
{
	DEBUG("TYPE: %s\n", info.ATAPI_dev ? "ATAPI" : "ATA");
	DEBUG("NUM CYLINDERS: %d\n", info.numLogCylinders);
	DEBUG("NUM HEADS: %d\n", info.numLogHeads);
	DEBUG("SEC PER TRACK: %d\n", info.numSecPreTrack);
	DEBUG("SERIAL NUMBER: ");
	for(int i=0; i < 20; ++i) DEBUG("%c", info.serialNumber[i]);
	DEBUG("\n");
	
	DEBUG("FIRMWARE VERSION: ");
	for(int i=0; i < 8; ++i) DEBUG("%c", info.firmwareVersion[i]);
	DEBUG("\n");
	
	DEBUG("MODEL NUMBER: ");
	for(int i=0; i < 40; ++i) DEBUG("%c", info.modelNumber[i]);
	DEBUG("\n");
	
	DEBUG("MAX SEC FOR MULT: %d\n", info.maxSectorsForMultiple);
	DEBUG("NUM CUR CYLINDERS: %d\n", info.numCurLogCylinders);
	DEBUG("NUM CUR HEADS: %d\n", info.numCurLogHeads);
	DEBUG("NUM CUR SEC PER TRACK: %d\n", info.numCurLogSectorsPerTrack);
	DEBUG("CAPACITY IN SEC: %d\n", info.curCapacityInSectors);
	DEBUG("NUM SEC FOR MULT: %d\n", info.curNumSectorsForMultiple);
	DEBUG("TOTAL SECTORS: %d\n", info.totalAddressableSectors);
	DEBUG("ATA VERSION: %d\n", info.ATAVersion);
}

