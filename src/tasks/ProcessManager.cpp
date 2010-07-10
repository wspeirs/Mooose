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


/** @file ProcessManager.cpp
 *
 */

#include <constants.h>
#include <types.h>
#include <AutoDisable.h>
#include <algorithms.h>
#include <ProcessManager.h>
#include <Debug.h>
#include <VirtualConsole.h>
#include <i386.h>
#include <PhysicalMemManager.h>
#include <syscalls.h>
#include <SystemCallHandler.h>
#include <KernelThread.h>
#include <UserThread.h>
#include <V86Thread.h>

using k_std::find;
using k_std::find_if;

void NullProc(void *arg)
{
	(void)arg;	// so we don't get a warning
/*	ulong	eflags;
	ulong	cs, ds;
	
	while(1)
	{
		asm("cli");	// want the print to be atomic
		
		// get the eflags reg
		asm __volatile__ ("pushf;\n pop %%eax;\nmovl %%eax, %0": "=r" (eflags) : : "%eax");
		asm __volatile__ ("mov %%ds, %0": "=r" (ds) : : );
		asm __volatile__ ("mov %%cs, %0": "=r" (cs) : : );
	
		DEBUG("NULL PROC\n");
		DEBUG("FLAGS: %x\tCS: %x\tDS: %x\n\n", eflags, cs, ds);
		
//		ProcessManager::GetInstance()->PerformTaskSwitch();
		
		asm("sti");
		
		for(int i=0; i < 100000; ++i);
	}
*/
	static ProcessManager &procMan = ProcessManager::GetInstance();
	
	
	while(1)
	{
// 		DEBUG("IN NULL\n");		
		procMan.PerformTaskSwitch();
	}
}


ProcessManager::ProcessManager()
	: curStackPointer(NULL), curProcID(0)
{
	// setup the null process (kernel thread)
	// ** Need to specify the PID or else we have a recursive constructor call **
 	CreateThread(NullProc, NULL, Thread::KERNEL, 0);
	
 	// We need a garbage location to store the "current" stack pointer
 	// so that the first time we context switch the NULL proc will run
	ulong	*tmp = new ulong;
	
 	curStackPointer = tmp;
	
	// setup the thread iterator
	curThreadIterator = runQueue.begin();
	
	// register the system calls
	SystemCallHandler	&sysCallHandler = SystemCallHandler::GetInstance();
	
	sysCallHandler.InstallSystemCall(SYSCALL_exit, (VoidFunPtr)Exit, 1);
	sysCallHandler.InstallSystemCall(SYSCALL_fork, (VoidFunPtr)Fork, 0);
}

int ProcessManager::CreateProcess(string name,
				  uint parentID,
				  ThreadFunction functionAddress,
				  void *arg,
				  VirtualConsole *console,
				  ulong stackSize)
{
	AutoDisable	lock;	// lock us down
	uint		procID;
	
	// find a free slot for this process
	vector<Process*>::iterator it = find(theProcs.begin(),
					       theProcs.end(),
					       reinterpret_cast<Process*>(NULL));
	
	if(it == theProcs.end())	// no space left, add to the end
	{
		procID = theProcs.size();	// the old size is the new proc ID
		theProcs.push_back(reinterpret_cast<Process*>(NULL));	// a blank entry
	}
	
	else	// found a free slot
		procID = it - theProcs.begin();	// calc the index
	
//	DEBUG("PROC ID: %d\n", procID);
	
	Thread	*mainThread;
			
	// call the process constructor to actually create the process and main thread
	theProcs[procID] = new Process(&mainThread,
				       name,
				       procID,
				       parentID,
				       functionAddress,
				       arg,
				       console,
				       stackSize);
			
// 	DEBUG("PAGE DIR ADDR: 0x%x\n", theProcs[procID]->pageDirAddr);
	
	// insert the main thread into the run queue
	runQueue.push_back(mainThread);	// TODO: Insert the thread into a more reasonible position
	
	// set the thread's location in the queues
	mainThread->SetLocation(&runQueue, --runQueue.end());
	
	return(procID);
}


Thread *ProcessManager::CreateThread(ThreadFunction functionAddress,
				     void *arg,
				     uchar type,
				     uint procID,
				     ulong stackSize,
				     Registers *regs)
{
	Thread	*tmpThread = NULL;	// create a pointer for a newly created thread
	
	// create the a new thread based on type
	switch(type)
	{
	case Thread::KERNEL:
		tmpThread = new KernelThread(functionAddress,
					     arg,
					     stackSize,
					     regs);
		break;
		
	case Thread::USER:
		// perform some checks for a user proc
		if(uint(procID) >= theProcs.size())
			PANIC("Error: Trying to add a thread to an invalid proc\n");
	
		if(theProcs[procID] == reinterpret_cast<Process*>(NULL))
			PANIC("Error: Trying to add a thread to an invalid proc\n");
		
		tmpThread = new UserThread(functionAddress,
					   arg,
					   procID,
					   stackSize,
					   regs);
		// add this thread to the process
		theProcs[procID]->AddThread(tmpThread);
	
		break;
			
	case Thread::V86:
		tmpThread = new V86Thread(functionAddress,
					  arg,
					  stackSize,
					  regs);
		break;
		
	default:
		PANIC("Tried to create an unknown thread type\n");
	}
			
	
	// add this thread to the run queue
	runQueue.push_back(tmpThread);	// TODO: Insert the thread into a more reasonible position
	
	// set the thread's location in the queues
	tmpThread->SetLocation(&runQueue, --runQueue.end());
	
	return(tmpThread);
}

void ProcessManager::DestroyProcess(uint procID)
{
	// first check to see if the ID is valid
	if(procID >= theProcs.size())
		PANIC("Tried to destroy a proc ID that isn't valid\n");

	DEBUG("RUN THREADS: %d\n", runQueue.size());
	
	// destroy the process
	theProcs[procID]->DestroyProcess();
	
	// delete the process
	delete theProcs[procID];
	
	// free the slot
	theProcs[procID] = reinterpret_cast<Process*>(NULL);
	
	DEBUG("RUN THREADS: %d\n", runQueue.size());
	
	// free all of the memory used by this proc
	PhysicalMemManager::GetInstancePtr()->FreeProcPages(procID);
	
	// schedule a new process
	PerformTaskSwitch();
}

void ProcessManager::Exit(int status)
{
	(void)status;	// need to do something with this in the future
	
	// we have to do this because we're a static member function
	ProcessManager	&procMan = ProcessManager::GetInstance();
	
	procMan.DestroyProcess(procMan.GetCurrentProcID());
}

uint ProcessManager::Fork(void)
{
	AutoDisable		lock;
	ProcessManager		&procMan = ProcessManager::GetInstance();
	PhysicalMemManager	*physMemMan = PhysicalMemManager::GetInstancePtr();
	
	// make a copy of the process
	// the copy constructor will take care of the basic stuff
	Process		*newProc = new Process(*procMan.theProcs[procMan.curProcID]);
	uint		procID;
	
	// find a free slot for this process
	vector<Process*>::iterator it = find(procMan.theProcs.begin(),
					     procMan.theProcs.end(),
					     reinterpret_cast<Process*>(NULL));
	
	if(it == procMan.theProcs.end())	// no space left, add to the end
	{
		procID = procMan.theProcs.size();	// the old size is the new proc ID
		procMan.theProcs.push_back(reinterpret_cast<Process*>(NULL));	// a blank entry
	}
	
	else	// found a free slot
		procID = it - procMan.theProcs.begin();	// calc the index
	
	DEBUG("FORK, NEW PROC ID: %d\n", procID);
	
	// insert the process into the list
	procMan.theProcs[procID] = newProc;
	
	// set the new proc ID of the process
	newProc->procID = procID;
	
	// now that we have the procID we can copy the page directory
	newProc->pageDirAddr = physMemMan->CopyPageDirectory(procID, procMan.GetProcPageDir(procMan.GetCurrentProcID()));
	
	DEBUG("NEW PAGE DIR ADDR: 0x%x\n", newProc->pageDirAddr);
	
	// make a copy of the current thread
	// this will insert it into the run queue for us
	// we assume the only threads calling fork are user threads
	UserThread *newThread = new UserThread(*reinterpret_cast<UserThread*>(procMan.GetCurrentThread()));
	
	// set the procID on the thread
	newThread->procID = procID;
	
	PANIC("END FORK");
	
	return procID;	// return the new ID
}

extern TaskStateSegment		theTSS;

void ProcessManager::ScheduleNextProcess(ulong **oldESP, ulong **newESP)
{
	AutoDisable	lock;
	
	*oldESP = curStackPointer;
	
//	printf("RUN QUEUE SIZE: %d\n", runQueue.size());
	
//  	printf("OLD PROC ID: %u\n", (*curThreadIterator)->procID);
	
	if(++curThreadIterator == runQueue.end())	// need to loop around
		curThreadIterator = runQueue.begin();
		
//  	printf("NEW PROC ID: %u\n", (*curThreadIterator)->procID);
	
// 	printf("NEW ESP: %x\n", (*curThreadIterator)->espReg);
//	printf("NEW ESP ADDR: %x\n", &((*curThreadIterator)->espReg));
	
	*newESP = curStackPointer = &((*curThreadIterator)->espReg);
	
	if(*oldESP == *newESP)	// same stack... nothing needed
		return;
	
	// set the TSS esp0 field
	theTSS.esp0 = (*curThreadIterator)->stackEnd;
	
	if((*curThreadIterator)->procID != 0)	// only need to do this for user threads
	{
		// swap in the new page dir
		PhysicalMemManager::GetInstancePtr()->SetPageDirectory(theProcs[(*curThreadIterator)->procID]->pageDirAddr);
	}
		// record the proc ID
		curProcID = (*curThreadIterator)->procID;
}

/// This will schedule & actually perform a task switch... only to be called from kernel land
void ProcessManager::PerformTaskSwitch()
{
	ulong	*oldStackPointer, *curStackPointer;
	
	ScheduleNextProcess(&oldStackPointer, &curStackPointer);
	
	asm("cli");
	
// 	printf("PRE: ContextSwitch(%x, %x)\n", oldStackPointer, curStackPointer);	
	ContextSwitch(oldStackPointer, curStackPointer);
// 	printf("POST: ContextSwitch(%x, %x)\n", oldStackPointer, curStackPointer);

	asm("sti");	
}

ulong ProcessManager::GetProcPageDir(uint procID)
{
	if(procID > theProcs.size())
		PANIC("Invalid proc ID: 0x%x   SIZE: %d\n", procID, theProcs.size());
	
	return(theProcs[procID]->pageDirAddr);
}

int ProcessManager::InsertFileDescriptor(FileDescriptorBase* ptr)
{
	AutoDisable	lock;
	
	// get the current process
	Process	*curProc = theProcs[curProcID];
	
	// find an open spot in the descriptor bitmap
	vector<FileDescriptorBase*>::iterator freeSlot = find(curProc->fileDescriptors.begin(),
							      curProc->fileDescriptors.end(),
							      reinterpret_cast<FileDescriptorBase*>(NULL));
	int	index;
	
	if(freeSlot == curProc->fileDescriptors.end())
	{
		index = curProc->fileDescriptors.size();
		curProc->fileDescriptors.push_back(ptr);
	}
	
	else
	{
		index = freeSlot - curProc->fileDescriptors.begin();
		*freeSlot = ptr;
	}
	
	return index;
}

FileDescriptorBase *ProcessManager::GetFileDescriptor(int fd)
{
	AutoDisable	lock;
	
	// get the current process
	Process	*curProc = theProcs[curProcID];
	
	// perform a bounds check
	if(uint(fd) >= curProc->fileDescriptors.size())
		return NULL;
	else
		return curProc->fileDescriptors[fd];
}

void ProcessManager::RemoveFileDescriptor(int fd)
{
	AutoDisable	lock;
	
	// get the current process
	Process	*curProc = theProcs[curProcID];
	
	// perform a bounds check
	if(uint(fd) < curProc->fileDescriptors.size())
		curProc->fileDescriptors[fd] = NULL;
}

