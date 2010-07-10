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


/** @file VirtualConsole.cpp
 *
 */

#include <constants.h>
#include <types.h>
#include <VirtualConsole.h>
#include <VirtualConsoleManager.h>
#include <KeyboardDriver.h>
#include <printf.h>

VirtualConsole::VirtualConsole(VideoDriver *vd)
: videoDriver(vd),	// set the video driver
  consoleSem(0)		// set so the console is NOT active
{
	videoDriver->ClearScreen();	// clear the screen
}

void VirtualConsole::SetVideoDriver(VideoDriver *vd)
{
	videoDriver = vd;	// swap the pointers
	ClearScreen();		// clear the screen
}

uchar VirtualConsole::GetCharacter()
{
	uchar	ret;
	
	do
	{
		WaitSignalConsoleSemaphore();
		
		ret = KeyboardDriver::GetInstance().GetCharacter();
	
	} while(VirtualConsoleManager::GetInstance().GetCurrentConsole() != this);
	
	return ret;
}

void VirtualConsole::SignalConsoleSemaphore()
{
	consoleSem.Signal();
}

void VirtualConsole::WaitSignalConsoleSemaphore()
{
	// wait until we're the active console
	if(VirtualConsoleManager::GetInstance().GetCurrentConsole() != this)
	{
//		VirtualConsoleManager::GetInstance()->GetCurrentConsole()->printf("Having to wait on console...\n");
		consoleSem.Wait();
	}
}

/*void VirtualConsole::EraseLastCharacter()
{
	WaitSignalConsoleSemaphore();
		
	xpos = xpos == 0 ? 0 : xpos - 1;	// move the position back
	
	*(consoleBuffer + (xpos + ypos * SCREEN_COLUMNS) * 2) = 0xFF;		// clear out the character
	*(consoleBuffer + (xpos + ypos * SCREEN_COLUMNS) * 2 + 1) = 0x00;
	
	RefreshScreen();

}
*/

int VirtualConsole::printf(const char *format, ...)
{
// 	WaitSignalConsoleSemaphore();

	char	buffer[1024];	// limited to this size message
	va_list ap;
	
	va_start(ap, format);	

	// format the string
	uint size = vsnprintf(buffer, sizeof(buffer), format, ap);
	buffer[size] = '\0';	// set the null

	uint i;
	for(i=0; i < size-1; ++i)
		videoDriver->PutCharacter(buffer[i], false);	// write the buffer
	
	videoDriver->PutCharacter(buffer[i], true);		// write the last char with a refresh
	
	return size;
}

void VirtualConsole::ClearScreen(void)
{
	if(this == VirtualConsoleManager::GetInstance().GetCurrentConsole())
		videoDriver->ClearScreen();
}

void VirtualConsole::RefreshScreen()
{
	if(this == VirtualConsoleManager::GetInstance().GetCurrentConsole())
		videoDriver->RefreshScreen();
}

void VirtualConsole::PutCharacter(char c)
{
	videoDriver->PutCharacter(c);
}



