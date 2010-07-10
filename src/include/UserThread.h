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


/** @file UserThread.h
 *
 */

#ifndef USERTHREAD_H
#define USERTHREAD_H


#include <constants.h>
#include <types.h>
#include <Thread.h>


/** @class UserThread
 *
 * @brief A thread that runs in Ring 3.
 *
 **/

class UserThread : public Thread
{
	friend class ProcessManager;
public:
	/**
	 * Creates a user thread for a given process.
	 * @param functionAddress The address of the function to run.
	 * @param arg A pointer to the arguments for this thread.
	 * @param procID The process this thread is attached to.
	 * @param stackSize The size of the stack for this thread.
	 */
	UserThread(ThreadFunction functionAddress,
		   void *arg,
		   int procID,
		   ulong stackSize,
		   Registers *regs = NULL);
	
	/**
	 * Copy constructor.
	 * @param arg The UserThread to copy.
	 */
	UserThread(const UserThread &arg);
	
	/**
	 * Assignment operator.
	 * @param right The UserThread to copy.
	 * @return A reference to the newly created UserThread.
	 */
	UserThread &operator=(const UserThread &right);
	
private:
	struct	StackLayout
	{
		ulong	gs;
		ulong	fs;
		ulong	es;
		ulong	ds;
		ulong	edi;
		ulong	esi;
		ulong	ebp;
		ulong	esp;
		ulong	ebx;
		ulong	edx;
		ulong	ecx;
		ulong	eax;
		ulong	eip;
		ulong	cs;
		ulong	eflags;
		ulong	esp3;
		ulong	ss3;
	};
};


#endif // UserThread.h


