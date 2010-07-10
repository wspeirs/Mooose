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


/** @file VideoDriver.cpp
 * Contains the implementation of the VGA video driver
 */

#include <constants.h>
#include <types.h>
#include <VideoDriver.h>

VGADriver::VGADriver()
: color(FG_LIGHT_GREY),		// set the default font to light grey
  xpos(0), ypos(0),		// set the x & y position to zero
  videoAddr(reinterpret_cast<volatile uchar*>(0xC00B8000)), // set the address of video memory
  bufferSize(maxX * maxY * 2),	// two bytes for each position
  screenBuffer(new uchar[bufferSize])	// create the new screen console
{
	ClearScreen();	// clear the screen
}

VGADriver::~VGADriver()
{
	delete [] screenBuffer;	// free the memory
}

void VGADriver::SetFontColor(Color fc)
{
	switch(fc)
	{
		case RED:
			color = FG_RED;
			break;
		
		case GREEN:
			color = FG_GREEN;
			break;
			
		case BLUE:
			color = FG_BLUE;
			break;
			
		case WHITE:
			color = FG_WHITE;
			break;
		
		case GREY:
			color = FG_LIGHT_GREY;
			break;
			
		default:
			color = FG_LIGHT_GREY;
			break;
	}
}

void VGADriver::PutCharacter(char c, bool refresh)
{
	// we got a newline character
	if (c == '\n' || c == '\r')
	{
		xpos = 0;
		ypos++;
		
		if (ypos >= maxY)	// we want to scroll the screen up
		{
			// move all the lines up
			for(int i=0; i < maxY-1; ++i)
				MemCopy((uchar*)(screenBuffer + (i*maxX) * 2),
					 (uchar*)(screenBuffer + ((i+1)*maxX) * 2), maxX*2);
			
			// zero out the last line
			MemSet((uchar*)(screenBuffer + (maxY-1) * maxX * 2), 0, maxX * 2);
			
			ypos = maxY-1;	// set to the last line
		}
	}
	
	else if(c == '\t')
	{
		if(xpos >= maxX)	// check to see if we need to wrap
			this->PutCharacter('\n', refresh);
		
		xpos += 5 - (xpos % 5); // use 5 character tab	
	}

	else
	{	
		if (xpos >= maxX)	// check to see if we need to wrap
			this->PutCharacter('\n', refresh);
		
		*(screenBuffer + (xpos + ypos * maxX) * 2) = c & 0xFF;	// clears out the first part
		*(screenBuffer + (xpos + ypos * maxX) * 2 + 1) = color;	// set the color
		
		xpos++;	// increase the xposition by the 1 character we wrote
	}
	
	// refresh the screen if asked for
	if(refresh)
		RefreshScreen();
}

void VGADriver::ClearScreen()
{
	MemSet(screenBuffer, 0, bufferSize);	// zero out all the console memory
	xpos = ypos = 0;	// reset the x & y positions
	RefreshScreen();	// refresh the screen
}

void VGADriver::RefreshScreen()
{
	MemCopy(const_cast<uchar*>(videoAddr), screenBuffer, bufferSize);
}

