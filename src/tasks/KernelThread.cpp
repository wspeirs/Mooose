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


/** @file KernelThread.cpp
 *
 */

#include <constants.h>
#include <types.h>
#include <KernelThread.h>
#include <AutoDisable.h>

#include <screen_utils.h>

KernelThread::KernelThread(ThreadFunction functionAddress, void *arg, ulong stackSize, Registers *regs)
	: Thread(functionAddress, arg, stackSize)	// call the base constructor
{
	AutoDisable	lock;
		
	// get a stack layout pointer
	StackLayout *stk = reinterpret_cast<StackLayout*>(espReg - sizeof(StackLayout));
	
	stk->gs = 0x10;
	stk->fs = 0x10;
	stk->es = 0x10;
	stk->ds = 0x10;
	stk->edi = regs == NULL ? 0 : regs->edi;
	stk->esi = regs == NULL ? 0 : regs->esi;
	stk->ebp = regs == NULL ? 0 : regs->ebp;
	stk->esp = regs == NULL ? 0 : regs->esp;
	stk->ebx = regs == NULL ? 0 : regs->ebx;
	stk->edx = regs == NULL ? 0 : regs->edx;
	stk->ecx = regs == NULL ? 0 : regs->ecx;
	stk->eax = regs == NULL ? 0 : regs->eax;
	stk->eip = reinterpret_cast<ulong>(functionAddress);
	stk->cs = 0x08;	// set to kernel code segment
	stk->eflags = 0x00000202;	// set the reserved bit
	
	espReg = reinterpret_cast<ulong>(stk);
	
// 	printf("FUNCTION ADDR: %x\n", functionAddress);
// 	printf("ESP REG: %x (ADDR: %x)\n", espReg, &espReg);
}
