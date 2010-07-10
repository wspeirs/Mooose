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


/** @file text_utils.cpp
 * This file contains all of the functions to display text on the screen
 */

#include <constants.h>
#include <types.h>
#include <screen_utils.h>
#include <mem_utils.h>
#include <printf.h>

// Globals... 
static int xpos;			// Save the X position. 
static int ypos;			// Save the Y position. 
static volatile unsigned char *video;	// Point to the video memory. 

// Clear the screen and initialize VIDEO, XPOS and YPOS. 
void ClearScreen (void)
{
	video = (uchar *) VGA_VIDEO_ADDR;
	
	for (int i = 0; i < VGA_SCREEN_COLUMNS * VGA_SCREEN_LINES * 2; ++i)
		*(video + i) = 0;
	
	xpos = 0;
	ypos = 0;
}

void vga_put(char c)
{
	switch(c)
	{
	// new line characters
	case '\n':
	case '\r':
		xpos = 0;
		ypos++;
		
		if (ypos >= VGA_SCREEN_LINES)	// we want to scroll the screen up
		{
			for(int i=0; i < VGA_SCREEN_LINES-1; ++i)
				MemCopy((uchar*)(video + (i*VGA_SCREEN_COLUMNS) * 2),
					 (uchar*)(video + ((i+1)*VGA_SCREEN_COLUMNS) * 2), VGA_SCREEN_COLUMNS*2);
			
			MemSet((uchar*)(video + (VGA_SCREEN_LINES-1) * VGA_SCREEN_COLUMNS * 2), 0, VGA_SCREEN_COLUMNS * 2);	// zero out the last line
			
			ypos = VGA_SCREEN_LINES-1;	// set to the last line
		}
		break;
	case '\t':
		xpos += 5;	// tab is 5 spaces
		break;
	default:
		// actually print the character
		*(video + (xpos + ypos * VGA_SCREEN_COLUMNS) * 2) = c & 0xFF;
		*(video + (xpos + ypos * VGA_SCREEN_COLUMNS) * 2 + 1) = VGA_CHAR_ATTRIBUTE;
	}
	
	// check to see if we're too far in the x direction
	xpos++;
	if (xpos >= VGA_SCREEN_COLUMNS)
		vga_put('\n');
}


void vga_printf (const char *format, ...)
{
	//
	// Maybe add something so that this function prints an error if called after InitTasks
	//
	char	buffer[1024];	// limited to this size message
	va_list ap;
	va_start(ap, format);	

	// format the string
	vsnprintf(buffer, sizeof(buffer), format, ap);

	for(int i=0; buffer[i] != '\0'; ++i)
		vga_put(buffer[i]);
}
