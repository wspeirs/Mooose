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


/** @file Semaphore.cpp
 *
 */

#include <constants.h>
#include <types.h>
#include <AutoDisable.h>
#include <Semaphore.h>
#include <ProcessManager.h>


Semaphore::Semaphore()
{
	count = 0;
}

Semaphore::Semaphore(int initialCount)
{
	count = initialCount;
}

int Semaphore::Signal()
{
	AutoDisable		lock;
	ProcessManager		&theProcessManager = ProcessManager::GetInstance();	// the process manager
	
	// increase the count and see if greater then zero
	// also check to see if anyone is waiting on this semaphore
	if(++count > 0 && waitingThreads.size() > 0)
	{
		// we have just freed up a slot, so move a waiting thread over to the run queue
		
		theProcessManager.runQueue.push_back(waitingThreads.front());	// add to the running threads
		waitingThreads.pop_front();	// remove it from the waiting list
	}
		
	return(count);
}

int Semaphore::Wait()
{
	AutoDisable		lock;
	ProcessManager		&theProcessManager = ProcessManager::GetInstance();	// the process manager
	
	
	if(count <= 0)	// we must wait for the count to be raised
	{
		waitingThreads.push_back(*(theProcessManager.curThreadIterator));	// add this thread to the end of the waiting list
		
		theProcessManager.curThreadIterator = theProcessManager.runQueue.erase(theProcessManager.curThreadIterator);	// remove from the run queue
		
		if(theProcessManager.curThreadIterator == theProcessManager.runQueue.end())
			theProcessManager.curThreadIterator = theProcessManager.runQueue.begin();
		
		theProcessManager.PerformTaskSwitch();	// get another process to run
	}
	
	// we have either come back from scheduling or the count < 0 from the start
	--count;	// decrease the count;
	
	return(count);	
}

void Semaphore::SignalAll()
{
	AutoDisable	lock;
	ProcessManager	&theProcessManager = ProcessManager::GetInstance();
	
	// simply take all of them threads in the wait queue and return them to the run queue
	for(list<Thread *>::iterator it = waitingThreads.begin(); it != waitingThreads.end(); ++it)
		theProcessManager.runQueue.push_back(*it);
	
	waitingThreads.clear();	// clear the list
	count = 0;		// reset the count
}

int Semaphore::GetValue()
{
	int		ret;
	AutoDisable	lock;
	
	ret = count;	// copy the count
	
	return(ret);
}
