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


/** @file V86Thread.h
 *
 */

#ifndef V86THREAD_H
#define V86THREAD_H


#include <constants.h>
#include <types.h>
#include <Thread.h>


/** @class V86Thread
 *
 * @brief Virtual 86 monitor threads.
 *
 **/

class V86Thread : public Thread
{
public:
	/**
	 * Creates a virtual 86 monitor thread.
	 * @param functionAddress The address of the function to run in the thread.
	 * @param arg A pointer to the arguments for this thread.
	 * @param stackSize The size of the stack for this thread.
	 */
	V86Thread(ThreadFunction functionAddress,
		  void *arg,
		  ulong stackSize,
		  Registers *regs = NULL);

	bool	interrupts;

private:
	struct StackLayout
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
		ulong	v86_es;
		ulong	v86_ds;
		ulong	v86_fs;
		ulong	v86_gs;
	};
};


#endif // V86Thread.h


