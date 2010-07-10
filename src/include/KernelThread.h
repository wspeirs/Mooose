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


/** @file KernelThread.h
 *
 */

#ifndef KERNELTHREAD_H
#define KERNELTHREAD_H


#include <constants.h>
#include <types.h>
#include <Thread.h>


/** @class KernelThread
 *
 * @brief These are threads that run inside of the kernel at Ring 0.
 *
 **/

class KernelThread : public Thread
{
public:
	/**
	 * Creates a kernel thread.
	 * @param functionAddress The address of the function to run.
	 * @param arg A pointer to the arguments for this thread.
	 * @param stackSize The size of the stack for this thread.
	 */
	KernelThread(ThreadFunction functionAddress,
		     void *arg,
		     ulong stackSize,
		     Registers *regs = NULL);

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
	};
};


#endif // KernelThread.h


