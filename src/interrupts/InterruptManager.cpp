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


/** @file InterruptManager.cpp
 *
 */

#include <constants.h>
#include <types.h>
#include <AutoDisable.h>
#include <InterruptManager.h>
#include <PhysicalMemManager.h>
#include <i386.h>
#include <mem_utils.h>
#include <io_utils.h>
#include <algorithms.h>
#include <Debug.h>

using k_std::find_if;

extern InterruptDescriptor idt[];

InterruptManager::InterruptManager()
{
	// install all of the stubs
	InstallStubs();
	
	// install all of the CPU handlers
	NullHandler	*nullHandler = new NullHandler;	// this is never deleted on purpose
	
	// the first 32 are all CPU reserved
	for(uchar i=0; i < NUM_RESERVED_INTERRUPTS; ++i)
	{
		if(i != INT_PAGE_FAULT)
			InstallHandler(nullHandler, i);
	}
 	
	// reprogram the IRQs to interrupts 32 - 47
	outb(0x20, 0x11);
	outb(0xA0, 0x11);
	outb(0x21, 0x20);
	outb(0xA1, 0x28);
	outb(0x21, 0x04);
	outb(0xA1, 0x02);
//	outb(0x21, 0x01);
	outb(0x21, 0x05);
	outb(0xA1, 0x01);
	outb(0x21, 0x0);	// turn on all primary interrups
	outb(0xA1, 0x0);	// turn on all secondary interrupts
}

int InterruptManager::Dispatcher(Registers *regs)
{
	AutoDisable	lock;
	int		ret = 0;
	
	// find the handler
	map<uchar, Handler*>::iterator it = interruptHandlers.find(regs->int_no);
	
	
	if(it != interruptHandlers.end())
	{
		ret = (*it).second->Handle(regs);
	}
	
	else
		PANIC("UNHANDLED FAULT INT: %d\n", regs->int_no);
	
	// it is a driver (BAD WAY TO CHECK THIS RANGE)
	if(regs->int_no >= NUM_RESERVED_INTERRUPTS && regs->int_no <= NUM_RESERVED_INTERRUPTS + NUM_IRQS)
	{
//		printf("Sending EOI\n");
		
		// IRQs 8 - 15 need an EOI sent to the secondary controller
		if(regs->int_no >= 40)
			outb(0xA0, 0x20);
	
		// all IRQs need an EOI sent to the master
		outb(0x20, 0x20);
	}
	
	return(ret);
}


int InterruptManager::InstallHandler(Handler *theHandler, uchar interruptNumber)
{
	// install the handler
	InstallHandlerNoStartup(theHandler, interruptNumber);
	
	// call startup on the new one
	return(theHandler->Startup());
}

void InterruptManager::InstallHandlerNoStartup(Handler *theHandler, uchar interruptNumber)
{
	AutoDisable		lock;
	pair<uchar, Handler*>	tmpPair(interruptNumber, theHandler);
	
	// TODO: Add a check to make sure the pointer is on the heap
	// TODO: Add a check to make sure the number is valid
	
//  	printf("INSTALLING INT: %d 0x%x\n", interruptNumber, theHandler);

	// add the pair to the end of the list
  	interruptHandlers.insert(tmpPair);
}

int InterruptManager::RemoveHandler(uchar interruptNumber)
{
	AutoDisable			lock;
	map<uchar, Handler*>::iterator	it = interruptHandlers.find(interruptNumber);
	
	DEBUG("REMOVING HANDLER: %d\n", interruptNumber);
	
	if(it == interruptHandlers.end())
		PANIC("Tried to remove handler that didn't exist\n");

	// first call shutdown on the handler
	int	ret = (*it).second->Shutdown();
	
	interruptHandlers.erase(it);	// remove the handler pointer
		
	return(ret);
}


void InterruptManager::SetStub(uchar num, ulong addr, uint desPrivLevel, uint present)
{
	MemSet(&idt[num], 0, sizeof(InterruptDescriptor));      // zero out the struct

	// fill in the required stuff
	idt[num].type = 0xE;    // see 5-14, Vol 3
	idt[num].segSelector = 0x8;

	idt[num].funPtrLow = addr & 0xFFFF;
	idt[num].funPtrHigh = addr >> 16;
	idt[num].desPrivLevel = desPrivLevel;
	idt[num].present = present;
}

void InterruptManager::InstallStubs()
{
	// Set the reserved handlers
	SetStub(INT_DIV_ZERO, (unsigned)isr0, 0, 1);
	SetStub(INT_DEBUG, (unsigned)isr1, 0, 1);
	SetStub(INT_NON_MASKABLE, (unsigned)isr2, 0, 1);
	SetStub(INT_BREAKPOINT, (unsigned)isr3, 0, 1);
	SetStub(INT_INTO_OVERFLOW, (unsigned)isr4, 0, 1);
	SetStub(INT_OUT_OF_BOUNDS, (unsigned)isr5, 0, 1);
	SetStub(INT_INVALID_OPCODE, (unsigned)isr6, 0, 1);
	SetStub(INT_NO_COPROCESSOR, (unsigned)isr7, 0, 1);
	SetStub(INT_DOUBLE_FAULT, (unsigned)isr8, 0, 1);
	SetStub(INT_COPROC_SEG_OVERRUN, (unsigned)isr9, 0, 1);
	SetStub(INT_BAD_TSS, (unsigned)isr10, 0, 1);
	SetStub(INT_SEGMENT_FAULT, (unsigned)isr11, 0, 1);
	SetStub(INT_STACK_FAULT, (unsigned)isr12, 0, 1);
	SetStub(INT_GEN_PROTECT_FAULT, (unsigned)isr13, 0, 1);
	SetStub(INT_PAGE_FAULT, (unsigned)isr14, 0, 1);
	SetStub(INT_UNKNOWN, (unsigned)isr15, 0, 1);
	SetStub(INT_COPROCESSOR_FAULT, (unsigned)isr16, 0, 1);
	SetStub(INT_ALIGN_CHECK, (unsigned)isr17, 0, 1);
	SetStub(INT_MACHINE_CHECK, (unsigned)isr18, 0, 1);
/*	SetStub(19, (unsigned)isr19, 0, 1);
	SetStub(20, (unsigned)isr20, 0, 1);
	SetStub(21, (unsigned)isr21, 0, 1);
	SetStub(22, (unsigned)isr22, 0, 1);
	SetStub(23, (unsigned)isr23, 0, 1);
	SetStub(24, (unsigned)isr24, 0, 1);
	SetStub(25, (unsigned)isr25, 0, 1);
	SetStub(26, (unsigned)isr26, 0, 1);
	SetStub(27, (unsigned)isr27, 0, 1);
	SetStub(28, (unsigned)isr28, 0, 1);
	SetStub(29, (unsigned)isr29, 0, 1);
	SetStub(30, (unsigned)isr30, 0, 1);
	SetStub(31, (unsigned)isr31, 0, 1);
*/
	
	// Set the IRQ handlers
	SetStub(IRQ_0, (unsigned)irq0, 0, 1);
	SetStub(IRQ_1, (unsigned)irq1, 0, 1);
	SetStub(IRQ_2, (unsigned)irq2, 0, 1);
	SetStub(IRQ_3, (unsigned)irq3, 0, 1);
	SetStub(IRQ_4, (unsigned)irq4, 0, 1);
	SetStub(IRQ_5, (unsigned)irq5, 0, 1);
	SetStub(IRQ_6, (unsigned)irq6, 0, 1);
	SetStub(IRQ_7, (unsigned)irq7, 0, 1);
	SetStub(IRQ_8, (unsigned)irq8, 0, 1);
	SetStub(IRQ_9, (unsigned)irq9, 0, 1);
	SetStub(IRQ_10, (unsigned)irq10, 0, 1);
	SetStub(IRQ_11, (unsigned)irq11, 0, 1);
	SetStub(IRQ_12, (unsigned)irq12, 0, 1);
	SetStub(IRQ_13, (unsigned)irq13, 0, 1);
	SetStub(IRQ_14, (unsigned)irq14, 0, 1);
	SetStub(IRQ_15, (unsigned)irq15, 0, 1);
	
	// Set the software interrupts
	SetStub(S_IRQ_0, (unsigned)s_irq0, 3, 1);	// make it the same as linux
/*	SetStub(49, (unsigned)s_irq1, 3, 1);
	SetStub(50, (unsigned)s_irq2, 3, 1);
	SetStub(51, (unsigned)s_irq3, 3, 1);
	SetStub(52, (unsigned)s_irq4, 3, 1);
	SetStub(53, (unsigned)s_irq5, 3, 1);
	SetStub(54, (unsigned)s_irq6, 3, 1);
	SetStub(55, (unsigned)s_irq7, 3, 1);
	SetStub(56, (unsigned)s_irq8, 3, 1);
	SetStub(57, (unsigned)s_irq9, 3, 1);
*/
}

extern "C" int fault_handler(Registers *r)
{
// 	printf("INT: %u\n", r->int_no);
	
/*	if(r->int_no < 32)
	{
		printf("INT: %d ERR: 0x%x\n", r->int_no, r->err_code);
		if(r->err_code != 0)
			PANIC("");
	}*/
	
	return(InterruptManager::GetInstance().Dispatcher(r));
}


