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


/** @file mem_utils.cpp
 * This file contains memory utilities used throughout the kernel
 */

#include <constants.h>
#include <types.h>

void MemSet(void *buf, uchar val, ulong size)
{
	uchar	*ptr = reinterpret_cast<uchar*>(buf);

	for(register ulong i=0; i < size; ++i)
		*ptr++ = val;
}

void MemCopy(void *dest, const void *src, ulong size)
{
	uchar		*ptrd = reinterpret_cast<uchar*>(dest);
	const uchar	*ptrs = reinterpret_cast<const uchar*>(src);

	for(register ulong i=0; i < size; ++i)
		*ptrd++ = *ptrs++;
}

bool MemEqual(void *loc1, void *loc2, ulong size)
{
	uchar	*ptr1 = reinterpret_cast<uchar*>(loc1);
	uchar	*ptr2 = reinterpret_cast<uchar*>(loc2);
	
	for(register ulong i=0; i < size; ++i)
	{
		if(*ptr1++ != *ptr2++)
			return false;
	}

	return true;
}

ulong FarPointerToLinearAddress(ushort segment, ushort offset)
{
	ulong	tmp(segment);
	
	tmp = tmp << 4;
	
	return (tmp + offset);
}



#ifndef _STRING_H
// This is required for g++... it uses it for assignment (=) of objects when the operator isn't overloaded.
extern "C" void memcpy(void *dest, const void *src, ulong size)
{
	MemCopy(dest, src, size);
}
#endif
