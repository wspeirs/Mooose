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


/** @file Partition.h
 *
 */

#ifndef PARTITION_H
#define PARTITION_H


#include <constants.h>
#include <types.h>


/** @class Partition
 *
 * @brief 
 *
 **/

class Partition
{
public:
	Partition();
	Partition(uchar *firstSector);
	
	struct ParsedPartitionDescriptor
	{
		bool	bootable;
		uchar	partitionType;
		ulong	lbaPartitionStart;
		ulong	partitionSize;	
	};
	
	int ParsePartitionInformation(uchar *firstSector);
	ParsedPartitionDescriptor GetPartitionInformation(int partitionNumber);
	ulong GetPartitionStartAddress(int partitionNumber);

	static const uchar	LINUX_TYPE = 0x83;
	
private:
	
	struct PartitionDescriptor
	{
		uchar	bootIndicator;
		uchar	chsPartitionStart[3];
		uchar	partitionType;
		uchar	chsPartitionEnd[3];
		ulong	lbaPartitionStart;
		ulong	partitionSize;	
	} __attribute__((packed));
	
	ParsedPartitionDescriptor	primaryPartitions[4];
};


#endif // Partition.h


