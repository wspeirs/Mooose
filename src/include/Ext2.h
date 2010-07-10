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


/** @file Ext2.h
 *
 */

#ifndef EXT2_H
#define EXT2_H


#include <constants.h>
#include <types.h>
#include <vector.h>
#include <algorithms.h>
#include <list.h>
#include <string.h>

#ifdef UNIT_TEST

#include "harness_code.h"
#include "FileSystemBase.h"

#else

#include <ATADriver.h>
#include <FileSystemBase.h>

#endif

using k_std::vector;
using k_std::list;
using k_std::string;


/** @class ext2
 *
 * @brief A port of the ext2 file system for MOOOSE.
 * 
 * This file system works with Linux, but is extremely inefficient. The allocation scheme is a first-free scheme instead
 * of the 8-block scheme used in Linux. There are no buffer pools so it is extremely memory intensive. For example reading
 * a 1MB file from the disk requires about 1,000 malloc and free calls. Much can be done to improve the speed.
 *
 **/

class ext2 : public FileSystemBase
{
public:
	/**
	 * The ext2 file system constructor.
	 * @param theDrive A block device for which the file system should read and write blocks.
	 * @param baseAddress The base address of the file system on the block device. Address is in terms of blocks.
	 */
	ext2(BlockDevice *theDrive, ulong baseAddress = 0);
	
	/**
	 * Creates a file descriptor for the ext2 file system.
	 * @return A file descriptor for the ext2 file system.
	 */
	FileDescriptorBase *CreateFileDescriptor(FileSystemBase *fsBase);

	/**
	 * Destroys a file descriptor for a given file system.
	 * 
	 * This function needs to delete any memory that was made in CreateFileDescriptor.
	 * @param fd The file descriptor to destroy.
	 */
	void DestroyFileDescriptor(FileDescriptorBase *fd);
	
	/**
	 * Used to open a file on the file system.
	 * @param fileName A string containing the full path to the file.
	 * @param flags The flags used to open the file.
	 * @return A file descriptor index or an error code.
	 */
	int Open(FileDescriptorBase *fileDescriptor, const string &fileName, const int flags);
	
	/**
	 * Used to read data from a file on the file system.
	 * @param fileDescriptor An index into the descriptor table for this file.
	 * @param buff A pointer to memory where the data should be sent.
	 * @param numBytes The number of bytes to read.
	 * @return The number of bytes read or an error code.
	 */
	int Read(FileDescriptorBase *fileDescriptor, void *buff, uint numBytes);
	
	/**
	 * Used to write data to a file on the file system.
	 * @param fileDescriptor An index into the descriptor table for this file.
	 * @param buff A pointer to memory where the data should be read from.
	 * @param numBytes The number of bytes to write.
	 * @return The number of bytes written or an error code.
	 */
	int Write(FileDescriptorBase *fileDescriptor, void *buff, uint numBytes);
	
	/**
	 * Used to seek inside a file.
	 * @param fileDescriptor An index into the descriptor table for this file.
	 * @param offset The number of bytes from whence, positive or negative.
	 * @param whence Where the offset starts from: SEEK_SET, SEEK_CUR, SEEK_END.
	 * @return The number of bytes the file pointer moved.
	 */
	int Seek(FileDescriptorBase *fileDescriptor, int offset, int whence);
	
	/**
	 * Close an open file.
	 * @param fileDescriptor An index into the descriptor table for the file to close.
	 */
	void Close(FileDescriptorBase *fileDescriptor);
	
	/**
	 * Stats a file, via the path.
	 * @param path The path to the file to stat.
	 * @param buffer A pointer to a stat structure.
	 * @return Zero on success and negative on error.
	 */
	int Stat(string path, stat *buffer)
	{
		(void)path;
		(void)buffer;
		return(0);
	}
	
	/**
	 * Stats a file, via file descriptor.
	 * @param fileDescriptor The file descriptor to stat.
	 * @param buffer A pointer to a stat structure.
	 * @return Zero on success and negative on error.
	 */
	int FileStat(FileDescriptorBase* fileDescriptor, stat *buff)
	{
		(void)fileDescriptor;
		(void)buff;
		return(0);
	}
	
	/**
	 * Stats a symbolic link.
	 * @param path The path to the link.
	 * @param buffer A pointer to a stat structure.
	 * @return Zero on success and negative on error.
	 */
	int LinkStat(string path, stat *buffer)
	{
		(void)path;
		(void)buffer;
		return(0);
	}

	/**
	 * Open a directory for reading.
	 * @param path The path to the directory.
	 * @return The success or failure of the open.
	 */
//	int OpenDirectory(const string &path);
	
	/**
	 * Read a directory getting it's contents.
	 * @param dirDescriptor An index into the descriptor table for this directory.
	 * @param theEntries A list where the directory entries will be added. They are added to the end.
	 * @return The success or failure of the read.
	 */
//	int ReadDirectory(int dirDescriptor, list<DirectoryEntry> &theEntries);
	
	/**
	 * Closes an open directory.
	 * @param dirDescriptor An index into the descriptor table for this directory.
	 */
//	void CloseDirectory(int dirDescriptor);
	
private:
	//
	// Constants
	//
	static const ulong	ROOT_INODE = 2;	///< The number of the root inode
	
	static const int	NUM_BLOCK_PTRS		   = 15;	///< The number of block pointers in an inode
	static const int	DIRECT_BLOCK_PTRS	   = 11;	///< The number of direct data blocks (11)
	static const int	INDIRECT_BLOCK_PTR	   = DIRECT_BLOCK_PTRS + 1;	///< The pointer to the indirect block (12)
	static const int	DOUBLE_INDIRECT_BLOCK_PTR  = INDIRECT_BLOCK_PTR + 1;	///< The pointer to the double indirect block (13)
	static const int	TRIPPLE_INDIRECT_BLOCK_PTR = DOUBLE_INDIRECT_BLOCK_PTR + 1; ///< The pointer to the tripple indirect block (14)
	
	static const ulong	DIR_FILE_MODE = 0x4000;
	static const ulong	FILE_FILE_MODE = 0x8000;
	
	//
	// On disk structures
	//
	/**
	 * The structure of the on-disk super block.
	 */
	struct SuperBlock	
	{
		ulong	inodeCount;		///< Inodes count
		ulong	blockCount;		///< Blocks count 
		ulong	reservedBlockCount;	///< Reserved blocks count 
		ulong	freeBlockCount;		///< Free blocks count 
		ulong	freeInodeCount;		///< Free inodes count 
		ulong	firstDataBlock;		///< First Data Block 
		ulong	blockSize;		///< Block size 0 = 1024, 1 = 2048, 2 = 4096
		ulong	fragmentSize;		///< Fragment size 
		ulong	blocksPerGroup;		///< # Blocks per group 
		ulong	fragmentsPerGroup;	///< # Fragments per group 
		ulong	inodesPerGroup;		///< # Inodes per group 
		ulong	mountTime;		///< Mount time 
		ulong	writeTime;		///< Write time 
		ushort	mountCount;		///< Mount count 
		ushort	maxMountCount;		///< Maximal mount count 
		ushort	magicValue;		///< Magic signature 
		ushort	state;			///< File system state 
		ushort	errors;			///< Behaviour when detecting errors 
		ushort	minorRevisionLevel;	///< minor revision level 
		ulong	timeLastChecked;	///< time of last check 
		ulong	checkInterval;		///< max. time between checks 
		ulong	creatorOS;		///< OS 
		ulong	revisionLevel;		///< Revision level 
		ushort	defaultReservedUID;	///< Default uid for reserved blocks 
		ushort	defaultReservedGID;	///< Default gid for reserved blocks 
		ulong	first_ino;		///< First non-reserved inode 
		ushort	inode_size;		///< size of inode structure 
		ushort	block_group_nr;		///< block group # of this superblock 
		ulong	feature_compat;		///< compatible feature set 
		ulong	feature_incompat;	///< incompatible feature set 
		ulong	feature_ro_compat;	///< readonly-compatible feature set 
		uchar	uuid[16];		///< 128-bit uuid for volume 
		char    volume_name[16];	///< volume name 
		char    last_mounted[64];	///< directory where last mounted 
		ulong	algorithm_usage_bitmap;	///< For compression 
	} __attribute__((packed));
	
	/**
	 * The structure of a directory entry found on the disk in a data block.
	 */
	struct DirEntry {
		ulong	inode;			///< Inode number 
		ushort	recordLength;		///< Directory entry length 
		uchar	nameLength;		///< Name length 
		uchar	fileType;		///< 1 = file, 2 = dir, 7 = sym link
		char	name[255];		///< File name 
	} __attribute__((packed));

	/**
	 * The structure of an Inode on the disk.
	 */
	struct Inode {
		ushort	fileMode;		///< File mode 
		ushort	ownerUID;		///< Low 16 bits of Owner Uid 
		ulong	size;			///< Size in bytes 
		ulong	accessTime;		///< Access time 
		ulong	createTime;		///< Creation time 
		ulong	modificationTime;	///< Modification time 
		ulong	deletionTime;		///< Deletion Time 
		ushort	groupID;		///< Low 16 bits of Group Id 
		ushort	linkCount;		///< Links count 
		ulong	blockCount;		///< Blocks count 
		ulong	fileFlags;		///< File flags 
		ulong	reserved1;
		ulong	blockPointers[NUM_BLOCK_PTRS];	///< Pointers to blocks 
		ulong	generation;		///< File version (for NFS) 
		ulong	fileACL;		///< File ACL 
		ulong	directoryACL;		///< Directory ACL 
		ulong	fragmentAddress;	///< Fragment address 
		uchar	fragmentNumber;		///< Fragment number 
		uchar	fragementSize;		///< Fragment size 
		ushort	pad1;
		ushort	uidHighWord;		///< these 2 fields    
		ushort	gidHighWord;		///< were reserved2[0] 
		ulong	reserved2;
	} __attribute__((packed));

	/**
	 * The structure of a group descriptor on the disk.
	 */
	struct GroupDescriptor
	{
		ulong	blockBitmapAddress;	///< address of the block bitmap
		ulong	inodeBitmapAddress;	///< address of the inode bitmap
		ulong	inodeTableAddress;	///< address of the inode table
		ushort	freeBlockCount;		///< the number of free blocks in the group
		ushort	freeInodeCount;		///< the number of free inodes in the group
		ushort	usedDirCount;		///< the number of inodes used for directories
		ushort	padding;
		ulong	reserved[3];
	} __attribute__((packed));
	
	/**
	 * The file descriptor for the ext2 file system.
	 */
	class FileDescriptor : public FileDescriptorBase
	{
		friend class ext2;
	public:
		FileDescriptor(FileSystemBase *ptr) : FileDescriptorBase(ptr)
		{ ; }
			
	private:
		Inode	fileInode;	///< The inode for the file
		uint	inodeNumber;	///< The number of the inode
		uint	filePosition;	///< Where we are in the file
		uint	blockPosition;	///< Where we are in the block
		uint	blockNumber;	///< Which block of the file is loaded into blockData
		uchar	*blockData;	///< Where the current block is stored in memory
	};
	
	struct DirDescriptor
	{
		Inode	theInode;	///< The directory's inode
	};
	
	vector<DirDescriptor>	theDirDescriptors;	///< A vector of directory descriptors
	vector<GroupDescriptor>	theGroupDescriptors;	///< A vector of group descriptors
	
	SuperBlock	theSuperBlock;	///< The in memory super block
	ulong		lbaBaseAddress;	///< This is the base LBA of the files system on the disk
	ulong		blockSize;	///< The size of a file system block
	ulong		addrPerBlock;	///< The number of data block addresses per data block
	

	//
	// Internal functions
	//
	/**
	 * Finds an inode number given a path.
	 * @param path The path to the inode.
	 * @returns The number for the inode.
	 */
	int FindInodeByPath(string path);
	
	/**
	 * Finds an inode number of a file in an inode
	 * @param inode The inode to search for the given name (file or directory)
	 * @param name The name of the item to search for.
	 * @returns The number for the inode for the name.
	 */
	int FindByNameInInode(ulong inode, string name);

	/**
	 * Finds the address of a data block given a data block number and block pointers.
	 * @param blockNumber The data block number who's address we want.
	 * @param blockPointers The block pointers in the inode.
	 * @return The address of the data block.
	 */
	ulong FindAddressByBlockNumber(ulong blockNumber, ulong blockPointers[]);

	/**
	 * Reads an inode off the disk.
	 * @param inode The inode number to read from disk.
	 * @param theInode A pointer to memory to read the inode into.
	 * @returns The return value from the device read.
	 */
	int ReadInode(ulong inode, Inode *theInode);
	
	/**
	 * Reads data blocks for a file from the disk.
	 * @param blockNumber The data block number of the block to read.
	 * @param blocksPointers The block pointers for the file.
	 * @param dest A pointer to memory to read the block into.
	 * @returns The return value from the device read.
	 */
	int ReadDataBlock(ulong blockNumber, ulong blocksPointers[], uchar *dest);

	/**
	 * Writes an inode to the disk.
	 * @param inode The inode number to write to disk.
	 * @param theInode A pointer to memory to read the inode from.
	 * @returns The return value from the device write.
	 */
	int WriteInode(ulong inode, Inode *theInode);
	
	/**
	 * Writes data blocks for a file to the disk.
	 * @param blockNumber The data block number of the block to write.
	 * @param fd The index of the file descriptor.
	 * @param src A pointer to memory to write to the disk.
	 * @returns The return value from the device write.
	 */
	int WriteDataBlock(ulong blockNumber, FileDescriptor *fd, uchar *src);

	/**
	 * Allocates a new block for blockNumber near the other allocated blocks.
	 * <b>This is where the allocated algorithm is implemented.</b>
	 * @param blockNumber The data block number that needs a new block allocated for it.
	 * @param fileInode The file's inode that needs to be updated.
	 * @return The address of the newly allocated block.
	 */
	ulong AllocateNewDataBlock(ulong blockNumber, FileDescriptor *fd);

	//
	// Helper function (SHOULD BE SOME OTHER PLACE, utils/math.cpp maybe?)
	//
	template<typename T>
	inline ulong DivUp(T arg, T divisor) { return( T(arg / divisor) + (arg % divisor == 0 ? 0 : 1)); }
};


#endif // ext2.h


