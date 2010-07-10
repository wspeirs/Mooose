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


/** @file Semaphore.h
 *
 */

#ifndef SEMAPHORE_H
#define SEMAPHORE_H


#include <constants.h>
#include <types.h>
#include <list.h>

class Thread;

using k_std::list;

/** @class Semaphore
 *
 * @brief 
 *
 **/

class Semaphore
{
public:
	/**
	 * The default constructor.
	 *
	 * Sets up a semaphore with a default value of 0.
	 */
	Semaphore();
	
	/**
	 * Constructor where you can specify the inital value of count.
	 * @param initialCount The inital value of count.
	 */
	Semaphore(int initialCount);

	/**
	 * Signals a semaphore.
	 * 
	 * Signaling a semaphore will increase it's count and remove a waiting thread.
	 * @return The value of the semaphore.
	 */
	int Signal();
	
	/**
	 * Waits on a semaphore.
	 * 
	 * Waiting on a semaphore will decrease the count and remove the thread from the run queue.
	 * @return The value of the semaphore.
	 */
	int Wait();
	
	/**
	 * Signals all of the waiting threads putting them in the run queue.
	 * 
	 * The count is reset to 0.
	 */
	void SignalAll();
	
	/**
	 * Returns the value of the semaphore.
	 * @return The value of the run queue.
	 */
	int GetValue();
	
private:
	list<Thread *>		waitingThreads;		// the list of waiting threads
	int			count;			// this is the count for the semaphore
};


#endif // Semaphore.h


