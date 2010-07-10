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


/** @file FileSystemBase.h
 *
 */

#ifndef FILESYSTEMBASE_H
#define FILESYSTEMBASE_H


#include <constants.h>
#include <types.h>
#include <string.h>
#include <list.h>

#ifdef UNIT_TEST

#include "harness_code.h"

#else

#include <Devices.h>
#include <Debug.h>

#endif

using k_std::list;
using k_std::string;

class FileDescriptorBase;	// forward dec

/** @class FileSystemBase
 *
 * @brief This is the base class for all file systems.
 * 
 * It provides the interface between the file system and the block device used to access the file system.
 * It also provides a commone interface with all file systems used.
 *
 **/
class FileSystemBase
{
public:
	/**
	 * This is the POSIX conforming structure for directory information.
	 */
	struct dirent
	{
		uint	d_ino;		///< inode number
		uint	d_off;		///< offset to the next dirent
		ushort	d_reclen;	///< length of this record
		uchar	d_type;		///< type of file
		char	d_name[256];	///< filename
	};
	
	/**
	 * This is the POSIX conforming structure for calling stat.
	 */
	struct stat
	{
		uint	deviceID;	///< ID of device containing file
		uint	inodeNumber;	///< inode number
		uint	mode;		///< protection
		uint	hardLinkCount;	///< number of hard links
		uint	uid;		///< user ID of owner
		uint	gid;		///< group ID of owner
		uint	deviceID2;	///< device ID (if special file)
		uint	fileSize;	///< total size, in bytes
		uint	blockSize;	///< blocksize for filesystem I/O
		uint	blockCount;	///< number of blocks allocated
		uint	accessTime;	///< time of last access
		uint	modifyTime;	///< time of last modification
		uint	changeTime;	///< time of last status change
	};



	/**
	 * The file system base constructor.
	 * @param blockDevice A block device to read blocks from.
	 * @param startAddr The offset, in device blocks, to start reads from.
	 */
	FileSystemBase(BlockDevice *blockDevice, ulong startAddr)
		: device(blockDevice), offset(startAddr), blockMultiplier(1)
	{ ; }		
	
	/**
	 * Creates a file descriptor for a given file system and returns a pointer to it.
	 * @return A file descriptor for the file system.
	 */
	virtual FileDescriptorBase *CreateFileDescriptor(FileSystemBase *fsBase) = 0;

	/**
	 * Destroys a file descriptor for a given file system.
	 * 
	 * This function needs to delete any memory that was made in CreateFileDescriptor.
	 * @param fd The file descriptor to destroy.
	 */
	virtual void DestroyFileDescriptor(FileDescriptorBase *fd) = 0;

	/**
	 * Sets the file system's block size.
	 * @param blockSize The file system's block size in bytes.
	 */
	void SetFileSystemBlockSize(ulong blockSize)
	{
		if(blockSize < device->GetBlockSize() ||
		   blockSize % device->GetBlockSize() != 0)
		{
		   	DEBUG("FS BLOCK SIZE: %d\n", blockSize);
			DEBUG("DEVICE BLOCK SIZE: %d\n", device->GetBlockSize());
		   	PANIC("ERROR IN BLOCK SIZES\n");
		}
		
		blockMultiplier = blockSize / device->GetBlockSize();
	}
	
	/**
	 * Reads blocks off the disk.
	 * @param blockNumber The number of the first file system block to read.
	 * @param numBlocks The number of file system blocks to read.
	 * @param dest A pointer to memory to read the data into.
	 * @returns The return value from the device read.
	 */
	int ReadBlocks(ulong blockNumber, ulong numBlocks, uchar *dest)
	{
		return device->ReadBlocks(offset + (blockNumber * blockMultiplier),
					numBlocks * blockMultiplier,
					dest);
	}

	/**
	 * Writes blocks to the disk.
	 * @param blockNumber The number of the first file system block to write.
	 * @param numBlocks The number of file system blocks to write.
	 * @param src A pointer to memory to write the data from.
	 * @returns The return value from the device write.
	 */
	int WriteBlocks(ulong blockNumber, ulong numBlocks, uchar *src)
	{
		return device->WriteBlocks(offset + (blockNumber * blockMultiplier),
					 numBlocks * blockMultiplier,
					 src);
	}

	//
	// system call interfaces
	//
	virtual int Open(FileDescriptorBase* fd, const string &fileName, const int flags) = 0;
	virtual int Read(FileDescriptorBase* fileDescriptor, void *buff, uint numBytes) = 0;
	virtual int Write(FileDescriptorBase* fileDescriptor, void *buff, uint numBytes) = 0;
	virtual int Seek(FileDescriptorBase* fileDescriptor, int offset, int whence) = 0;
	virtual void Close(FileDescriptorBase* fileDescriptor) = 0;
	virtual int Stat(string path, stat *buffer) = 0;
	virtual int FileStat(FileDescriptorBase* fileDescriptor, stat *buff) = 0;
	virtual int LinkStat(string path, stat *buffer) = 0;
	
	virtual ~FileSystemBase()
	{
		delete device;
	}
	
#ifndef UNIT_TEST	
	// seek constants
	enum { SEEK_SET, SEEK_CUR, SEEK_END };
#endif
	
	// type constants
	enum { FILE = 1, DIRECTORY = 2, SYM_LINK = 7 };
	
protected:
	BlockDevice	*device;
	ulong		offset;
	ulong		blockMultiplier;
};

/**
 * @class FileDescriptorBase
 * The base class for all file system's file descriptors.
 */
class FileDescriptorBase
{
public:
	/**
	 * The only constructor for the file descriptor base.
	 * @param fsBase A pointer to the file system base.
	 */
	FileDescriptorBase(FileSystemBase *fsBase) : fileSystem(fsBase)
	{ ; }
	
	virtual ~FileDescriptorBase() { ; }
	
	/**
	 * Returns a pointer to the file system this descriptor belongs to.
	 * @return The file system pointer.
	 */
	inline FileSystemBase *GetFileSystem()
	{ return fileSystem; }
	
private:
	FileSystemBase	*fileSystem; ///< A pointer to the file system this descriptor belongs to, so it can be found.
};


class FileSystemFactory
{
public:
	virtual FileSystemBase *CreateFileSystem(BlockDevice *blockDevice, ulong startAddr) = 0;
	virtual ~FileSystemFactory() { ; }
};

template<typename T>
class FileSystemFactoryT : public FileSystemFactory
{
public:
	/**
	 * Creates an instance of the given file system and returns it.
	 */
	T *CreateFileSystem(BlockDevice *blockDevice, ulong startAddr)
	{ return new T(blockDevice, startAddr);	}
};

#endif // FileSystemBase.h


