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


/** @file io_utils.h
 *
 */

#ifndef IO_UTILS_H
#define IO_UTILS_H

extern "C" 
{
	// Reading data from an I/O port
	int inb(int reg);
	int inw(int reg);
	int inl(int reg);
	int insb(int reg, void *des, int size);		// for bytes
	int insw(int reg, void *des, int size);		// for words

	// Writing data out to an I/O port
	int outb(int reg, int byte);
	int outw(int reg, int word);
	int outl(int reg, int doubleWord);
	int outsb(int reg, void *des, int size);	// for bytes
	int outsw(int reg, void *des, int size);	// for words
	
}

#endif // IO_UTILS_H


