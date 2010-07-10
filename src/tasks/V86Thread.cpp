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


/** @file V86Thread.cpp
 *
 */

#include <constants.h>
#include <types.h>
#include <V86Thread.h>
#include <AutoDisable.h>

V86Thread::V86Thread(ThreadFunction functionAddress, void *arg, ulong stackSize, Registers *regs)
	: Thread(functionAddress, arg, stackSize), interrupts(true)	// call the base constructor
{
	AutoDisable	lock;
		
	this->procID = procID;
	
	// get a stack layout pointer
	StackLayout *stk = reinterpret_cast<StackLayout*>(espReg - sizeof(StackLayout));
	
	stk->gs = 0x23;
	stk->fs = 0x23;
	stk->es = 0x23;
	stk->ds = 0x23;
	stk->edi = regs == NULL ? 0 : regs->edi;
	stk->esi = regs == NULL ? 0 : regs->esi;
	stk->ebp = regs == NULL ? 0 : regs->ebp;
	stk->esp = regs == NULL ? 0 : regs->esp;
	stk->ebx = regs == NULL ? 0 : regs->ebx;
	stk->edx = regs == NULL ? 0 : regs->edx;
	stk->ecx = regs == NULL ? 0 : regs->ecx;
	stk->eax = regs == NULL ? 0 : regs->eax;
	stk->eip = reinterpret_cast<ulong>(functionAddress) & 0x0000FFFF;	// low bits
	stk->cs  = reinterpret_cast<ulong>(functionAddress) >> 16;		// high bits
	stk->eflags = 0x00020202;	// set this as a virtual monitor thread
	stk->esp3 = regs == NULL ? 0x8000 : regs->useresp;	// set the stack 
	stk->ss3 = 0x0;
	stk->v86_es = regs == NULL ? 0x0 : regs->v86_es;
	stk->v86_ds = regs == NULL ? 0x0 : regs->v86_ds;
	stk->v86_fs = regs == NULL ? 0x0 : regs->v86_fs;
	stk->v86_gs = regs == NULL ? 0x0 : regs->v86_gs;
	
	espReg = reinterpret_cast<ulong>(stk);

//	printf("ESP REG: %x (ADDR: %x)\n", espReg, &espReg);

}
