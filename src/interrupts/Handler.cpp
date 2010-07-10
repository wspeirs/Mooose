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


/** @file Handler.cpp
 *
 */

#include <constants.h>
#include <types.h>
#include <Handler.h>
#include <io_utils.h>
#include <screen_utils.h>
#include <mem_utils.h>
#include <i386.h>
#include <ProcessManager.h>
#include <V86Thread.h>
#include <Debug.h>

int Driver::Handle(Registers *regs)	// this can be overwritten, but not advised
{
	IRQSignaled(regs);	// call the "handler"
	
	// IRQs 8 - 15 need an EOI sent to the secondary controller
	// DOUBLE CHECK THAT THIS SHOULD BE COMMENTED OUT!!!
/*	if(regs->int_no >= 40)
		outb(0xA0, 0x20);
	
	// all IRQs need an EOI sent to the master
	outb(0x20, 0x20);
*/
	return(0);
}

NullHandler::NullHandler()	// constructor
{
	exception_messages[0] = "Division By Zero";
	exception_messages[1] = "Debug";
	exception_messages[2] = "Non Maskable Interrupt";
	exception_messages[3] = "Breakpoint Exception";
	exception_messages[4] = "Into Detected Overflow Exception";
	exception_messages[5] = "Out of Bounds Exception";
	exception_messages[6] = "Invalid Opcode Exception";
	exception_messages[7] = "No Coprocessor Exception";
	exception_messages[8] = "Double Fault Exception";
	exception_messages[9] = "Coprocessor Segment Overrun Exception";
	exception_messages[10] = "Bad TSS Exception";
	exception_messages[11] = "Segment Not Present Exception";
	exception_messages[12] = "Stack Fault Exception";
	exception_messages[13] = "General Protection Fault Exception";
	exception_messages[14] = "Page Fault Exception";
	exception_messages[15] = "Unknown Interrupt Exception";
	exception_messages[16] = "Coprocessor Fault Exception";
	exception_messages[17] = "Alignment Check Exception (486+)";
	exception_messages[18] = "Machine Check Exception (Pentium/586+)";
	exception_messages[19] = "Reserved";
}

int NullHandler::Handle(Registers *regs)
{
	if(regs->int_no <=19)
	{
		if(regs->int_no == 13 && regs->eflags & 0x20000) // GPF for V86
		{
			V86Handler(regs);
			return(0);	// so we don't halt
		}
		
		else
			DEBUG("EXCEPTION: %s\n", exception_messages[regs->int_no]);
	}

	else if(regs->int_no > 19 && regs->int_no <= 31)
		DEBUG("EXCEPTION: %s\n", exception_messages[19]);

	else
		DEBUG("EXCEPTION UNKNOWN!!!\n");

	DEBUG("ERROR CODE: %d\n", regs->err_code);

	return(0);
}


int NullDriver::IRQSignaled(Registers *regs)
{
	if(regs->int_no-NUM_RESERVED_INTERRUPTS == 0)
		return(0);

	DEBUG("IRQ %d RAISED WITH NO DRIVER INSTALLED\n", regs->int_no-NUM_RESERVED_INTERRUPTS);

	return(0);
}


void V86Handler(Registers *regs)
{
	// should be in a header some place
	enum V8086_PREFIX {
		PFX_ES		= 0x001,
		PFX_CS		= 0x002,
		PFX_SS		= 0x004,
		PFX_DS		= 0x008,
		PFX_FS		= 0x010,
		PFX_GS		= 0x020,
		PFX_OP32	= 0x040,
		PFX_ADR32	= 0x080,
		PFX_LOCK	= 0x100,
		PFX_REPNE	= 0x200,
		PFX_REP		= 0x400
	};

	ushort	*interruptVectorTable = 0;	// The interrupt table lives at 0x0
	
	union {
		ulong	value;
		uchar	*ptr;
	} instPtr = { FarPointerToLinearAddress(regs->cs, regs->eip) };
	
	union {
		ulong	value;
		ushort	*ptr16;
		ulong	*ptr32;
	} stack = { FarPointerToLinearAddress(regs->ss, regs->useresp) };

	ulong	prefixMask = 0;	// a mask for all the possible prefixes
	bool	isPrefix = true;
	
	V86Thread *curThread = static_cast<V86Thread*>(ProcessManager::GetInstance().GetCurrentThread());
	
	// go through all the valid prefixes
	do {
		switch(instPtr.ptr[0])
		{
		case 0x26:
			prefixMask |= PFX_ES;
			instPtr.value++;
			regs->eip = (regs->eip + 1) & 0xffff;
			break;
		case 0x2e:
			prefixMask |= PFX_CS;
			instPtr.value++;
			regs->eip = (regs->eip + 1) & 0xffff;
			break;
		case 0x36:
			prefixMask |= PFX_SS;
			instPtr.value++;
			regs->eip = (regs->eip + 1) & 0xffff;
			break;
		case 0x3e:
			prefixMask |= PFX_DS;
			instPtr.value++;
			regs->eip = (regs->eip + 1) & 0xffff;
			break;
		case 0x64:
			prefixMask |= PFX_FS;
			instPtr.value++;
			regs->eip = (regs->eip + 1) & 0xffff;
			break;
		case 0x65:
			prefixMask |= PFX_GS;
			instPtr.value++;
			regs->eip = (regs->eip + 1) & 0xffff;
			break;
		case 0x66:	// O32
			prefixMask |= PFX_OP32;
			instPtr.value++;
			regs->eip = (regs->eip + 1) & 0xffff;
			break;
		case 0x67:	// A32
			prefixMask |= PFX_ADR32;
			instPtr.value++;
			regs->eip = (regs->eip + 1) & 0xffff;
			break;
		case 0xf0:
			prefixMask |= PFX_LOCK;
			instPtr.value++;
			regs->eip = (regs->eip + 1) & 0xffff;
			break;
		case 0xf2:
			prefixMask |= PFX_REPNE;
			instPtr.value++;
			regs->eip = (regs->eip + 1) & 0xffff;
			break;
		case 0xf3:
			prefixMask |= PFX_REP;
			instPtr.value++;
			regs->eip = (regs->eip + 1) & 0xffff;
			break;
		default:
			isPrefix = false;
		}
	} while(isPrefix);
	
/*	DEBUG("CS:IP = %x:%x, SS:SP = %x:%x, FLAGS: %x, OP = %x\n", 
		regs->cs, regs->eip,
		regs->ss, regs->useresp,
		regs->eflags,
		instPtr.ptr[0]);*/
		
	// after the prefixes, look at the instruction
	switch(instPtr.ptr[0])
	{
	case 0xcd:	// INT n
// 		DEBUG("GOT INT %d (0x%x)\n", instPtr.ptr[1], instPtr.ptr[1]);
		
		if(instPtr.ptr[1] == S_IRQ_0)	// SYSTEM CALL, assume exit call
		{
// 			printf("GOT AN EXIT: EAX == 0x%x\n", regs->eax);
			ProcessManager &procMan = ProcessManager::GetInstance();
			
			procMan.GetCurrentThread()->Destroy();	// kill the thread
			procMan.PerformTaskSwitch();
		}
		
		else	// we got a valid INT
		{
			// push everything onto the stack for a valid INT
			*--stack.ptr16 = (regs->eflags & 0xFFFF);	// flip the INT Flag
			*--stack.ptr16 = regs->cs & 0xFFFF;
			*--stack.ptr16 = (regs->eip + 2) & 0xFFFF;
			
			regs->useresp = ((regs->useresp & 0xFFFF) - 6) & 0xFFFF;
			
			// flip the virtual IF bit
 			if(curThread->interrupts)
				stack.ptr16[2] |= 0x0200;	// turn on
			else
				stack.ptr16[2] &= ~0x0200;	// turn off
			
			// virtually shut off interrupts
			curThread->interrupts = false;
			
			// setup the call in the vector table
			regs->cs =  interruptVectorTable[instPtr.ptr[1] * 2 + 1];
			regs->eip = interruptVectorTable[instPtr.ptr[1] * 2];
		}
		break;
		
	case 0xcf:	// IRET
		regs->eip	= *stack.ptr16++;
		regs->cs	= *stack.ptr16++;
		regs->eflags	= *stack.ptr16++;
		
		// record in the process what interrupts where on the stack
		curThread->interrupts = ((regs->eflags & 0x00200) != 0);
		
		// force interrupts and vm86 mode enabled
		regs->eflags |= 0x20200;
		regs->useresp = ((regs->useresp & 0xffff) + 6) & 0xffff;
		break;

	case 0xfb:	// STI
		curThread->interrupts = true;
		regs->eip = (regs->eip + 1) & 0xffff;
		break;

	case 0xfa:	// CLI
		curThread->interrupts = false;
		regs->eip = (regs->eip + 1) & 0xffff;
		break;

	case 0xee:	// OUT DX.AL
		outb(regs->edx & 0xffff, regs->eax & 0xff);
		regs->eip = (regs->eip + 1) & 0xffff;
		break;

	case 0xef:	// OUT
		if(prefixMask & PFX_OP32)
			outl(regs->edx & 0xffff, regs->eax);
		
		else
			outw(regs->edx & 0xffff, regs->eax & 0xffff);
		
		regs->eip = (regs->eip + 1) & 0xffff;
		break;

	case 0xed:	//IN DX.eAX
		if(prefixMask & PFX_OP32)
			regs->eax = inl(regs->edx & 0xffff);
		
		else
		{
			regs->eax &= 0xffff0000;
			regs->eax |= inw(regs->edx & 0xffff) & 0xffff;
		}
		
		regs->eip = (regs->eip + 1) & 0xffff;
		break;

	case 0xec:	// IN DX.AL
		regs->eax &= 0xffffff00;
		regs->eax |= inb(regs->edx & 0xffff) & 0xff;
		regs->eip = (regs->eip + 1) & 0xffff;
		break;

	case 0x9c:	// PUSHF
		if (prefixMask & PFX_OP32)
		{
			regs->useresp = ((regs->useresp & 0xffff) - 4) & 0xffff;
	
			*--stack.ptr32 = regs->eflags;
	
			// set eflags on stack to what process thinks it is
			if (curThread->interrupts)
				stack.ptr32[0] |= 0x00200;
			else
				stack.ptr32[0] &= ~0x00200;
		}
		
		else
		{
			regs->useresp = ((regs->useresp & 0xffff) - 2) & 0xffff;
	
			*--stack.ptr16 = regs->eflags & 0xffff;
	
			if (curThread->interrupts)
				stack.ptr16[0] |= 0x00200;
			else
				stack.ptr16[0] &= ~0x00200;
		}
	
		regs->eip = (regs->eip + 1) & 0xffff;
		break;

	case 0x9d:	// POPF
		if (prefixMask & PFX_OP32)
		{
			regs->eflags = 0x20200 | stack.ptr32[0];
 			curThread->interrupts = ((stack.ptr32[0] & 0x00200) != 0);
			regs->useresp = ((regs->useresp & 0xffff) + 4) & 0xffff;
		}
		
		else
		{
			regs->eflags = 0x20200 | stack.ptr16[0];
 			curThread->interrupts = ((stack.ptr16[0] & 0x00200) != 0);
			regs->useresp = ((regs->useresp & 0xffff) + 2) & 0xffff;
		}

		regs->eip = (regs->eip + 1) & 0xffff;
		break;

		
	default:
 		DEBUG("INSTRUCTION: 0x%x\n", instPtr.ptr[0]);
		PANIC("V86 Instruction NOT implemented yet!!!", false);
		break;		
	}
}



