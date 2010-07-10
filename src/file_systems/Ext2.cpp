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


/** @file Ext2.cpp
 *
 */

#include <constants.h>
#include <types.h>

#ifdef UNIT_TEST

#include "ext2.h"

#else

#include <Ext2.h>
#include <Debug.h>

#endif

// the fd will be changed to a block device
ext2::ext2(BlockDevice *theDrive, ulong baseAddress)
	: FileSystemBase(theDrive, baseAddress)
{
	uchar		*buff = new uchar[1024];	// 2 block buffer, assume 512 byte blocks for ATA driver
	
	//
	// CLEAN THIS UP & DOUBLE CHECK
	//
	ReadBlocks(2, 2, buff);
	
/*	for(int i=0; i < 100; ++i)
		printf("%x ", buff[i]);
	*/
	MemCopy(&theSuperBlock, buff, sizeof(SuperBlock));	// copy over the super block
	
	delete [] buff;	// delete this memory
	
	// sanity checks
	if(theSuperBlock.magicValue != 0xEF53 ||
	   theSuperBlock.creatorOS != 0)
	{
		DEBUG("MAGIC VALUE: 0x%x\n", theSuperBlock.magicValue);
		DEBUG("OS TYPE: 0x%x\n", theSuperBlock.creatorOS);
		PANIC("Invalid super block or wrong OS type\n", false);
	}
	
	// fix up the block size
	switch(theSuperBlock.blockSize)
	{
		case 0:	theSuperBlock.blockSize = blockSize = 1024; break;	
		case 1:	theSuperBlock.blockSize = blockSize = 2048; break;	
		case 2:	theSuperBlock.blockSize = blockSize = 4096; break;
		default: PANIC("Unknown blocks size\n");
	}

	// set the block size multiplier
	SetFileSystemBlockSize(blockSize);
	
	// set the addresses per block
	addrPerBlock = blockSize / sizeof(ulong);
	
/*	DEBUG("BLOCK SIZE: %d\n", blockSize);
	DEBUG("BLOCK MUL: %d\n", blockMultiplier);
	
	DEBUG("*** SUPER BLOCK ***\n");
	DEBUG("INODE COUNT: %ul\n", theSuperBlock.inodeCount);			// Inodes count
	DEBUG("BLOCK COUNT: %ul\n", theSuperBlock.blockCount);			// Blocks count 
	DEBUG("RESERVE COUNT: %u\n", theSuperBlock.reservedBlockCount);	// Reserved blocks count 
	DEBUG("FREE COUNT: %u\n", theSuperBlock.freeBlockCount);		// Free blocks count 
	DEBUG("FREE INODE COUNT: %u\n", theSuperBlock.freeInodeCount);		// Free inodes count 
	DEBUG("FIRST DATA BLOCK: %u\n", theSuperBlock.firstDataBlock);		// First Data Block 
	DEBUG("BLOCK SIZE: %u\n", theSuperBlock.blockSize);			// Block size 
	DEBUG("%u\n", theSuperBlock.fragmentSize);				// Fragment size 
	DEBUG("BLOCKS PER GROUP: %u\n", theSuperBlock.blocksPerGroup);		// # Blocks per group 
	DEBUG("%u\n", theSuperBlock.fragmentsPerGroup);			// # Fragments per group 
	DEBUG("INODES PER GROUP: %u\n", theSuperBlock.inodesPerGroup);		// # Inodes per group 
	DEBUG("MOUNT TIME: %u\n", theSuperBlock.mountTime);			// Mount time 
	DEBUG("WRITE TIME: %u\n", theSuperBlock.writeTime);			// Write time 
	DEBUG("MOUNT COUNT: %u\n", theSuperBlock.mountCount);			// Mount count 
	DEBUG("MAX MOUNT COUNT: %u\n", theSuperBlock.maxMountCount);		// Maximal mount count 
	DEBUG("MAGIC: 0x%x\n", theSuperBlock.magicValue);			// Magic signature 
	DEBUG("STATE: 0x%x\n", theSuperBlock.state);				// File system state 
	DEBUG("ERRORS: %u\n", theSuperBlock.errors);				// Behaviour when detecting errors 
	DEBUG("MINOR REVISION: %u\n", theSuperBlock.minorRevisionLevel);	// minor revision level 
	DEBUG("LAST CHECKED: %u\n", theSuperBlock.timeLastChecked);		// time of last check 
	DEBUG("CHECK INTERVAL: %u\n", theSuperBlock.checkInterval);		// max. time between checks 
	DEBUG("OS: %u\n", theSuperBlock.creatorOS);				// OS 
	DEBUG("REVISION: %u\n", theSuperBlock.revisionLevel);			// Revision level 
	DEBUG("UID: %u\n", theSuperBlock.defaultReservedUID);			// Default uid for reserved blocks 
	DEBUG("GID: %u\n\n", theSuperBlock.defaultReservedGID);		// Default gid for reserved blocks 
*/
	
	//
	// Figure out how many blocks there are for all the group descriptors
	//
	ulong			numGroups = DivUp(theSuperBlock.blockCount, theSuperBlock.blocksPerGroup);
	ulong			blocksForGroupDescriptors = DivUp(numGroups*sizeof(GroupDescriptor), blockSize);
	uchar			*groupBlocks = new uchar[blocksForGroupDescriptors * blockSize];
	GroupDescriptor		*tmpDescriptor;
	
/*	printf("NUM GROUPS: %d\n", numGroups);	
	printf("BLOCKS NEEDED: %d\n", blocksForGroupDescriptors);
	
	printf("SIZE: %d\n", blocksForGroupDescriptors * blockSize);
*/	
	
	int	groupAddr = 0;
	
	// I have no idea how to calculate this value
	switch(blockSize)
	{
		case 1024: groupAddr += 2; break;
		case 4096: groupAddr += 4; break;
		default: PANIC("Unknown block size\n", false);
	}
	
	// read in the group
	ReadBlocks(groupAddr, blocksForGroupDescriptors, groupBlocks);
	
	for(ulong i=0; i < numGroups; ++i)
	{
		tmpDescriptor = reinterpret_cast<GroupDescriptor *>(&groupBlocks[i*sizeof(GroupDescriptor)]);
		
/*		DEBUG("BLOCK BITMAP ADDR: %d\n", tmpDescriptor->blockBitmapAddress);
		DEBUG("INODE BITMAP ADDR: %d\n", tmpDescriptor->inodeBitmapAddress);
		DEBUG("INODE TABLE ADDR: %d\n", tmpDescriptor->inodeTableAddress);
		DEBUG("FREE BLOCK COUNT: %d\n", tmpDescriptor->freeBlockCount);
		DEBUG("FREE INODE COUNT: %d\n", tmpDescriptor->freeInodeCount);
		DEBUG("USED DIR: %d\n\n", tmpDescriptor->usedDirCount);
*/
		theGroupDescriptors.push_back(*tmpDescriptor);	// add it to our list
	}
	
	delete [] groupBlocks;	// free up some memory
}

FileDescriptorBase *ext2::CreateFileDescriptor(FileSystemBase *fsBase)
{
	return reinterpret_cast<FileDescriptorBase*>(new FileDescriptor(fsBase));
}

void ext2::DestroyFileDescriptor(FileDescriptorBase *fd)
{
	delete fd;
}

//
// Find functions
//
int ext2::FindInodeByPath(string path)
{
	list<string>	theDirs;
	
	path.Tokenize('/', theDirs);	// parse the path
	
	int	curInode = 2;
	
// 	DEBUG("SIZE: %d\n", theDirs.size());
// 	DEBUG("PATH: %s\n", path.c_str());
			
	for(list<string>::iterator it = theDirs.begin(); it != theDirs.end(); ++it)
	{
// 		DEBUG("DIR: %s\n", (*it).c_str());
		
		curInode = FindByNameInInode(curInode, *it);
	
// 		DEBUG("CUR INODE: %d\n", curInode);

		if(curInode == -1)
			return(-1);
	}
	
	return(curInode);	
}

int ext2::FindByNameInInode(ulong inode, string name)
{
	Inode		theInode;
	ushort		curSize;
	DirEntry	*tmpDir;
	uchar		*buff = new uchar[blockSize];
	int		ret = -1;
	
	ReadInode(inode, &theInode);	// read in the inode
	
// 	DEBUG("INODE SIZE: %d\n", theInode.size);
// 	DEBUG("INODE BLOCK COUNT: %d\n", theInode.blockCount);
		
	//
	// TODO: Need to change when this stops...
	//
	for(int i=0; i < NUM_BLOCK_PTRS; ++i)
	{
		if(theInode.blockPointers[i] == 0)
			continue;
		
		// read in the data block
		if(ReadDataBlock(i, theInode.blockPointers, buff) < 0)
		{
			WARN("Error reading data block: %d\n", i);
			return(-1);
		}
		
		curSize = 0;
		
		while(curSize < blockSize)
		{
			tmpDir = reinterpret_cast<DirEntry *>(&buff[curSize]);
			
			if(tmpDir->inode != 0)
			{
// 				DEBUG("NAME: %s\n", tmpDir->name);
// 				DEBUG("INODE: %d\n", tmpDir->inode);
// 				DEBUG("TYPE: %d\n", tmpDir->fileType);
				
				// we found the dir we were looking for
				if(name == string(tmpDir->name, tmpDir->nameLength))
				{
					// found what we're looking for, cleanup and return
					int	ret = tmpDir->inode;
					
					delete [] buff;
					
					return(ret);	// return the inode
				}
			
			}
				
			curSize += tmpDir->recordLength;
		}
	}
	
	delete [] buff;	// free up our memory
	
	return(ret);	
}

ulong ext2::FindAddressByBlockNumber(ulong blockNumber, ulong blockPointers[])
{
	ulong	addr = 0;
	ulong	*addrs = NULL;
	
	// make sure the request isn't larger then the largest block number
	if(blockNumber >= ulong(DIRECT_BLOCK_PTRS) + (addrPerBlock * addrPerBlock * addrPerBlock))
		return(0);
	
	// direct data block
	if(blockNumber <= ulong(DIRECT_BLOCK_PTRS))
		addr = blockPointers[blockNumber];
	
	// single indirection
	else if(blockNumber < ulong(INDIRECT_BLOCK_PTR + addrPerBlock))
	{
// 		DEBUG("\nSINGLE INDIRECT\n");
// 		DEBUG("BLOCK NUM: %d\n", blockNumber);
		
		// make space for our inodes
		addrs = new ulong[blockSize/sizeof(ulong)];
		
		// read in the indirect blocks
		ReadBlocks(blockPointers[INDIRECT_BLOCK_PTR], 1, reinterpret_cast<uchar*>(addrs));
		
		// update the block number
		blockNumber -= INDIRECT_BLOCK_PTR;
		
// 		DEBUG("ASKING FOR: %d\n", addrs[blockNumber % addrPerBlock]);
		
		addr = addrs[blockNumber % addrPerBlock];
	}
	
	// double indirection
	else if(blockNumber < ulong(INDIRECT_BLOCK_PTR + (addrPerBlock * addrPerBlock)))
	{
// 		DEBUG("\nDOUBLE INDIRECT\n");
// 		DEBUG("BLOCK NUM: %d\n", blockNumber);
		
		// make space for our inodes
		addrs = new ulong[blockSize/sizeof(ulong)];
		
		// read in the indirect blocks
		ReadBlocks(blockPointers[DOUBLE_INDIRECT_BLOCK_PTR], 1, reinterpret_cast<uchar*>(addrs));
		
		// update the block number
		blockNumber = blockNumber - INDIRECT_BLOCK_PTR - addrPerBlock;
		 
		// read in the double indirect blocks
		ReadBlocks(addrs[blockNumber / addrPerBlock], 1, reinterpret_cast<uchar*>(addrs));
		
// 		DEBUG("ASKING FOR: %d\n", addrs[blockNumber % addrPerBlock]);
		
		addr = addrs[blockNumber % addrPerBlock];
	}
	
	// tripple indirection
	else
	{
// 		DEBUG("\nTRIPPLE INDIRECT\n");
// 		DEBUG("BLOCK NUM: %d\n", blockNumber);
		
		// make space for our inodes
		addrs = new ulong[blockSize/sizeof(ulong)];
		
		// read in the indirect blocks
		ReadBlocks(blockPointers[TRIPPLE_INDIRECT_BLOCK_PTR], 1, reinterpret_cast<uchar*>(addrs));
		
		// update the block number
		blockNumber = blockNumber - INDIRECT_BLOCK_PTR - addrPerBlock - (addrPerBlock * addrPerBlock);
		 
		// read in the double indirect blocks
		ReadBlocks(addrs[blockNumber / (addrPerBlock * addrPerBlock)], 1, reinterpret_cast<uchar*>(addrs));
		
		// read in the tripple indirect blocks
		ReadBlocks(addrs[blockNumber / addrPerBlock], 1, reinterpret_cast<uchar*>(addrs));
		
// 		DEBUG("ASKING FOR: %d\n", addrs[blockNumber % addrPerBlock]);
		
		addr = addrs[blockNumber % addrPerBlock];
	}
	
	// cleanup any memory we might have made
	if(addrs != NULL)
		delete [] addrs;

	return(addr);
}


//
// Read functions
//
int ext2::ReadInode(ulong inode, Inode *theInode)
{
	if(inode < 2)
	{
		WARN("Tried to get inode zero\n");
		return(-1);
	}
	
	ulong	groupNumber = inode / theSuperBlock.inodesPerGroup; // DivUp(inode, theSuperBlock.inodesPerGroup);
	ulong	blocksNeeded = DivUp(theSuperBlock.inodesPerGroup * sizeof(Inode), blockSize); // cal the blocks needed for all inodes
	uchar	*buff = new uchar[blocksNeeded * blockSize];
	Inode	*tmpInode = reinterpret_cast<Inode*>(buff);
	
/*	DEBUG("INODE: %d\n", inode);
	DEBUG("INODES PER GROUP: %d\n", theSuperBlock.inodesPerGroup);
	DEBUG("GROUP NUMBER: %d\n", groupNumber);
	DEBUG("INODE TABLE ADDR: %d\n", theGroupDescriptors[groupNumber].inodeTableAddress);
	DEBUG("BLOCKS REQUESTED: %d\n", blocksNeeded);
	DEBUG("INODE SIZE: %d\n", sizeof(Inode));
	*/
	
	// read the inode table for that group
	int ret = ReadBlocks(theGroupDescriptors[groupNumber].inodeTableAddress, blocksNeeded, buff);
	
	if(ret >= 0)	 // decrease inode because array index at zero and inodes at one
		MemCopy(theInode, &tmpInode[(inode-1) % theSuperBlock.inodesPerGroup], sizeof(Inode));
	
	delete [] buff;
	
	return(ret);
}

// this function reads a data block performing indirection if necessary
int ext2::ReadDataBlock(ulong blockNumber, ulong blocksPointers[], uchar *dest)
{
// 	DEBUG("BLOCK NUM: %u\n", blockNumber);
// 	DEBUG("BLOCK: %u\n", blocksPointers[blockNumber]);
	
	ulong	addr = FindAddressByBlockNumber(blockNumber, blocksPointers);
	
	if(addr == 0)
		PANIC("Couldn't find block");
	
	return ReadBlocks(addr, 1, dest);
}


//
// Write functions
//
int ext2::WriteInode(ulong inode, Inode *theInode)
{
	if(inode < 2)
	{
		ERROR("Tried to write inode zero\n");
		return(-1);
	}
	
	ulong	groupNumber = inode / theSuperBlock.inodesPerGroup;
	ulong	blocksNeeded = DivUp(theSuperBlock.inodesPerGroup * sizeof(Inode), blockSize); // calc the blocks needed for all inodes
	uchar	*buff = new uchar[blocksNeeded * blockSize];
	Inode	*tmpInode = reinterpret_cast<Inode*>(buff);
	
/*	DEBUG("INODE: %d\n", inode);
	DEBUG("INODES PER GROUP: %d\n", theSuperBlock.inodesPerGroup);
	DEBUG("GROUP NUMBER: %d\n", groupNumber);
	DEBUG("INODE TABLE ADDR: %d\n", theGroupDescriptors[groupNumber].inodeTableAddress);
	DEBUG("BLOCKS REQUESTED: %d\n", blocksNeeded);
	DEBUG("INODE SIZE: %d\n", sizeof(Inode));
*/
	// read the inode table for that group
	int ret = ReadBlocks(theGroupDescriptors[groupNumber].inodeTableAddress, blocksNeeded, buff);
	
	// check for an error on the read
	if(ret < 0)
	{
		delete [] buff;
		return ret;
	}
	
	// decrease inode because array index at zero and inodes at one
	MemCopy(&tmpInode[(inode-1) % theSuperBlock.inodesPerGroup], theInode, sizeof(Inode));
	
	// write this back out to disk
	ret = WriteBlocks(theGroupDescriptors[groupNumber].inodeTableAddress, blocksNeeded, buff);
	
	delete [] buff;
	
	return(ret);
	
}


int ext2::WriteDataBlock(ulong blockNumber, FileDescriptor *fd, uchar *src)
{
	// find if we need to allocate a block or not
	ulong	addr = FindAddressByBlockNumber(blockNumber, fd->fileInode.blockPointers);
	
	if(addr == 0)	// we need to allocate a new block
	{
		PANIC("NEED TO ALLOCATE NEW BLOCK\n", false);
		addr = AllocateNewDataBlock(blockNumber, fd);
	}
	
	// write the block out to disk
	return WriteBlocks(addr, 1, src);
}

ulong ext2::AllocateNewDataBlock(ulong blockNumber, FileDescriptor *fd)
{
	// FIX ME
	(void)blockNumber;
	
	int	groupNumber = fd->inodeNumber / theSuperBlock.inodesPerGroup;
	
	if(theGroupDescriptors[groupNumber].freeBlockCount == 0)
	{
		ERROR("FILE SYSTEM FULL\n");
		return 0;
	}
	
	// read in the block bitmap
	ulong	numBlocksForBitmap = DivUp(DivUp(theSuperBlock.blocksPerGroup,ulong(8)), ulong(blockSize));
	uchar	*freeBlockBitmap = new uchar[numBlocksForBitmap * blockSize];
	
	DEBUG("BLOCKS PER GROUP: %d\n", theSuperBlock.blocksPerGroup);
	DEBUG("NUM BLOCKS: %d\n", numBlocksForBitmap);
	
	// search through the bitmap for an unused block
	ReadBlocks(theGroupDescriptors[groupNumber].blockBitmapAddress, numBlocksForBitmap, freeBlockBitmap);
	
	for(uint i=0; i < numBlocksForBitmap * blockSize; ++i)
		DEBUG("%x ", freeBlockBitmap[i]);
	
/*	vector<bool>	bitMap(reinterpret_cast<ulong*>(freeBlockBitmap), theSuperBlock.blocksPerGroup);
	
	vector<bool>::iterator it = find(bitMap.begin(), bitMap.end(), false);
	
	
 	for(it = bitMap.begin(); it != bitMap.end(); ++it)
 		printf("%s", *it == true ? "U" : "F");
*/
	
	// mark it as used
	
	// update the count in the group descriptor
	
	// update the count in the super block
	
	// update the count in the inode
	
	return(0);
}

		

//
// Directory stuff
//
/*
int ext2::OpenDirectory(const string &path)
{
	DirDescriptor	tmpDirDescriptor;

	int	inode = FindInodeByPath(string(path));
	
	if(inode < 0)
	{
		printf("FindInodeByPath returned: %u\n", inode);
		return(-1);
	}
		
	ReadInode(inode, &tmpDirDescriptor.theInode);
	
	// check to make sure it returns a directory
	if(!(tmpDirDescriptor.theInode.fileMode & DIR_FILE_MODE) )
	{
		printf("FILE MODE: 0x%x != 0x%x (DIR)\n", tmpDirDescriptor.theInode.fileMode, DIR_FILE_MODE);
		printf("SIZE: %u\n", tmpDirDescriptor.theInode.size);
		printf("BLOCK COUNT: %u\n", tmpDirDescriptor.theInode.blockCount);
		return(-2);
	}
	
	// SHOULD WRAP WITH SEMAPHORES
	int ret = theDirDescriptors.size();
	
	theDirDescriptors.push_back(tmpDirDescriptor);
	
	return(ret);
}

int ext2::ReadDirectory(int dirDescriptor, list<DirectoryEntry> &theEntries)
{
	// NEED TO WRAP IN SEMAPHORE
	
	if(uint(dirDescriptor) > theDirDescriptors.size())
	{
		PANIC("Invalid dir descriptor\n");
		return(-1);
	}
	
	ushort		curSize;
	DirEntry	*tmpDir;
	uchar		*buff = new uchar[blockSize];
	
	//
	// TODO: Need to double check when this stops
	//
	for(int i=0; i < NUM_BLOCK_PTRS; ++i)
	{
		if(theDirDescriptors[dirDescriptor].theInode.blockPointers[i] == 0)
			continue;
		
		// read in the data block
		if(ReadDataBlock(i, theDirDescriptors[dirDescriptor].theInode.blockPointers, buff) < 0)
		{
			printf("Error reading data block: %d\n", i);
			return(-1);
		}
		
		curSize = 0;
		
		while(curSize < blockSize)
		{
			DirectoryEntry	tmpEntry;
			
			tmpDir = reinterpret_cast<DirEntry *>(&buff[curSize]);
			
			if(tmpDir->inode != 0)
			{
				Inode	tmpInode;
				
				ReadInode(tmpDir->inode, &tmpInode);	// read in the inode to get the file's info
				
				tmpEntry.name = string(tmpDir->name, tmpDir->nameLength);	// copy over the name
				tmpEntry.size = tmpInode.size;		// get the file's size
				tmpEntry.fileType = tmpDir->fileType;
				
				theEntries.push_back(tmpEntry);
			}
				
			curSize += tmpDir->recordLength;
		}
	}
	
	delete [] buff;
	
	return(0);
}

void ext2::CloseDirectory(int dirDescriptor)
{
	if(static_cast<uint>(dirDescriptor) > theDirDescriptors.size())
	{
		PANIC("Out of bounds CloseDirectory call\n");	
	}
	
	vector<DirDescriptor>::iterator it = theDirDescriptors.begin();
	
	// remove from the vector
	theDirDescriptors.erase(it + dirDescriptor);
}
*/

int ext2::Open(FileDescriptorBase *fileDescriptor, const string &file, const int flags)
{
	// FIX ME
	(void)flags;
	
	FileDescriptor	*fd = reinterpret_cast<FileDescriptor*>(fileDescriptor);
	
	int inode = FindInodeByPath(string(file));
	
	if(inode < 0)
	{
		WARN("COULDN'T FIND INODE\n");
		return(-1);
	}

	fd->inodeNumber = inode;		// set the inode number
	ReadInode(inode, &fd->fileInode);	// read in the inode

/*	printf("SIZE: %d\n", fd->fileInode.size);
	printf("OWNER ID: %d\n", fd->fileInode.ownerUID);
	printf("FILE MODE: %d\n", fd->fileInode.fileMode);
	printf("BLOCK COUNT: %d\n", fd->fileInode.blockCount);
	printf("FILE FLAGS: %d\n", fd->fileInode.fileFlags);
*/
	
	if(!(fd->fileInode.fileMode & FILE_FILE_MODE))
	{
		WARN("Not a file\n");
		return(-2);
	}
	
	// read in the first data block
	if(fd->fileInode.size > 0)
	{
		fd->blockData = new uchar[blockSize];	// make the memory
		
		// Read in the first data block
		ReadDataBlock(0, fd->fileInode.blockPointers, fd->blockData);
	}

	else
		fd->blockData = NULL;

	// zero the block number and positions
	fd->blockNumber = fd->filePosition = fd->blockPosition = 0;

	return(0);
}

int ext2::Read(FileDescriptorBase *fileDescriptor, void *buff, uint numBytes)
{	
	if(numBytes == 0)
		return(0);
	
	FileDescriptor	*tmpDescriptor = reinterpret_cast<FileDescriptor*>(fileDescriptor);
	uint		bytesToRead = MIN(tmpDescriptor->fileInode.size - tmpDescriptor->filePosition, numBytes);
	uint		bytesRead = 0;
	int		amt;
	
	// this could be faster by reading directly into buff and saving a mem copy... but easier to code :-)
	while(bytesToRead > bytesRead)
	{
		amt = MIN(blockSize - tmpDescriptor->blockPosition, bytesToRead - bytesRead);
		
		// copy over the data
		MemCopy(reinterpret_cast<uchar*>(buff) + bytesRead, tmpDescriptor->blockData + tmpDescriptor->blockPosition, amt);
		tmpDescriptor->blockPosition += amt;
		tmpDescriptor->filePosition += amt;
		
		// check if we're done with this block and need to read in the next one
		if(tmpDescriptor->blockPosition == blockSize &&
		   tmpDescriptor->filePosition < tmpDescriptor->fileInode.size)
		{
			ReadDataBlock(++tmpDescriptor->blockNumber, tmpDescriptor->fileInode.blockPointers, tmpDescriptor->blockData);
			tmpDescriptor->blockPosition = 0;
		}
		
		bytesRead += amt;
	}

	return(bytesRead);
}


int ext2::Write(FileDescriptorBase *fileDescriptor, void *buff, uint numBytes)
{
	if(numBytes == 0)
		return(0);
	
	FileDescriptor	*tmpDescriptor = reinterpret_cast<FileDescriptor*>(fileDescriptor);
	uint		bytesWritten = 0;
	int		amt;
	
	while(numBytes > bytesWritten)
	{
		amt = MIN(blockSize - tmpDescriptor->blockPosition, numBytes - bytesWritten);

		// copy in the current block
		MemCopy(tmpDescriptor->blockData + tmpDescriptor->blockPosition, reinterpret_cast<uchar*>(buff) + bytesWritten, amt);
		
		// write this block to the disk
		WriteDataBlock(tmpDescriptor->blockNumber, tmpDescriptor, tmpDescriptor->blockData);
		
		// update the positions
		tmpDescriptor->blockPosition += amt;
		tmpDescriptor->filePosition += amt;
		
		if(tmpDescriptor->filePosition > tmpDescriptor->fileInode.size)
			tmpDescriptor->fileInode.size = tmpDescriptor->filePosition;
		
		// we're done with this block and their's another to read in
		if(tmpDescriptor->blockPosition == blockSize &&
		   tmpDescriptor->filePosition < tmpDescriptor->fileInode.size)
		{
			ReadDataBlock(++tmpDescriptor->blockNumber, tmpDescriptor->fileInode.blockPointers, tmpDescriptor->blockData);
			tmpDescriptor->blockPosition = 0;
		}
		
		// we're at the end of the block and the file
		else if(tmpDescriptor->blockPosition == blockSize &&
			tmpDescriptor->filePosition == tmpDescriptor->fileInode.size)
		{
			MemSet(tmpDescriptor->blockData, 0, blockSize);	// set the block to all zeros
			tmpDescriptor->blockPosition = 0;	// reset the block position
			++tmpDescriptor->blockNumber;		// increase the block number
		}

		bytesWritten += amt;
	}
	
	return bytesWritten;
}

int ext2::Seek(FileDescriptorBase *fileDescriptor, int offset, int whence)
{
	if(whence == SEEK_SET && offset < 0)
		return(-2);
	
	if(whence == SEEK_END && offset > 0)
		return(-3);
	
	FileDescriptor	*tmpFd = reinterpret_cast<FileDescriptor*>(fileDescriptor);
	int		fileSize = tmpFd->fileInode.size;
	
	// based on the whence, calculate the new file position
	switch(whence)
	{
		case SEEK_SET:	// we know that offset is positive
			tmpFd->filePosition = MIN(offset, fileSize);
			break;
			
		case SEEK_CUR:
			// calculate the new file position
			tmpFd->filePosition = MIN(int(tmpFd->filePosition + offset), fileSize) < 0 ? \
						0 : MIN(int(tmpFd->filePosition + offset), fileSize);
			break;
			
		case SEEK_END:	// we know offset is negative or zero
			tmpFd->filePosition = fileSize + offset < 0 ? 0 : fileSize + offset;
			break;
		
		default:
			return(-4);
	}
			
	// based off the file position, calculate everything else
	tmpFd->blockPosition = tmpFd->filePosition % blockSize;
	tmpFd->blockNumber = tmpFd->filePosition / blockSize;
			
	// bring in the right data block (this could be a wasted call... oh well)
	ReadDataBlock(tmpFd->blockNumber, tmpFd->fileInode.blockPointers, tmpFd->blockData);
			
	return(tmpFd->filePosition);
}

void ext2::Close(FileDescriptorBase* fileDescriptor)
{
	FileDescriptor	*tmpFd = reinterpret_cast<FileDescriptor*>(fileDescriptor);
	
	// write the file's iNode back to disk
	// NEED TO CHANGE THIS SO WE ONLY WRITE IT BACK ON A WRITE
//	WriteInode(tmpFd->inodeNumber, &tmpFd->fileInode);
	
	// free up the iNode
	delete [] tmpFd->blockData;
}
				

