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


/** @file mem_utils.h
 *
 */

#ifndef MEM_UTILS_H
#define MEM_UTILS_H

/**
 * Sets a block of memory
 * @param buf The location of the memory to set.
 * @param val The value to set the memory to.
 * @param size The length, in bytes, of the memory.
 */
void MemSet(void *buf, uchar val, ulong size);

/**
 * Copys a block of memory to a memory location
 * @param dest The destination.
 * @param src The source.
 * @param size The length, in bytes, of the memory.
 */
void MemCopy(void *dest, const void *src, ulong size);

/**
 * Compares two blocks of memory.
 * @param loc1 The first location.
 * @param loc2 The second location.
 * @param size The length, in bytes, of the memory.
 */
bool MemEqual(void *loc1, void *loc2, ulong size);

/**
 * Converts a far pointer to a linear address.
 * @param segment The segment of the memory.
 * @param offset The offset into the segment.
 * @return The far pointer.
 */
ulong FarPointerToLinearAddress(ushort segment, ushort offset);

#ifndef _STRING_H
extern "C" void memcpy(void *dest, const void *src, ulong size);
#endif

#endif // MEM_UTILS_H


