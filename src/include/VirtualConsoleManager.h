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


/** @file VirtualConsoleManager.h
 *
 */

#ifndef VIRTUALCONSOLEMANAGER_H
#define VIRTUALCONSOLEMANAGER_H

#include <constants.h>
#include <types.h>
#include <Singleton.h>
#include <VirtualConsole.h>
#include <vector.h>

using k_std::vector;

/** @class VirtualConsoleManager
 *
 * @brief Enables consoles on all of the F keys. F12 is used for debug output.
 *
 **/
class VirtualConsoleManager : public Singleton<VirtualConsoleManager>
{
public:
	/**
	 * The default constructor the establishes all the VirtualConsoles.
	 */
	VirtualConsoleManager();

	/**
	 * Switches from one console to another refreshing the screen.
	 * @param consoleNumber The number of the console to switch to.
	 */
	void SwitchConsole(uint consoleNumber);
	
	/**
	 * Returns a pointer to the current console.
	 * @return A VirtualConsole pointer pointing to the current console.
	 */
	inline VirtualConsole *GetCurrentConsole()
	{ return(theVirtualConsoles[currentConsole-1]); }
	
	/**
	 * Returns the debug console.
	 * @return A VirtualConsole pointer pointing to the debug console.
	 */
	inline VirtualConsole *GetDebugConsole()
	{ return(theVirtualConsoles[NUM_VIRTUAL_CONSOLES-1]); }
	
	/**
	 * Returns a VirtualConsole pointer given a console number.
	 * @param num A valid console number (1 - GetMaxConsoleNumer).
	 * @return A pointer to a the VirtualConsole that corresponds to the number.
	 */
	VirtualConsole *GetVirtualConsole(int num);
	
	/**
	 * Returns the maximum console number.
	 * @return The max console number.
	 */
	inline int GetMaxConsoleNumber(void)
	{ return NUM_VIRTUAL_CONSOLES; }

private:
	vector<VirtualConsole*>		theVirtualConsoles;	///< A vector of consoles
	uint	currentConsole;		///< The index into the vector for the current console
	static const int NUM_VIRTUAL_CONSOLES = 12;	///< The max number of consoles, one for each F key
};


#endif // VirtualConsoleManager.h


