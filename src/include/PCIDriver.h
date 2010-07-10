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


/** @file PCIDriver.h
 *
 */

#ifndef PCIDRIVER_H
#define PCIDRIVER_H


#include <constants.h>
#include <types.h>


/** @class PCIDriver
 *
 * @brief 
 *
 **/

class PCIDriver
{
public:
	/**
	 * Sets up the PCI bus by scanning it for devices.
	 */
	PCIDriver(uchar bus);

private:
	// Taken from http://www.osdev.org/wiki/PCI
	struct DeviceConfig
	{
		ushort	vendorID;	///< The ID of the vendor
		ushort	deviceID;	///< The ID of the device
		ushort	command;
		ushort	status;		///< The status of the device
		ushort	revisionID;
		uchar	subClass;
		uchar	classCode;
		uchar	cacheLineSize;
		uchar	latencyTimer;
		uchar	headerType;
		uchar	selfTest;
		ulong	baseAddr0;
		ulong	baseAddr1;
		ulong	baseAddr2;
		ulong	baseAddr3;
		ulong	baseAddr4;
		ulong	baseAddr5;
		ulong	cardbusCISPtr;
		ushort	subsysVendorID;
		ushort	subsysID;
		ulong	expansionROMAddr;
		ulong	reserved[2];
		uchar	intLine;
		uchar	intPin;
		uchar	minGrant;
		uchar	maxLatency;
	};
	
	class ConfigRegister
	{
		friend class PCIDriver;
	public:
		ConfigRegister(uchar bus, uchar device, uchar function, uchar regNum)
		{
			theReg.internal.regNum = regNum & 0xFC;
			theReg.internal.function = function;
			theReg.internal.device = device;
			theReg.internal.busNum = bus;
			theReg.internal.reserved = 0;
			theReg.internal.enabled = 1;
		}
		
	private:
		union ConfigReg
		{
			ulong	config;
			struct
			{
				ulong	regNum:8;
				ulong	function:3;
				ulong	device:5;
				ulong	busNum:8;
				ulong	reserved:7;
				ulong	enabled:1;
			} __attribute__((packed)) internal;
		};
		
		ConfigReg	theReg;
	};
	
	void GetDeviceConfiguration(DeviceConfig &config, uchar device);
	ulong ReadConfig(const ConfigRegister &device);
	
	// variables
	uchar	bus;
	
	// constants
	static const ushort CONFIG_ADDRESS = 0xCF8;
	static const ushort CONFIG_DATA	   = 0xCFC;
};


#endif // PCIDriver.h


