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


/** @file Preempt.cpp
 *
 */

#include <constants.h>
#include <types.h>
#include <Preempt.h>
#include <io_utils.h>
#include <Debug.h>

//void PerformPreempt(ulong **oldESP, ulong **newESP)
//ProcessManager &Preempt::theProcManager = ProcessManager::GetInstance();

ulong PerformPreempt(ulong curESP)
{
	static int		counter = 0;

	if(++counter == 100)
	{
		ulong *oldESP, *newESP;
		
		counter = 0;
	
		ProcessManager::GetInstance().ScheduleNextProcess(&oldESP, &newESP);
		
// 		printf("OLD ESP: %x    NEW ESP: %x\n", oldESP, newESP);
		
		*oldESP = curESP;	// save the value
		
		return(*newESP);
	}
	
	else
		return(curESP);
}

int Preempt::Startup()
{
	count = 0;
	
	return(0);
}

int Preempt::IRQSignaled(Registers *regs)
{
	(void)regs;
	
	if(++count % numTicksPer == 0)
	{
		DEBUG("Called schedule...\n");
		ProcessManager::GetInstance().PerformTaskSwitch();	// we are always called from an INT
//		DEBUG("Back from Schedule\n");
		count = 0;	// prevent anything weird from a wrap around
	}
	
	return(0);
}

int Preempt::Shutdown()
{
	return(0);
}


