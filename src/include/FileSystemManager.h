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


/** @file FileSystemManager.h
 *
 */

#ifndef FILESYSTEMMANAGER_H
#define FILESYSTEMMANAGER_H

#include <constants.h>
#include <types.h>
#include <string.h>
#include <map.h>
#include <syscalls.h>

#ifdef UNIT_TEST

#include "Singleton.h"
#include "FileSystemBase.h"
#include "ProcessManager.h"

#else

#include <Singleton.h>
#include <FileSystemBase.h>
#include <ProcessManager.h>

#endif

using k_std::string;
using k_std::map;
using k_std::pair;

/** @class FileSystemManager
 *
 * @brief 
 *
 **/

class FileSystemManager : public Singleton<FileSystemManager>
{
public:
	FileSystemManager();
	
	/**
	 * Registers a file system type with the OS.
	 * 
	 * The RemoveFileSystem call will cleanup any memory created.
	 * These two calls should be used like this: 
	 * <br>InstallFileSystem(string("ext2"), new FileSystemFactoryT&lt;ext2&gt;)
	 * <br>RemoveFileSystem(string("ext2")) where this will call <b>delete</b> on the factory.
	 * @param fileSystemType A string describing the type.
	 * @param theFactory A pointer to the file system factory that will return a new file system instance.
	 */
	void InstallFileSystem(string fileSystemType, FileSystemFactory *theFactory);
	
	/**
	 * Removes a file system type with the OS.
	 * 
	 * This call will cleanup any memory made in the InstallFileSystem call.
	 * These two calls should be used like this: 
	 * <br>InstallFileSystem(string("ext2"), new FileSystemFactor&lt;ext2&gt;)
	 * <br>RemoveFileSystem(string("ext2")) where this will call <b>delete</b> on the factory.
	 * @param fileSystemType A string describing the type.
	 */
	void RemoveFileSystem(string fileSystemType);
	
	/**
	 * Mounts a file system in the OS. (SYSCALL_mount)
	 * @param source A path to a device or a dummy value
	 * @param target The directory to mount the filesystem
	 * @param fileSystemType A string representing the filesystem
	 * @param mountFlags Flags for mounting the file system
	 * @param data A pointer to filesystem specific data
	 * @return Zero on success and -1 on error with errno set.
	 */
	static int MountFileSystem(const char *source, const char *target, const char *fileSystemType, ulong mountFlags, const void *data);
	
	/**
	 * Unmounts a file system in the OS. (SYSCALL_umount)
	 * @param target The directory of the mounted filesystem
	 * @return Zero on success and -1 on error with errno set.
	 */
	static int UnmountFileSystem(const char *target);
	
	/**
	 * Opens a file on a file system. (SYSCALL_open)
	 * @param path The path to the file to open.
	 * @param flags The flags to pass to the file system for the open call.
	 * @return A file descriptor index or an error.
	 */
	static int Open(const char *path, const int flags);
	
	/**
	 * Opens a file on the files sytem and returns the descriptor
	 * This function is for internal use only
	 * @param path The path to the file to open.
	 * @param flags The flags to pass to the file system for the open call.
	 * @return A file descriptor pointer or null.
	 */
	FileDescriptorBase *kOpen(const string &path, const int flags);

	/**
	 * Reads from a file in a file system. (SYSCALL_read)
	 * @param fileDescriptor The file descriptor for the open file.
	 * @param buff The buffer to read the data into.
	 * @param numBytes The number of bytes to read.
	 * @return The number of bytes read or an error.
	 */
	static int Read(int fileDescriptor, void *buff, uint numBytes);

	/**
	 * Writes to a file in a file system. (SYSCALL_write)
	 * @param fileDescriptor The file descriptor for the open file.
	 * @param buff The buffer to write the data from.
	 * @param numBytes The number of bytes to write.
	 * @return The number of bytes written or an error.
	 */
	static int Write(int fileDescriptor, void *buff, uint numBytes);
	
	/**
	 * Seeks to a position in a file in a file system. (SYSCALL_lseek)
	 * @param fileDescriptor The file descriptor for the open file.
	 * @param offset The offset from whence to seek to.
	 * @param whence The position to start the seek at.
	 * @return The number of bytes the file position moved or an error.
	 */
	static int Seek(int fileDescriptor, int offset, int whence);
	
	/**
	 * Closes an open file. (SYSCALL_close)
	 * @param fileDescriptor The file descriptor for the open file.
	 */
	static void Close(int fileDescriptor);
	
	/**
	 * Closes an open file.
	 * This function is for internal use only.
	 * @param fileDescriptor A pointer to the file descriptor of the open file.
	 */
	void kClose(FileDescriptorBase *fd);
	
private:
	map<string, FileSystemBase*>		mountPoints;	///< A map of the mounted file systems, (mountPoint, FileSystemBase)
	map<string, FileSystemFactory*>		fileSystems;	///< A map of the known files systems, (fsName, FileSystemFactory)

	/**
	 * A comparitor used in Open to compare the path to the mount points.
	 */
	struct ComparePrefix
	{
		ComparePrefix(const string &p) : path(p) { ; }
		
		bool operator()(const pair<string, FileSystemBase*> &arg)
		{ return path.BeginsWith(arg.first); }
	private:
		string path;
	};
};


#endif // FileSystemManager.h


