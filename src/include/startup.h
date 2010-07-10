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


/** @file startup.h
 *
 */

#ifndef STARTUP_H
#define STARTUP_H

#include <multiboot.h>
#include <i386.h>

void PrintMultiBootInfo(MultibootInfo *multibootInfo);
bool GetRAMStartAndEnd(MultibootInfo *multibootInfo, ulong &ramStart, ulong &ramEnd);

#endif // STARTUP_H
