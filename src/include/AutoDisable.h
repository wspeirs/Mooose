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


/** @file AutoDisable.h
 *
 */

#ifndef AUTODISABLE_H
#define AUTODISABLE_H


#include <constants.h>
#include <types.h>


/** @class AutoDisable
 *
 * @brief If this object is created then interrupts are disabled ONLY if they were already on.
 *
 **/

class AutoDisable
{
public:
	AutoDisable();		// turn off interrupts
	~AutoDisable();		// turn on interrupts
	
	void Enable();		// turn interrputs on

private:
	bool	turnOn;
};


#endif // AutoDisable.h


