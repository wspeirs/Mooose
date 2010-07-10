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


/** @file constants.h
 * Thile file contains all of the kernel-wide constants
 */

#ifndef CONSTANTS_H
#define CONSTANTS_H

#ifndef NULL

// General stuff
#define NULL	0x0
#define FALSE	0x0
#define TRUE	0x1

#endif

// Memory stuff
#define BYTES_PER_PAGE		4096	// number of bytes in a page

// Helpful macros
#define MIN(x,y)	( x < y ? x : y )
#define MAX(x,y)	( x > y ? x : y )

#endif // CONSTANTS_H
