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


/** @file FileSystemManager.cpp
 *
 */

#include <constants.h>
#include <types.h>
#include <algorithms.h>
#include <errno.h>

#ifdef UNIT_TEST

#include "FileSystemManager.h"

#else

#include <FileSystemManager.h>
#include <SystemCallHandler.h>
#include <ATADriver.h>
#include <ATAManager.h>
#include <Debug.h>

#endif

using k_std::find_if;

FileSystemManager::FileSystemManager()
{
	//
	// Install all the system calls with the system call handler
	//
#ifndef UNIT_TEST
	SystemCallHandler	&sysCallHandler = SystemCallHandler::GetInstance();
	
	// the main file system calls
	sysCallHandler.InstallSystemCall(SYSCALL_open, (VoidFunPtr)Open, 2);
	sysCallHandler.InstallSystemCall(SYSCALL_read, (VoidFunPtr)Read, 3);
	sysCallHandler.InstallSystemCall(SYSCALL_write, (VoidFunPtr)Write, 3);
	sysCallHandler.InstallSystemCall(SYSCALL_lseek, (VoidFunPtr)Seek, 3);
	sysCallHandler.InstallSystemCall(SYSCALL_close, (VoidFunPtr)Close, 1);
	
	// mount & unmount
	sysCallHandler.InstallSystemCall(SYSCALL_mount, (VoidFunPtr)MountFileSystem, 5);
	sysCallHandler.InstallSystemCall(SYSCALL_umount, (VoidFunPtr)UnmountFileSystem, 1);
#endif
}


int FileSystemManager::MountFileSystem(const char *source, const char *target, const char *fileSystemType, ulong mountFlags, const void *data)
{
	// FIX ME
	(void) mountFlags;
	(void) data;
	
	FileSystemManager	&fileSysMan = FileSystemManager::GetInstance();
	
	// lookup the file system type to see if we know about it
	map<string, FileSystemFactory* >::iterator it = fileSysMan.fileSystems.find(string(fileSystemType));
	
	if(it == fileSysMan.fileSystems.end())
	{
		DEBUG("Trying to mount a file system the kernel doesn't know about\n");
		return(ENODEV * -1);
	}
	
	// get the device from the manager
	ATADriver	*tmpDriver = ATAManager::GetInstance().GetDeviceByName(string(source));
	
	if(tmpDriver == NULL)
	{
		DEBUG("Couldn't find device: %s\n", source);
		return(ENODEV *-1);
	}
	
	// create a new file system and insert it into the mount points
	fileSysMan.mountPoints.insert(pair<string, FileSystemBase*>(string(target), (*it).second->CreateFileSystem(tmpDriver, 0)));
	
	return(0);
}

int FileSystemManager::UnmountFileSystem(const char *target)
{
	FileSystemManager	&fileSysMan = FileSystemManager::GetInstance();
	
	const string tmp(target);

	map<string, FileSystemBase*>::iterator it = fileSysMan.mountPoints.find(tmp);
	
	// make sure it is in there
	if(it == fileSysMan.mountPoints.end())
	{
		DEBUG("Trying to remove %s that doesn't exist\n", target);
		return(ENODEV * -1);	// NEED TO FIGURE OUT HOW TO DO ERRNO
	}
	
	// delete the file system object
	delete (*it).second;

	// erase file system entry
	fileSysMan.mountPoints.erase(it);

	return(0);
}

void FileSystemManager::InstallFileSystem(string fileSystemType, FileSystemFactory *theFactory)
{
	map<string, FileSystemFactory* >::iterator it = fileSystems.find(fileSystemType);
	
	// make sure it isn't already installed
	if(it != fileSystems.end())
		PANIC("Trying to install %s twice\n", fileSystemType.c_str());
	
	// install the file system into the list
	fileSystems.insert(pair<string, FileSystemFactory* >(fileSystemType, theFactory));
}

void FileSystemManager::RemoveFileSystem(string fileSystemType)
{
	map<string, FileSystemFactory* >::iterator it = fileSystems.find(fileSystemType);
	
	// make sure it isn't already removed
	if(it == fileSystems.end())
		PANIC("Trying to remove %s that doesn't exist\n", fileSystemType.c_str());
	
	// delete the factory
	delete (*it).second;
	
	// remove the file system entry
	fileSystems.erase(it);
}
	
int FileSystemManager::Open(const char *path, const int flags)
{
	FileSystemManager	&fileSysMan = FileSystemManager::GetInstance();
	
	// search through the mount points looking for the one contains this file
	map<string, FileSystemBase*>::iterator it = find_if(fileSysMan.mountPoints.begin(),
							    fileSysMan.mountPoints.end(),
							    ComparePrefix(string(path)));
	// couldn't find this mount point
	if(it == fileSysMan.mountPoints.end())
		return(-1 * EFAULT);	// I think this is the right error

	// create a new file descriptor
	FileDescriptorBase *fd = (*it).second->CreateFileDescriptor((*it).second);
	
	// insert the file descriptor into the process
	int index = ProcessManager::GetInstance().InsertFileDescriptor(fd);
	
	// call the file system's open function
	int ret = (*it).second->Open(fd, string(path), flags);
	
	if(ret < 0)
		(*it).second->DestroyFileDescriptor(fd);
	
	// return the error code or the index into the file descriptor vector
	return(ret < 0 ? ret : index);
}

FileDescriptorBase *FileSystemManager::kOpen(const string &path, const int flags)
{
	// search through the mount points looking for the one contains this file
	map<string, FileSystemBase*>::iterator it = find_if(mountPoints.begin(),
							    mountPoints.end(),
							    ComparePrefix(path));
	// couldn't find this mount point
	if(it == mountPoints.end())
		return NULL;

	// create a new file descriptor
	FileDescriptorBase *fd = (*it).second->CreateFileDescriptor((*it).second);
	
	// call the file system's open function
	int ret = (*it).second->Open(fd, string(path), flags);
	
	if(ret < 0)
		(*it).second->DestroyFileDescriptor(fd);
	
	// return the error code or the file descriptor
	return(ret < 0 ? NULL : fd);
}

int FileSystemManager::Read(int fileDescriptor, void *buff, uint numBytes)
{
	// get the file descriptor pointer
	FileDescriptorBase *fd = ProcessManager::GetInstance().GetFileDescriptor(fileDescriptor);
	
	// return a that this a bad file descriptor
	if(fd == NULL)
		return(-1 * EBADF);
	
	// call the file system's read function and return it's value
	return(fd->GetFileSystem()->Read(fd, buff, numBytes));
}

int FileSystemManager::Write(int fileDescriptor, void *buff, uint numBytes)
{
	//
	// HACK
	//
	if(fileDescriptor == 1)	// stdout
	{
		char	*tmp = reinterpret_cast<char*>(buff);
		
		for(uint i=0; i < numBytes; ++i)
		{
// 			DEBUG("%s", reinterpret_cast<char *>(buff));
			DEBUG("%c", *tmp++);
		}
		
		return(numBytes);
	}
	
	// get the file descriptor pointer
	FileDescriptorBase *fd = ProcessManager::GetInstance().GetFileDescriptor(fileDescriptor);
	
	// return a that this a bad file descriptor
	if(fd == NULL)
		return(-1 * EBADF);
	
	// call the file system's read function and return it's value
	return(fd->GetFileSystem()->Write(fd, buff, numBytes));
}

int FileSystemManager::Seek(int fileDescriptor, int offset, int whence)
{
	// get the file descriptor pointer
	FileDescriptorBase *fd = ProcessManager::GetInstance().GetFileDescriptor(fileDescriptor);
	
	// return a that this a bad file descriptor
	if(fd == NULL)
	{
		DEBUG("BAD FD\n");
		return(-1 * EBADF);
	}
	
	// call the file system's read function and return it's value
	return(fd->GetFileSystem()->Seek(fd, offset, whence));
}

void FileSystemManager::Close(int fileDescriptor)
{
	ProcessManager	&procMan = ProcessManager::GetInstance();
	
	// get the file descriptor pointer
	FileDescriptorBase *fd = procMan.GetFileDescriptor(fileDescriptor);
	
	// return on a bad file descriptor
	if(fd == NULL)
		return;
	
	// call the file system's close
	fd->GetFileSystem()->Close(fd);
	
	// remove the file descriptor from the process
	procMan.RemoveFileDescriptor(fileDescriptor);
	
	// call destory on the file descriptor
	fd->GetFileSystem()->DestroyFileDescriptor(fd);
}

void FileSystemManager::kClose(FileDescriptorBase *fd)
{
	// return on a bad file descriptor
	if(fd == NULL)
		return;
	
	// call the file system's close
	fd->GetFileSystem()->Close(fd);
	
	// call destory on the file descriptor
	fd->GetFileSystem()->DestroyFileDescriptor(fd);
}

