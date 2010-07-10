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


/** @file Debug.h
 *
 */

#ifndef DEBUG_H
#define DEBUG_H

#define STRINGIFY(x)	#x
#define TOSTRING(x)	STRINGIFY(x)
#define AT		__FILE__ ":" TOSTRING(__LINE__) "  "

//
// DEBUG
//
/*#define DEBUG(...) \
do { vga_printf(AT __VA_ARGS__); } while(0)

#define WARN(...) \
do { vga_printf(AT __VA_ARGS__); } while(0)

#define ERROR(...) \
do { vga_printf(AT __VA_ARGS__); } while(0)

#define PANIC(...) \
do { vga_printf(AT __VA_ARGS__); } while(0)
*/

#define DEBUG_PRINT_ON_ALL	true

//
// For some reason AT __VA_ARGS__ is messing things up on ther serial port
//

/// Prints standard debug info
#define DEBUG(...) \
do { Debug::GetInstance().Print(DEBUG_PRINT_ON_ALL, false, false, __VA_ARGS__); } while(0)

#define DEBUG_NL(...) \
do { Debug::GetInstance().Print(DEBUG_PRINT_ON_ALL, false, false, __VA_ARGS__); } while(0)

/// Prints to all consoles
#define WARN(...) \
do { Debug::GetInstance().Print(true, false, false, __VA_ARGS__); } while(0)

/// Prints to all consoles with a back trace
#define ERROR(...) \
do { Debug::GetInstance().Print(true, true, false, __VA_ARGS__); } while(0)

/// Prints to all consoles and halts the machine
#define PANIC(...) \
do { Debug::GetInstance().Print(true, true, true, __VA_ARGS__); } while(0)


#include <constants.h>
#include <types.h>
#include <string.h>
#include <VirtualConsoleManager.h>
#include <Singleton.h>
#include <SerialDriver.h>

using namespace k_std;


/** @class Debug
 *
 * @brief Provides debug output for the kernel.
 * 
 * All output is printed to VirtualConsole #12 and to COM1.
 *
 **/

class Debug : public Singleton<Debug>
{
public:
	/**
	 * Sets up debug printing.
	 */
	Debug();
	
	/**
	 * Sets the serial device to print debug messages to.
	 * @param com1 The COM device.
	 */
	inline void SetSerial(SerialDriver *com1)
	{ this->com1 = com1; }
	
	/**
	 * Prints a debug message to the screen and serial port.
	 * @param allConsoles True - prints to all consoles, False - only debug.
	 * @param backTrace True - prints the backtrace.
	 * @param halt True - halts the machine.
	 * @param message The printf style message to print.
	 */
	void Print(bool allConsoles, bool backTrace, bool halt, const char *message, ...);
	
	/**
	 * Prints a debug message to the screen and serial port.
	 * @param allConsoles True - prints to all consoles, False - only debug.
	 * @param backTrace True - prints the backtrace.
	 * @param halt True - halts the machine.
	 * @param str The string to print, no formatting is done.
	 */
	inline void Write(bool allConsoles, bool backTrace, bool halt, const char *str)
	{
		Print(allConsoles, backTrace, halt, "%s", str);
	}
	
	/**
	 * Simply disables interrupts and puts the machine into a while loop.
	 * You should prob use PANIC to halt the machine so the user knows why.
	 */
	void HaltMachine();
	
	// TODO: Make a function that can load symbols for the back trace

private:
	SerialDriver		*com1;			///< A pointer to the serial port to write to
	VirtualConsoleManager	&consoleManager;	///< The virtual console manager
	VirtualConsole		*debugConsole;		///< The debug console to write to
	static bool		startedStackDump;	///< To tell if we've already stack traced
	static const int	DEPTH = 10;		///< How many levels to travel on a stack trace
	
	/**
	 * Allows us to print things to the current console from the Debug class.
	 * @param str The string to print.
	 */
	inline void DebugPrint(string str)
	{
		if(com1 != NULL)
			com1->PutCharacters(0, str.size(), const_cast<char*>(str.c_str()));

		debugConsole->printf(str.c_str());
		consoleManager.GetCurrentConsole()->printf(str.c_str());
	}
};


#endif // Debug.h


