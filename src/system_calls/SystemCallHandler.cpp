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


/** @file SystemCallHandler.cpp
 *
 */

#include <constants.h>
#include <types.h>
#include <SystemCallHandler.h>
#include <Debug.h>
#include <errno.h>

SystemCallHandler::SystemCallHandler()
	: nullCall((VoidFunPtr)SystemCallHandler::NullSystemCall, 0),
	  systemCallTable(MAX_SYS_CALL_NUM, nullCall)
{
	;
}

int SystemCallHandler::Startup()
{
	return(0);
}

int SystemCallHandler::Shutdown()
{
	return(0);
}

int SystemCallHandler::Handle(Registers *regs)
{
	if(regs->eax > systemCallTable.size())
	{
		ERROR("Invalid system call\n");
		regs->eax = EIDRM * -1;
		return EIDRM * -1;
	}
	
 	if(regs->eax != 4)	// 4 is write, so could mess up output
		DEBUG("SYS CALL: %d\n", regs->eax);
	
	// get a copy of the system call information
	SystemCall	tmp(systemCallTable[regs->eax]);
	
	int	ret = 0;
	
	switch(tmp.numArgs)
	{
		case 0:
			ret = reinterpret_cast<Fun0ArgPtr_t>(tmp.addr)();
			break;
			
		case 1:
			ret = reinterpret_cast<Fun1ArgPtr_t>(tmp.addr)(regs->ebx);
			break;
			
		case 2:
			ret = reinterpret_cast<Fun2ArgPtr_t>(tmp.addr)(regs->ebx,
								       regs->ecx);
			break;
			
		case 3:
			ret = reinterpret_cast<Fun3ArgPtr_t>(tmp.addr)(regs->ebx,
								       regs->ecx, 
								       regs->edx);
			break;
			
		case 4:
			ret = reinterpret_cast<Fun4ArgPtr_t>(tmp.addr)(regs->ebx,
								       regs->ecx,
								       regs->edx,
								       regs->esi);
			break;
			
		case 5:
			ret = reinterpret_cast<Fun5ArgPtr_t>(tmp.addr)(regs->ebx, 
								       regs->ecx,
								       regs->edx,
								       regs->esi,
								       regs->edi);
			break;
			
		case 6:
			ret = reinterpret_cast<Fun6ArgPtr_t>(tmp.addr)(regs->ebx, 
								       regs->ecx,
								       regs->edx,
								       regs->esi,
								       regs->edi,
								       regs->ebp);
			break;
			
		default:
			ERROR("Invalid num of args in switch\n");
			break;
	}
		
	regs->eax = ret;	// set the return value
	
	return(ret);
}

void SystemCallHandler::InstallSystemCall(uint num, VoidFunPtr addr, int numArgs)
{
	if(num > systemCallTable.size())
	{
		ERROR("Bad System call number\n");
		return;
	}
	
	if(numArgs > 6)
	{
		ERROR("Number of args too large\n");
		return;
	}
	
	SystemCall	tmp(addr, numArgs);
	
	systemCallTable[num] = tmp;	// install the call
}

void SystemCallHandler::RemoveSystemCall(uint num)
{
	if(num > systemCallTable.size())
	{
		ERROR("Bad System call number\n");
		return;
	}
	
	systemCallTable[num] = nullCall;
}

void SystemCallHandler::NullSystemCall()
{
	PANIC("NULL CALL\n");
}



