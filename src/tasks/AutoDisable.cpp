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


/** @file AutoDisable.cpp
 *
 */

#include <constants.h>
#include <types.h>
#include <AutoDisable.h>
#include <screen_utils.h>

// Get the value in the EFLAGS register and check to see if interrupts are already on
// then disable appropriately
AutoDisable::AutoDisable()
{
	ulong	eflags;
	
	// get the eflags reg
	asm __volatile__ ("pushf;\n pop %%eax;\nmovl %%eax, %0": "=r" (eflags) : : "%eax");
	
	if(eflags & 0x200)	// interrupts are on, so turn them off
	{
		asm("cli");
		turnOn = true;
	}
	
	else	// they aren't on so we basically do nothing
		turnOn = false;
}

// destructor
AutoDisable::~AutoDisable()
{
	if(turnOn)
		asm("sti");
}

// turn on interrupts if we should, and then don't turn off during destruction
void AutoDisable::Enable()
{
	if(turnOn)
		asm("sti");
	
	turnOn = false;
}
