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


/** @file Devices.h
 *
 */

#ifndef DEVICES_H
#define DEVICES_H

/**
 * @class Device This is the base class all devices are derived from.
 */
class Device
{
	
};

/**
 * @class BlockDevice The interface that all block devices must implement.
 */
class BlockDevice : public Device
{
public:
	virtual int ReadBlocks(ulong address, int blockCount, void *dest) = 0;
	virtual int WriteBlocks(ulong address, int blockCount, void *src) = 0;
	virtual ulong GetBlockSize() = 0;
	virtual ~BlockDevice() { ; }
};

/**
 * @class CharacterDevice The interface that all character devices must implement.
 */
class CharacterDevice : public Device
{
public:
	virtual int GetCharacters(ulong address, int count, void *dest) = 0;
	virtual int PutCharacters(ulong address, int count, void *src) = 0;
	virtual ~CharacterDevice() { ; }
};

#endif // DEVICES_H


