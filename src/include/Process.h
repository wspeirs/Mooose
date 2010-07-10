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


/** @file Process.h
 *
 */

#ifndef PROCESS_H
#define PROCESS_H


#include <constants.h>
#include <types.h>
#include <list.h>
#include <vector.h>
#include <string.h>
#include <Thread.h>
#include <VirtualConsole.h>
#include <i386.h>
#include <FileSystemBase.h>

using k_std::list;
using k_std::vector;
using k_std::string;


/** @class Process
 *
 * @brief This is a process. It simply holds meta information about a group of threads.
 *
 **/

class Process
{
	friend class ProcessManager;
	friend void ThreadReturn();

private:
	/**
	 * Creates a process and a main thread for that process.
	 * @param mainThread Fills in a pointer to the main thread.
	 * @param name The name of the process.
	 * @param procID The ID of the process.
	 * @param parentID The ID of the parent process.
	 * @param functionAddress The address of the function to run in the main thread.
	 * @param arg A pointer to the arguments for the thread.
	 * @param console A pointer to the virtual console for this process.
	 * @param stackSize The size of the stack for this thread.
	*/
	Process(Thread **mainThread,
		string name,
		uint procID,
		uint parentID,
		ThreadFunction functionAddress,
		void *arg,
		VirtualConsole *console,
		ulong stackSize);
	
	/**
	 * The copy constructor for the process.
	 * A call to the assignment operator is made, see it's documentation.
	 * @param arg The process to copy.
	 */
	Process(const Process &arg)
	{ *this = arg; }
	
	/**
	 * The assignment operator.
	 * This is basically fork, it makes a copy of everything in the process but creates
	 * a new virtual memory space for the process.
	 * @param right The right side of the equal
	 * @return A reference to the newly created process.
	 */
	Process &operator=(const Process &right);
	
	/**
	 * Add a thread to the process.
	 * @param theThread A pointer to the thread to add.
	 */
	void AddThread(Thread *theThread);
	
	/**
	 * Destroys a thread given a pointer to it.
	 * @param theThread A pointer to the thread to distroy.
	 */
	void DestroyThread(Thread *theThread);
	
	/**
	 * Destroy the process by destroying all of the threads.
	 */
	void DestroyProcess();
		
	uint			procID;		///< The ID of the process
	uint			parentID;	///< The ID of the parent process
	string			name;		///< The name of the process
	VirtualConsole		*theConsole;	///< The console associated with the process
	ulong			pageDirAddr;	///< The physical address of the page dir
	
	list<Thread*>			theThreads;	///< A list of the threads for the process
	vector<FileDescriptorBase*>	fileDescriptors; ///< A list of the file descriptors
};


#endif // Process.h


