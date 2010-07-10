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


/** @file panic.cpp
 *
 */

#ifdef UNIT_TEST

#include "harness_code.h"
#include "constants.h"
#include "types.h"
#include "panic.h"

#else

#include <constants.h>
#include <types.h>
#include <AutoDisable.h>
#include <panic.h>
#include <Debug.h>

#endif

bool Panic::startedStackDump = false;

Panic::Panic()
{
	;
}

Panic::Panic(const char *message)
{
	AutoDisable	lock;
	
	DEBUG("PANIC: %s\n", message);
	
	while(1);
}

void Panic::PrintMessage(const char *message, bool stackTrace, int depth)
{
	ulong	*ebp;
		
	asm("cli");
	
	DEBUG("PANIC: %s\n", message);
		
	if(stackTrace && !startedStackDump)
	{
		startedStackDump = true;
		
		DEBUG("STACK TRACE:\n\n");
		
		asm __volatile__ ("movl %%ebp, %0": "=r" (ebp) : : );

		for(int i=0; i < depth; ++i)
		{
			DEBUG("ADDR %d: 0x%x\n", i, *(ebp+1));
				
			if(*(ebp+1) == 0xDEADC0DE || *(ebp+1) < 0x100000)
				break;
			ebp = const_cast<ulong *>(reinterpret_cast<const ulong*>(*ebp));
		}
	}
		
	while(1);
}



