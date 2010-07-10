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


/** @file PCIDriver.cpp
 *
 */

#include <constants.h>
#include <types.h>
#include <PCIDriver.h>
#include <io_utils.h>
#include <Debug.h>


PCIDriver::PCIDriver(uchar bus) : bus(bus)
{
	DeviceConfig	tmp;
	
	for(int dev=0; dev < 32; ++dev)
		GetDeviceConfiguration(tmp, dev);
}

void PCIDriver::GetDeviceConfiguration(DeviceConfig &config, uchar device)
{
	for(uint func = 0; func < 8; ++func)
	{
		ConfigRegister	theConfigReg(bus, device, func, 0);
		ulong	tmp = ReadConfig(theConfigReg);
		
		if(tmp != 0xFFFFFFFF)
		{
			config.vendorID = tmp & 0xFFFF;
			config.deviceID = tmp >> 16;
			
			theConfigReg.theReg.internal.regNum += 4;
			tmp = ReadConfig(theConfigReg);
			config.command = tmp & 0xFFFF;
			config.status = tmp >> 16;
			
			theConfigReg.theReg.internal.regNum += 4;
			tmp = ReadConfig(theConfigReg);
			config.revisionID = tmp & 0xFFFF;
			config.subClass = (tmp >> 16) & 0xFF;
			config.classCode = tmp >> 24;
			
			theConfigReg.theReg.internal.regNum += 4;
			tmp = ReadConfig(theConfigReg);
			config.cacheLineSize = tmp & 0xFF;
			config.latencyTimer = (tmp >> 8) & 0xFF;
			config.headerType = (tmp >> 16) & 0xFF;
			config.selfTest = tmp >> 24;
			
			theConfigReg.theReg.internal.regNum += 4;
			config.baseAddr0 = ReadConfig(theConfigReg);

			theConfigReg.theReg.internal.regNum += 4;
			config.baseAddr1 = ReadConfig(theConfigReg);

			theConfigReg.theReg.internal.regNum += 4;
			config.baseAddr2 = ReadConfig(theConfigReg);

			theConfigReg.theReg.internal.regNum += 4;
			config.baseAddr3 = ReadConfig(theConfigReg);

			theConfigReg.theReg.internal.regNum += 4;
			config.baseAddr4 = ReadConfig(theConfigReg);
			
			theConfigReg.theReg.internal.regNum += 4;
			config.baseAddr5 = ReadConfig(theConfigReg);

			theConfigReg.theReg.internal.regNum += 4;
			config.cardbusCISPtr = ReadConfig(theConfigReg);

			theConfigReg.theReg.internal.regNum += 4;
			tmp = ReadConfig(theConfigReg);
			config.subsysVendorID = tmp & 0xFFFF;
			config.subsysID = tmp >> 16;

			theConfigReg.theReg.internal.regNum += 4;
			config.expansionROMAddr = ReadConfig(theConfigReg);
			
			theConfigReg.theReg.internal.regNum += 4;
			config.reserved[0] = ReadConfig(theConfigReg);
			
			theConfigReg.theReg.internal.regNum += 4;
			config.reserved[1] = ReadConfig(theConfigReg);

			theConfigReg.theReg.internal.regNum += 4;
			tmp = ReadConfig(theConfigReg);
			config.intLine = tmp & 0xFF;
			config.intPin = (tmp >> 8) & 0xFF;
			config.minGrant = (tmp >> 16) & 0xFF;
			config.maxLatency = tmp >> 24;
			
			DEBUG("%d:%d:%d\n", bus, device, func);
			DEBUG("  DEV: %x VEN: %x CLASS: %d SUB CLASS: %d\n",
				config.deviceID, config.vendorID, config.classCode, config.subClass);
			DEBUG("  ADDR0: 0x%x ADDR1: 0x%x ADDR2: 0x%x ADDR3: 0x%x ADDR4: 0x%x ADDR5: 0x%x \n", config.baseAddr0, config.baseAddr1, config.baseAddr2, config.baseAddr3, config.baseAddr4, config.baseAddr5);
			DEBUG("INT LINE: %d INT PIN: %d\n", config.intLine, config.intPin);
		}
	}
}

ulong PCIDriver::ReadConfig(const ConfigRegister &reg)
{
	outl(CONFIG_ADDRESS, reg.theReg.config);	// write out the register
	return inl(CONFIG_DATA);			// read and return the data
}

