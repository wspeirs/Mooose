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


/** @file Preempt.h
 *
 */

#ifndef PREEMPT_H
#define PREEMPT_H


#include <constants.h>
#include <types.h>
#include <Handler.h>
#include <ProcessManager.h>

/** @class Preempt
 *
 * @brief 
 *
 **/

extern "C" { ulong PerformPreempt(ulong curESP); }

class Preempt : public Driver
{
public:
	int Startup();
	int IRQSignaled(Registers *regs);
	int Shutdown();

private:
//	ProcessManager	&theProcManager;
	int			count;
	static const int	numTicksPer = 10;
};


#endif // Preempt.h


