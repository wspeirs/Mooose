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


/** @file Debug.cpp
 *
 */

#include <constants.h>
#include <types.h>
#include <Debug.h>
#include <printf.h>

bool Debug::startedStackDump = false;	// so we don't keep crashing on a back trace.

Debug::Debug()
	: com1(NULL),
	consoleManager(VirtualConsoleManager::GetInstance()),
	debugConsole(consoleManager.GetDebugConsole())
{
	// print out that we were installed
// 	Print(false, false, false, "*** Debugger Installed ***");
}

void Debug::Print(bool allConsoles, bool backTrace, bool halt, const char *message, ...)
{
	char		buffer[1024];	// limited to this size message
	int		strSize;
	va_list 	ap;

	va_start(ap, message);	// get the arguments

	// format the string
	strSize = vsnprintf(buffer, sizeof(buffer), message, ap);

	// create a string out of the buffer
	string	msg(buffer);

	// no matter what we want to print out to the serial device
	if(com1 != NULL)
		com1->Write(msg);

	if(allConsoles)	// want to print the message to all the consoles
	{
// 		for(int i=1; i < consoleManager.GetMaxConsoleNumber() + 1; ++i)
// 			consoleManager.GetVirtualConsole(i)->printf(buffer);

		consoleManager.GetCurrentConsole()->printf(buffer);	// refresh the screen
	}
	
	else
		debugConsole->printf(buffer);
	
	if(backTrace)
	{
		ulong	*ebp;
		
		if(!startedStackDump)
		{
			startedStackDump = true;	// set the flag
			
			DebugPrint(string("STACK TRACE:\n\n"));
			
			asm __volatile__ ("movl %%ebp, %0": "=r" (ebp) : : );
	
			for(int i=0; i < DEPTH; ++i)
			{
				snprintf(buffer, sizeof(buffer), "ADDR %d: 0x%x\n", i, *(ebp+1));
				DebugPrint(string(buffer));
					
				if(*(ebp+1) == 0xDEADC0DE || *(ebp+1) < 0x100000)
					break;
				ebp = const_cast<ulong *>(reinterpret_cast<const ulong*>(*ebp));
			}
			
			startedStackDump = false;	// reset the flag
		}	
	}
	
	if(halt)
	{
		asm("cli");	// clear the ints so we don't context switch
		while(1);	// loop forever
	}
}

void Debug::HaltMachine()
{
	asm("cli");
	while(1);
}
