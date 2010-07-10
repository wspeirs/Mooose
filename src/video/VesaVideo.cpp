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


/** @file VesaVideo.cpp
 *
 */

#include <constants.h>
#include <types.h>
#include <VesaVideo.h>
#include <io_utils.h>
#include <Debug.h>
#include <printf.h>

// int procID;

// debug
#define sgn(x) ((x<0)?-1:((x>0)?1:0))	// macro to return the sign of a number
#define abs(x) (x<0 ? -1 * x : x)	// absolute value
#define WIDTH	1024
#define HEIGHT	768
#define BPL	0x1000
void plot_pixel(ulong *lfb, int x, int y, ulong color);
void line_fast(ulong*lfb, int x1, int y1, int x2, int y2, ulong color);
void draw_rect(ulong *lfb, int x_top, int y_top, int x_bot, int y_bot, ulong color);
void DrawCharacter(uchar c, ulong *lfb, int x, int y, ulong color);
void print(char *str, ulong *lfb, ulong color);
extern unsigned char fontdata_8x8[];

VESAVideo::VESAVideo()
: currentMode(0), 
  physMem(PhysicalMemManager::GetInstancePtr()), 
  procMan(ProcessManager::GetInstance()),
  myPageDir(physMem->CreatePageDirectory(0)),
  color(0x00888888),
  xpos(0), ypos(0),
  videoMemSize(0),
  videoAddr(reinterpret_cast<volatile ulong*>(0xD2C00000)),
  bufferSize(0),
  screenBuffer(NULL)
{
	asm("cli");
	
	// switch page directories so the new thread will have everything it needs
	// also when we do the mem copy we don't want a fault
	physMem->SetPageDirectory(myPageDir);
	
	// map in the page for the IVT
	physMem->MapPage(0x0, 0x0, myPageDir);
	
	// map in the needed pages for BIOS and IVT
 	for(uint i = 0x000A0000; i <= 0x000D0000; i += PAGE_SIZE)
		physMem->MapPage(i, i, myPageDir);
	
	// map in a page for the code
	physMem->AcquireAndMapPage(0x00007000, myPageDir);
	
	// inject the code
	MemCopy((void*)0x00007000, "\xcd\x10\xcd\x80", 4);
	
	// map in a page for the struct
	DEBUG("AQUIRE & MAP\n");
	physMem->AcquireAndMapPage(0x00008000, myPageDir);
	
	// setup the registers...
	regs.eax = 0x4F00;
	regs.edi = 0x8000;
	regs.v86_es = 0x0000;
	regs.useresp = 0x8000;
	
	// create the thread
	v86Thread = procMan.CreateThread(reinterpret_cast<ThreadFunction>(0x00007000),
					NULL, 
					Thread::V86, 
					0,
					PAGE_SIZE,
					&regs);
	
	asm("sti");	// turn on ints so we task switch
	
	v86Thread->Join();	// pause this thread, until the other is done
	
	asm("cli");	// turn off ints
	
	
	DEBUG("VESA TEST DONE\n");
	
	VBEInfo	*info = reinterpret_cast<VBEInfo*>(0x8000);

	// need some sanity checks in here
	if(info->vbesignature[0] != 'V' ||
	   info->vbesignature[1] != 'E' ||
	   info->vbesignature[2] != 'S' ||
	   info->vbesignature[3] != 'A')
	{
		ERROR("VESA POLL DID NOT WORK");
		return;
	}
		

	DEBUG("SIG: %c%c%c%c\n", info->vbesignature[0],
	       			  info->vbesignature[1],
	       			  info->vbesignature[2],
	       			  info->vbesignature[3]);
	DEBUG("VERSION: %x\n", info->vbeversion);
// 	DEBUG("OEM PTR: 0x%x\n", info->oemstringptr);
// 	DEBUG("CAP: %d %d %d %d\n", info->capabilities[0],
// 				     info->capabilities[1],
// 				     info->capabilities[2],
// 				     info->capabilities[3]);
// 	printf("MODE PTR: 0x%x\n", info->videomodeptr);
	DEBUG("MEM: %d\n", info->totalmemory);

	DEBUG("MODE: %x:%x\n", info->videomodeptr[1], info->videomodeptr[0]);
	
	ushort	*modePtr = reinterpret_cast<ushort*>(FarPointerToLinearAddress(info->videomodeptr[1], info->videomodeptr[0]));
	
	// loop through the modes
	for(int i=0; modePtr[i] != 0xFFFF; ++i)
	{
		regs.eax = 0x4F01;
		regs.ecx = modePtr[i];
		regs.edi = 0x8000 + sizeof(VBEInfo);
		regs.v86_es = 0x0000;
		regs.useresp = 0x8000 + sizeof(VBEInfo); // make sure we don't trash the stack... the modes are stored here
		
		v86Thread = procMan.CreateThread(reinterpret_cast<ThreadFunction>(0x00007000),
						NULL, 
						Thread::V86, 
						0,
						PAGE_SIZE,
						&regs);
		
		asm("sti");	// turn on ints so we task switch
		
		v86Thread->Join();	// pause this thread, until the other is done
		
		asm("cli");	// turn off ints
		
		// get the info block
		VGAModeInfoBlock	*modeInfoPtr = reinterpret_cast<VGAModeInfoBlock*>(0x8000+ sizeof(VBEInfo));
		
		if(modeInfoPtr->modeattributes & 0x80 &&	// lfb
		   modeInfoPtr->modeattributes & 0x01 &&	// graphical
		   modeInfoPtr->bitsperpixel == 32)
		{
			VGAMode		theMode;
			
			// setup the struct
			theMode.modeNumber = modePtr[i];
			theMode.xresolution = modeInfoPtr->xresolution;
			theMode.yresolution = modeInfoPtr->yresolution;
			theMode.physicalAddr = modeInfoPtr->physbaseptr;
			
			DEBUG_NL("NUMBER: (%d) 0x%x: ", i, modePtr[i]);
		
			DEBUG_NL("%d x %d x %d @ 0x%x 0x%x\n", modeInfoPtr->xresolution, 
						 modeInfoPtr->yresolution,
						 modeInfoPtr->bitsperpixel,
						 modeInfoPtr->physbaseptr,
			      			 modeInfoPtr->bytesperscanline);
			// add the mode to the list
			modes.push_back(theMode);
		}
	}
}

void VESAVideo::PrintModes(VirtualConsole &console)
{
	for(uint i=0; i < modes.size(); ++i)
	{
		console.printf("%d:  NUM: 0x%X  RES: %d x %d\n", i,
			       modes[i].modeNumber, 
			       modes[i].xresolution, 
			       modes[i].yresolution);	// write out the description
	}
}

bool VESAVideo::SwitchMode(ushort mode)
{
	if(mode >= modes.size())
		return false;
	
	currentMode = mode;	// set the current mode
	
	// setup the registers for the switch call
	regs.eax = 0x4F02;
	regs.ebx = modes[mode].modeNumber | 0xC000;
	regs.useresp = 0x8000;
	
	// create the thread to do the switch
	v86Thread = procMan.CreateThread(reinterpret_cast<ThreadFunction>(0x00007000),
					 NULL, 
					 Thread::V86, 
					 0,
					 PAGE_SIZE,
					 &regs);
	
	asm("sti");	// turn on ints so we task switch
	
	v86Thread->Join();	// pause this thread, until the other is done
	
	asm("cli");	// turn off ints
	
	// set the size of the video memory in bytes
	videoMemSize = modes[mode].xresolution * modes[mode].yresolution * sizeof(ulong);
	
	// setting video memory to 0xD2C00000 -- why not?
	for(uint phys = modes[mode].physicalAddr, virt = 0xD2C00000;
		   phys < modes[mode].physicalAddr + videoMemSize; 
		   phys += PAGE_SIZE, virt += PAGE_SIZE)
	{
// 		DEBUG("MAP: %x -> %x\n", phys, virt);
		physMem->MapPage(virt, phys, myPageDir);
	}
	
	maxX = (modes[mode].xresolution / charBoxWidth) - 1;		// set the max X position
	maxY = (modes[mode].yresolution / charBoxHeight) - 1;		// set the max Y position
	
	// delete any memory that might have been there
	delete [] screenBuffer;
	
	// the character buffer is the height times the width
	bufferSize = maxX * maxY;
	
	// allocate the memory for the character buffer
	screenBuffer = new char[bufferSize];
	
	xpos = ypos = 0;	// reset the position
	lastPos = 0;	// reset the last positions
	
	ClearScreen();		// clear the screen
	
	return true;
}


void VESAVideo::SetFontColor(Color fc)
{
	switch(fc)
	{
		case RED:
			color = 0x00FF0000;
			break;
		
		case GREEN:
			color = 0x0000FF00;
			break;
			
		case BLUE:
			color = 0x000000FF;
			break;
			
		case WHITE:
			color = 0x00FFFFFF;
			break;
		
		case GREY:
			color = 0x00888888;
			break;
			
		default:
			color = 0x00888888;
			break;
	}
}

void VESAVideo::PutCharacter(char c, bool refresh)
{
	// we got a newline character
	if (c == '\n' || c == '\r')
	{
		xpos = 0;	// reset the x position
		ypos++;		// move down one in the y position
		
		if (ypos >= maxY)	// we want to scroll the screen up
		{
			BlackScreen();	// black the screen because it won't be valid

			// move all the lines up
			for(uint i=0; i < maxY-1; ++i)
				MemCopy((uchar*)(screenBuffer + i*maxX),
					 (uchar*)(screenBuffer + (i+1)*maxX), maxX);
			
			// zero out the last line
			MemSet((uchar*)(screenBuffer + (maxY-1) * maxX), 0, maxX);
			
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
		
		screenBuffer[(xpos + ypos * maxX)] = c;	// set the character
		
		xpos++;	// increase the xposition by the 1 character we wrote
	}
	
	// refresh the screen if asked for
	if(refresh)
		UpdateScreen();
}

void VESAVideo::DrawCharacter(uint x, uint y, char c)
{
	uchar	mask = 0x80;	// this should really be width independant
	uint 	count = 0;
	uint	line = 0;
	uint	index = c * charHeight;
	
	// go through each line of the font pixel map
	for(int i=0; i < charHeight; ++i)
	{
		mask = 0x80;	// this should really be width independant
		count = 0;
		line = (y + i) * modes[currentMode].xresolution;
		
		do
		{
			// check if the bit is set
			if(font[index + i] & mask)
				videoAddr[line + (x + count)] = color; // plot it
		
			mask >>= 1;	// move the bit mask
			++count;	// increase the x count
					
		} while(mask != 0x0);	// go until we're done
	}	
}

void VESAVideo::ClearScreen()
{
	MemSet(screenBuffer, 0, bufferSize);	// flush the character buffer
	xpos = ypos = 0;	// reset the position in the character buffer
	lastPos = 0;	// reset the last positions
	
// 	MemSet((uchar*)videoAddr, 0, videoMemSize);
	
	ulong	*ptr = (ulong*)videoAddr;
	ulong	*limit = const_cast<ulong*>(videoAddr);
	
	limit += modes[currentMode].xresolution * modes[currentMode].yresolution;
	
	while(ptr < limit)
		*ptr++ = 0x0;
}

void VESAVideo::RefreshScreen()
{
	ClearScreen();
	UpdateScreen();
}

void VESAVideo::BlackScreen()
{
	ulong curColor = color;
	
	lastPos = 0;
	color = 0x0;
	UpdateScreen();
	color = curColor;
	lastPos = 0;
}


void VESAVideo::UpdateScreen()
{
	uint	x, y;	// the x & y position in pixels
	ulong	curPos = xpos + ypos * maxX;
	
	// go through the character buffer rendering each character
	for( ; lastPos < curPos; ++lastPos)
	{
		// check for a space or zeroed memory
		if(screenBuffer[lastPos] == ' ' ||
		   screenBuffer[lastPos] == '\0')
			continue;
		
		y = charBoxHeight + (lastPos/maxX)*charBoxHeight;
		x = charBoxWidth + (lastPos%maxX)*charBoxWidth;
		
		DrawCharacter(x, y, screenBuffer[lastPos]);	// draw the character
	}
}







/*

void VesaTest(void *arg)
{
	asm("cli");
	
	PhysicalMemManager	*physMem = PhysicalMemManager::GetInstancePtr();
	ProcessManager		&procMan = ProcessManager::GetInstance();
	
	// first we want to get a new page directory so we can set it up for the threads
	ulong	myPageDir = physMem->CreatePageDirectory(0);
	
	// switch page directories so the new thread will have everything it needs
	// also when we do the mem copy we don't want a fault
	physMem->SetPageDirectory(myPageDir);
	
	// map in the page for the IVT
	physMem->MapPage(0x0, 0x0, myPageDir);
	
	// map in the needed pages for BIOS and IVT
 	for(uint i = 0x000A0000; i <= 0x000D0000; i += PAGE_SIZE)
		physMem->MapPage(i, i, myPageDir);
	
	// map in a page for the code
	physMem->AcquireAndMapPage(0x00007000, myPageDir);
	
	// inject the code
	MemCopy((void*)0x00007000, "\xcd\x10\xcd\x80", 4);
	
	// map in a page for the struct
	DEBUG("AQUIRE & MAP\n");
	physMem->AcquireAndMapPage(0x00008000, myPageDir);
	
	
	// setup the registers...
	Registers	regs;
	
	regs.eax = 0x4F00;
	regs.edi = 0x8000;
	regs.v86_es = 0x0000;
	regs.useresp = 0x8000;
	
	// create the thread
	Thread	*v86Thread = procMan.CreateThread(reinterpret_cast<ThreadFunction>(0x00007000),
						NULL, 
						Thread::V86, 
						0,
						PAGE_SIZE,
						&regs);
	
	asm("sti");	// turn on ints so we task switch
	
	v86Thread->Join();	// pause this thread, until the other is done
	
	asm("cli");	// turn off ints
	
	
	DEBUG("VESA TEST DONE\n");
	
	VBEInfo	*info = reinterpret_cast<VBEInfo*>(0x8000);

	DEBUG("SIG: %c%c%c%c\n", info->vbesignature[0],
	       			  info->vbesignature[1],
	       			  info->vbesignature[2],
	       			  info->vbesignature[3]);
	DEBUG("VERSION: %x\n", info->vbeversion);
// 	DEBUG("OEM PTR: 0x%x\n", info->oemstringptr);
// 	DEBUG("CAP: %d %d %d %d\n", info->capabilities[0],
// 				     info->capabilities[1],
// 				     info->capabilities[2],
// 				     info->capabilities[3]);
// 	printf("MODE PTR: 0x%x\n", info->videomodeptr);
	DEBUG("MEM: %d\n", info->totalmemory);

	DEBUG("MODE: %x:%x\n", info->videomodeptr[1], info->videomodeptr[0]);
	
	ushort	*modePtr = reinterpret_cast<ushort*>(FarPointerToLinearAddress(info->videomodeptr[1], info->videomodeptr[0]));
	ushort	modeNumber;
	ulong	physAddr;
	
	// loop through the modes
	for(int i=0; modePtr[i] != 0xFFFF; ++i)
	{
		regs.eax = 0x4F01;
		regs.ecx = modePtr[i];
		regs.edi = 0x8000 + sizeof(VBEInfo);
		regs.v86_es = 0x0000;
		regs.useresp = 0x8000 + sizeof(VBEInfo); // make sure we don't trash the stack... the modes are stored here
		
		v86Thread = procMan.CreateThread(reinterpret_cast<ThreadFunction>(0x00007000),
						NULL, 
						Thread::V86, 
						0,
						PAGE_SIZE,
						&regs);
		
		asm("sti");	// turn on ints so we task switch
		
		v86Thread->Join();	// pause this thread, until the other is done
		
		asm("cli");	// turn off ints
		
		// get the info block
		VGAModeInfoBlock	*modeInfoPtr = reinterpret_cast<VGAModeInfoBlock*>(0x8000+ sizeof(VBEInfo));
		
		if(modeInfoPtr->modeattributes & 0x80 &&	// lfb
		   modeInfoPtr->modeattributes & 0x01 &&	// graphical
		   modeInfoPtr->xresolution == WIDTH &&
		   modeInfoPtr->yresolution == HEIGHT &&
		   modeInfoPtr->bitsperpixel == 32)
		{
			DEBUG("NUMBER: (%d) 0x%x: ", i, modePtr[i]);
		
			DEBUG("%d x %d x %d @ 0x%x 0x%x", modeInfoPtr->xresolution, 
						 modeInfoPtr->yresolution,
						 modeInfoPtr->bitsperpixel,
						 modeInfoPtr->physbaseptr,
			      			 modeInfoPtr->bytesperscanline);
			DEBUG("  ATTR: 0x%x (%s)",
			       modeInfoPtr->modeattributes,
			      modeInfoPtr->modeattributes & 0x01 ? "GRAPHICS" : "TEXT");
			
			modeNumber = modePtr[i];
			physAddr = modeInfoPtr->physbaseptr;
			DEBUG("\n");
		}
	}
	
	// we want to switch into mode 0x112
	regs.eax = 0x4F02;
	regs.ebx = modeNumber | 0xC000;
	regs.useresp = 0x8000;
	
	v86Thread = procMan.CreateThread(reinterpret_cast<ThreadFunction>(0x00007000),
					NULL, 
					Thread::V86, 
					0,
					PAGE_SIZE,
					&regs);
	
	asm("sti");	// turn on ints so we task switch
	
	v86Thread->Join();	// pause this thread, until the other is done
	
	asm("cli");	// turn off ints
	
	// setting video memory to 0xD2C00000 -- why not?
	for(uint phys = physAddr, virt = 0xD2C00000;
	    phys < physAddr + WIDTH*HEIGHT*sizeof(uint); 
	    phys += PAGE_SIZE, virt += PAGE_SIZE)
	{
		DEBUG("MAP: %x -> %x\n", phys, virt);
		physMem->MapPage(virt, phys, myPageDir);
	}
	
	//0xA00 bytes per scan line
	ulong *lfb = (ulong*)0xD2C00000;
	
	
	//write out my name... keeping track of the current position
	print("Hello world\nBill Speirs", lfb, 0x00AA0F0F);	
	
	
	
	PANIC("");
	
	const long	color = 0x0000FF00;
	const int 	leftEdge = 120;	// 20
	
	// draw the M
	line_fast(lfb, leftEdge, leftEdge, leftEdge, leftEdge+80, color);
	line_fast(lfb, leftEdge+1, leftEdge, leftEdge+1, leftEdge+80, color);
	
	line_fast(lfb, leftEdge+50, leftEdge, leftEdge+50, leftEdge+80, color);
	line_fast(lfb, leftEdge+51, leftEdge, leftEdge+51, leftEdge+80, color);
	
	line_fast(lfb, leftEdge, leftEdge, leftEdge+25, leftEdge+30, color);
	line_fast(lfb, leftEdge+1, leftEdge, leftEdge+26, leftEdge+30, color);
	
	line_fast(lfb, leftEdge+50, leftEdge, leftEdge+25, leftEdge+30, color);
	line_fast(lfb, leftEdge+51, leftEdge, leftEdge+26, leftEdge+30, color);
	
	// draw the Os
	draw_rect(lfb, leftEdge+70, leftEdge+30, leftEdge+120, leftEdge+80, color);
	draw_rect(lfb, leftEdge+71, leftEdge+29, leftEdge+121, leftEdge+79, color);

	draw_rect(lfb, leftEdge+140, leftEdge+30, leftEdge+190, leftEdge+80, color);
	draw_rect(lfb, leftEdge+141, leftEdge+29, leftEdge+191, leftEdge+79, color);

	draw_rect(lfb, leftEdge+210, leftEdge+30, leftEdge+260, leftEdge+80, color);
	draw_rect(lfb, leftEdge+211, leftEdge+29, leftEdge+261, leftEdge+79, color);

	// draw the S
	line_fast(lfb, leftEdge+280, leftEdge+30, leftEdge+330, leftEdge+30, color);
	line_fast(lfb, leftEdge+280, leftEdge+31, leftEdge+330, leftEdge+31, color);
	
	line_fast(lfb, leftEdge+280, leftEdge+30, leftEdge+280, leftEdge+55, color);
	line_fast(lfb, leftEdge+281, leftEdge+30, leftEdge+281, leftEdge+55, color);
	
	line_fast(lfb, leftEdge+280, leftEdge+55, leftEdge+330, leftEdge+55, color);
	line_fast(lfb, leftEdge+280, leftEdge+56, leftEdge+330, leftEdge+56, color);
	
	line_fast(lfb, leftEdge+330, leftEdge+55, leftEdge+330, leftEdge+80, color);
	line_fast(lfb, leftEdge+331, leftEdge+55, leftEdge+331, leftEdge+80, color);

	line_fast(lfb, leftEdge+280, leftEdge+80, leftEdge+330, leftEdge+80, color);
	line_fast(lfb, leftEdge+280, leftEdge+81, leftEdge+330, leftEdge+81, color);
	
	// draw the E
	line_fast(lfb, leftEdge+350, leftEdge+30, leftEdge+400, leftEdge+30, color);
	line_fast(lfb, leftEdge+350, leftEdge+31, leftEdge+400, leftEdge+31, color);
	
	line_fast(lfb, leftEdge+350, leftEdge+55, leftEdge+380, leftEdge+55, color);
	line_fast(lfb, leftEdge+350, leftEdge+56, leftEdge+380, leftEdge+56, color);
	
	line_fast(lfb, leftEdge+350, leftEdge+80, leftEdge+400, leftEdge+80, color);
	line_fast(lfb, leftEdge+350, leftEdge+81, leftEdge+400, leftEdge+81, color);
	
	line_fast(lfb, leftEdge+350, leftEdge+30, leftEdge+350, leftEdge+80, color);
	line_fast(lfb, leftEdge+351, leftEdge+30, leftEdge+351, leftEdge+80, color);
	
	
	PANIC("");
}

int sx = 10;
int sy = 10;

void print(char *str, ulong *lfb, ulong color)
{
	for(int i=0; str[i] != '\0'; ++i)
	{
		if(str[i] == '\n')
		{
			sy += 10;
			sx = 10;
			continue;
		}
		
		DrawCharacter((uchar)str[i], lfb, sx, sy, color);
		sx += 10;
	}
}

void DrawCharacter(uchar c, ulong *lfb, int x, int y, ulong color)
{
	// go through each line
	for(int i=0; i < 8; ++i)
	{
		uchar mask = 0x80;
		uchar count = 0;
		
		do
		{
			if(fontdata_8x8[(c * 8) + i] & mask)	// that bit is set
				plot_pixel(lfb, x+count, y+i, color);
		
			mask >>= 1;
			++count;
					
		} while(mask != 0x0);
	}
}


void draw_rect(ulong *lfb, int x_top, int y_top, int x_bot, int y_bot, ulong color)
{
	line_fast(lfb, x_top, y_top, x_top, y_bot, color);
	line_fast(lfb, x_top, y_top, x_bot, y_top, color);
	line_fast(lfb, x_bot, y_top, x_bot, y_bot, color);
	line_fast(lfb, x_bot, y_bot, x_top, y_bot, color);
}

void line_fast(ulong *lfb, int x1, int y1, int x2, int y2, ulong color)
{
	int i,dx,dy,sdx,sdy,dxabs,dyabs,x,y,px,py;
	
	dx=x2-x1;      // the horizontal distance of the line
	dy=y2-y1;      // the vertical distance of the line
	dxabs=abs(dx);
	dyabs=abs(dy);
	sdx=sgn(dx);
	sdy=sgn(dy);
	x=dyabs>>1;
	y=dxabs>>1;
	px=x1;
	py=y1;
	
// 	lfb[(py<<8)+(py<<6)+px]=color;
	
	if (dxabs>=dyabs) // the line is more horizontal than vertical
	{
		for(i=0;i<dxabs;i++)
		{
			y+=dyabs;
			if (y>=dxabs)
			{
				y-=dxabs;
				py+=sdy;
			}
			px+=sdx;
			plot_pixel(lfb, px, py, color);
		}
	}
	
	else // the line is more vertical than horizontal
	{
		for(i=0;i<dyabs;i++)
		{
			x+=dxabs;
			if (x>=dyabs)
			{
				x-=dyabs;
				px+=sdx;
			}
			py+=sdy;
			plot_pixel(lfb, px, py, color);
		}
	}
}

*/




