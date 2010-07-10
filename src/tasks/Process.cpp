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


/** @file Process.cpp
 *
 */

#include <constants.h>
#include <types.h>
#include <AutoDisable.h>
#include <Process.h>
#include <algorithms.h>
#include <ProcessManager.h>
#include <PhysicalMemManager.h>
#include <UserThread.h>
#include <KernelThread.h>

using k_std::find;

Process::Process(Thread **mainThread,
		string name,
		uint procID,
		uint parentID,
		ThreadFunction functionAddress,
		void *arg,
		VirtualConsole *console,
		ulong stackSize)
	: procID(procID), parentID(parentID), name(name), theConsole(console)
{
	AutoDisable	lock;	// lock this down
		
	// Create a page directory for the process
	pageDirAddr = PhysicalMemManager::GetInstancePtr()->CreatePageDirectory(procID);
	
//	printf("PAGE DIR ADDR: 0x%x\n", pageDirAddr);
	
	// create a main thread for this process
	*mainThread = new UserThread(functionAddress,
				     arg,
				     procID,
				     stackSize);
	
	// add the main thread into the list of threads for this process
	theThreads.push_back(*mainThread);
}

Process &Process::operator=(const Process &right)
{
	// copy over all the basic stuff
	procID = right.procID;
	parentID = right.parentID;
	name = right.name;
	theConsole = right.theConsole;
// 	theThreads = right.theThreads;	// the thread copy is a bit harder
	fileDescriptors = right.fileDescriptors;
	
	// the page directory will have to be setup once this process has an ID
	pageDirAddr = 0;
	
	return *this;
}

void Process::AddThread(Thread *theThread)
{
	if(theThread == reinterpret_cast<Thread*>(NULL))
		PANIC("Tried to add a null thread\n");
	
	theThreads.push_back(theThread);
}

void Process::DestroyThread(Thread *theThread)
{
	// search through for the thread
	list<Thread*>::iterator it = find(theThreads.begin(), theThreads.end(), theThread);
	
	// check to see if is in this proc
	if(it == theThreads.end())
		PANIC("Attempted to delete an invalid thread\n");

	// call destroy on the thread
	(*it)->Destroy();
	
	// delete the thread
	delete *it;
	
	// remove from the list
	theThreads.erase(it);
	
	// check if there are no threads, then kill this proc
	if(theThreads.size() == 0)
	{
		ProcessManager::GetInstance().DestroyProcess(procID);
	}	
}

void Process::DestroyProcess()
{
	// go through and call destroy on all the threads
	for(list<Thread*>::iterator it = theThreads.begin(); it != theThreads.end(); ++it)
	{
		(*it)->Destroy();
		delete (*it);
	}
	
	// TODO: We need to notify the parent that we've died
}



