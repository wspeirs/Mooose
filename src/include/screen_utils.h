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


/** @file screen_utils.h
 *
 */

#ifndef SCREEN_UTILS_H
#define SCREEN_UTILS_H

// VGA stuff
#define VGA_SCREEN_COLUMNS		80	// The number of columns
#define VGA_SCREEN_LINES		24	// The number of lines
#define VGA_CHAR_ATTRIBUTE		7	// The attribute of a character
#define VGA_VIDEO_ADDR		0xC00B8000	// The video memory address

/**
 * Print a string to the VGA memory
 * This should only be used for the very initial startup stuff.
 * After the InitTasks function the regular printf should be used.
 * @param str The string to print.
 */
void vga_printf(const char *format, ...);

/**
 * Puts a single character into the VGA memory, with proper y-wrap.
 * @param c The character to put into VGA memory.
 */
void vga_put(char c);

/**
 * Clears the VGA screen memory.
 */
void ClearScreen(void);

#endif // SCREEN_UTILS_H
