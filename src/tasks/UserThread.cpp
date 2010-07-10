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


/** @file UserThread.cpp
 *
 */

#include <constants.h>
#include <types.h>
#include <UserThread.h>
#include <AutoDisable.h>
#include <mem_utils.h>
#include <screen_utils.h>


UserThread::UserThread(ThreadFunction functionAddress, void *arg, int procID, ulong stackSize, Registers *regs)
	: Thread(functionAddress, arg, stackSize)	// call the base constructor
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
	stk->eip = reinterpret_cast<ulong>(functionAddress);
	stk->cs = 0x1B;	// set to USER code segment
	stk->eflags = 0x00000202;	// set the reserved bit
	stk->esp3 = 0xC0000000 - 16;	// set the stack to 3 GB mark - 16 bytes so that libc can setup the stack properly
	stk->ss3 = 0x23;
	
	espReg = reinterpret_cast<ulong>(stk);
			
//	printf("ESP REG: %x (ADDR: %x)\n", espReg, &espReg);

}

UserThread::UserThread(const UserThread &arg) : Thread(arg)
{
	*this = arg;
}

UserThread &UserThread::operator=(const UserThread &right)
{
	uint	stackSize = reinterpret_cast<uchar*>(right.stackEnd) - right.stackMemory + sizeof(ulong);
	
	// need to make a copy of the stack
	stackMemory = new uchar[stackSize];
	
	MemCopy(stackMemory, right.stackMemory, stackSize); // prob don't need to copy all of this
	
	// calculate the end of the stack
	stackEnd = reinterpret_cast<uint>(stackMemory + stackSize - sizeof(ulong));
	
	espReg = reinterpret_cast<ulong>(stackMemory + stackSize - sizeof(ulong) - sizeof(StackLayout));
	
	StackLayout *stk = reinterpret_cast<StackLayout*>(espReg);
	
	stk->eax = 0;	// this is what is returned from fork()
	
/*	printf("GS: 0x%x\n", stk->gs);
	printf("EBP: 0x%x\n", stk->ebp);
	printf("ESP: 0x%x\n", stk->esp);
	printf("EIP: 0x%x\n", stk->eip);
	printf("CS: 0x%x\n", stk->cs);
	printf("EFLAGS: 0x%x\n", stk->eflags);
	printf("ESP3: 0x%x\n", stk->esp3);
	printf("SS3: 0x%x\n", stk->ss3);
	*/
/*	printf("OLD STACK: 0x%x -> 0x%x\n", right.stackMemory, right.stackMemory + stackSize);
	printf("NEW STACK: 0x%x -> 0x%x\n", stackMemory, stackMemory + stackSize);
	
*/
	return *this;
}
