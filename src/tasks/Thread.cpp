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


/** @file Thread.cpp
 *
 */

#include <constants.h>
#include <types.h>
#include <AutoDisable.h>
#include <Thread.h>
#include <ProcessManager.h>

// This constructs the basic stack for all threads
Thread::Thread(ThreadFunction functionAddress, void *arg, ulong stackSize)
	: procID(0)
{
	(void)functionAddress;
	AutoDisable	lock;
		
	// first we need to allocate a stack for the thread
	stackMemory = new uchar[stackSize];
	
// 	printf("GOT STACK MEMORY AT: 0x%x\n", stackMemory);
	
	// set the stack pointer to the top of the stack, as the stack grows down
	ulong	*stackPtr = reinterpret_cast<ulong *>(stackMemory + stackSize - sizeof(ulong));
	
	stackEnd = reinterpret_cast<uint>(stackPtr);
	
// 	printf("STACK END: %x\n", stackEnd);
	
	// push the pointer to the struct on the stack
	*--stackPtr = reinterpret_cast<ulong>(arg);
	
	// when this function/thread finishes it returns here
	*--stackPtr = reinterpret_cast<ulong>(ProcessManager::Exit);
	
	// save this value so the other threads can finish their setup
	espReg = reinterpret_cast<ulong>(stackPtr);
}
	
Thread &Thread::operator=(const Thread &right)
{
	AutoDisable	lock;
	
	theList = right.theList;	// we put it in the same queue
	procID = right.procID;		// proc id is the same
	
	//
	// stackMemory, espReg and stackEnd are taken care of in the other copy constructors
	//
	
	// insert ourself into the queue
	theList->push_back(this);
	myLocation = --theList->end();
	
	return *this;
}

void Thread::SetLocation(const list<Thread*> *locList, const list<Thread*>::iterator &loc)
{
	theList = const_cast<list<Thread*> *>(locList);
	myLocation = loc;
}

void Thread::Destroy()
{
	theList->erase(myLocation);	// delete the thread from whatever queue it's in
	
	joiningThreads.SignalAll();	// signal all the threads waiting for this one to finish
	
	delete [] stackMemory;	// free the stack
}

void Thread::Join()
{
	joiningThreads.Wait();	// wait until the thread dies
}

