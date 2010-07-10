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


/** @file panic.h
 *
 */

#ifndef PANIC_H
#define PANIC_H

#ifdef UNIT_TEST

#include "constants.h"
#include "types.h"
//#include "screen_utils.h"

#else

#include <constants.h>
#include <types.h>
#include <screen_utils.h>

#endif


/** @class Panic
 *
 * @brief This class handles all kernel panics.
 * All attempts are make to dump as much helpful information to the screen as possible, then infinite loop.
 *
 **/

class Panic
{
public:

	Panic();
	Panic(const char *message);

	static void HaltMachine()
	{
		asm("cli");
		while(1);
	}

	static void PrintMessage(const char *message, bool stackTrace = true, int depth = 8);
private:
	
	static bool startedStackDump;
};


#endif // panic.h


