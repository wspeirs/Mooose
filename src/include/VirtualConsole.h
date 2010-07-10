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


/** @file VirtualConsole.h
 *
 */

#ifndef VIRTUALCONSOLE_H
#define VIRTUALCONSOLE_H

#include <constants.h>
#include <types.h>
#include <mem_utils.h>
#include <VideoDriver.h>
#include <Semaphore.h>


/** @class VirtualConsole
 *
 * @brief A virtual console.
 *
 **/

class VirtualConsole
{
public:
	/**
	 * The default constructor.
	 * @param vd The video driver to display things on this console.
	 */
	VirtualConsole(VideoDriver *vd);

	/**
	 * Switches the video driver. 
	 * Caller is responsible for freeing any memory allocated for the video driver.
	 * @param vd A pointer to the new video driver to use.
	 */
	void SetVideoDriver(VideoDriver *vd);
	
	/**
	 * The print function for this console.
	 * @param format The format string to print.
	 * @return The number of characters printed to the screen.
	 */
	int printf(const char *format, ...);
	
	/**
	 * Sets the font color.
	 * @param fc The font color.
	 */
	void SetFontColor(VideoDriver::Color fc)
	{ videoDriver->SetFontColor(fc); }

	/**
	 * Writes a character to the screen.
	 * @param c The character to print to the screen.
	 */
	void PutCharacter(const char c);
	
	/**
	 * Clears the console's screen.
	 */
	void ClearScreen(void);
	
	/**
	 * Refreshes the screen only if it's the current screen.
	 */
	void RefreshScreen();

	/**
	 * Removes the last character from the screen.
	 */
// 	void EraseLastCharacter();
	
	/**
	 * Reads a character from the keyboard for this console.
	 * @return The character read from the keyboard.
	 */
	uchar GetCharacter();
	
	/**
	 * Signals the semaphore associated with this console.
	 */
	void SignalConsoleSemaphore();
	
	/**
	 * Waits on the semaphore associated with this console.
	 */
	void WaitSignalConsoleSemaphore();
	
private:
	VideoDriver		*videoDriver;	///< The video driver used to print to the screen	
	Semaphore		consoleSem;	///< to activate this console	
};

#endif // VirtualConsole.h


