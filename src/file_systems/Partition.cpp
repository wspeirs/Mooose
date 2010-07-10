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


/** @file Partition.cpp
 *
 */

#include <constants.h>
#include <types.h>
#include <Partition.h>
#include <screen_utils.h>
#include <Debug.h>

Partition::Partition()
{
	// MemSet(primaryPartitions, 0, sizeof(ParsedPartitionDescriptor)*4);
}

Partition::Partition(uchar *firstSector)
{
	ParsePartitionInformation(firstSector);	// setup the partitions
}

int Partition::ParsePartitionInformation(uchar *firstSector)
{
	if(firstSector[510] != 0x55 || firstSector[511] != 0xAA)
	{
		WARN("Invalid partition type\n");
		return(-1);
	}
	
	for(int i=0; i < 4; ++i)
	{
		PartitionDescriptor *tmpDes = (PartitionDescriptor*)&firstSector[446+(i*16)];
		
		primaryPartitions[i].bootable		= tmpDes->bootIndicator & 0x80;
		primaryPartitions[i].partitionType	= tmpDes->partitionType;
		primaryPartitions[i].lbaPartitionStart	= tmpDes->lbaPartitionStart;
		primaryPartitions[i].partitionSize	= tmpDes->partitionSize*512;
	}
	
/*	for(int i=0; i < 4; ++i)
	{
		printf("Partition %d...\n", i);
		printf(" TYPE: %s\n", primaryPartitions[i].partitionType == 0x83 ? "LINUX" : "UNKNOWN");
		printf(" SIZE: %u bytes\n", primaryPartitions[i].partitionSize);
		printf("START: 0x%x\n", primaryPartitions[i].lbaPartitionStart);
	}
*/
	return(0);
}

Partition::ParsedPartitionDescriptor Partition::GetPartitionInformation(int partitionNumber)
{
	if(partitionNumber < 0 || partitionNumber > 3)
	{
		PANIC("Out of bounds partition request\n");	
	}
	
	return(primaryPartitions[partitionNumber]);
}

ulong Partition::GetPartitionStartAddress(int partitionNumber)
{
	if(partitionNumber < 0 || partitionNumber > 3)
	{
		PANIC("Out of bounds partition request\n");	
	}
	
	return(primaryPartitions[partitionNumber].lbaPartitionStart);
}

/*
ulong Partition::GetPartitionStartAddress(int num)
{
	return(primaryPartitions[num].lbaPartitionStart);
}
*/


