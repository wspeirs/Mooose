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


/** @file VirtualConsoleManager.cpp
 *
 */

#include <constants.h>
#include <types.h>
#include <VirtualConsoleManager.h>
#include <VideoDriver.h>
#include <Debug.h>


VirtualConsoleManager::VirtualConsoleManager()
{
	// need to loop through because we want a NEW console for each slot	
	for(int i = 0; i < NUM_VIRTUAL_CONSOLES; ++i)	// make all the consoles
		theVirtualConsoles.push_back(new VirtualConsole(new VGADriver()));
	
	// the last virtual console is our debug screen and will always be RED
	theVirtualConsoles[NUM_VIRTUAL_CONSOLES-1]->SetFontColor(VideoDriver::RED);
	
/*	theVirtualConsoles[0]->SetAttribute(VirtualConsole::FG_WHITE);
	theVirtualConsoles[1]->SetAttribute(VirtualConsole::FG_BLUE);
	theVirtualConsoles[2]->SetAttribute(VirtualConsole::FG_RED);
	theVirtualConsoles[3]->SetAttribute(VirtualConsole::FG_GREEN);
	theVirtualConsoles[4]->SetAttribute(VirtualConsole::FG_YELLOW);
	theVirtualConsoles[5]->SetAttribute(VirtualConsole::FG_BROWN);
	theVirtualConsoles[6]->SetAttribute(VirtualConsole::FG_LIGHT_BLUE);
	theVirtualConsoles[7]->SetAttribute(VirtualConsole::FG_PINK);
	theVirtualConsoles[8]->SetAttribute(VirtualConsole::FG_LIGHT_GREEN);
	theVirtualConsoles[9]->SetAttribute(VirtualConsole::FG_GREEN_BLUE);
	theVirtualConsoles[10]->SetAttribute(VirtualConsole::FG_PURPLE);
	theVirtualConsoles[11]->SetAttribute(VirtualConsole::FG_DARK_GREY);
*/					
	currentConsole = 1;	// set the current console to 1
}

void VirtualConsoleManager::SwitchConsole(uint consoleNumber)
{
	if(consoleNumber > theVirtualConsoles.size())
		PANIC("Tried to switch to an unknown console");
	
	if(consoleNumber == currentConsole)
		return;	// don't need to do anything
	
	theVirtualConsoles[consoleNumber-1]->RefreshScreen();	// refresh the screen
	
	currentConsole = consoleNumber;	// update the current console
	
	// do this last so everything else is synched up
	theVirtualConsoles[consoleNumber-1]->SignalConsoleSemaphore();	// wake up the console
}

// the argument is from 1 to NUM_VIRTUAL_CONSOLES
VirtualConsole *VirtualConsoleManager::GetVirtualConsole(int num)
{
	if(num > NUM_VIRTUAL_CONSOLES)
		PANIC("Tried to get an unknown console");
	
	return(theVirtualConsoles[num-1]);
}


