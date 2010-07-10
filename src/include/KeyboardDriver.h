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


/** @file KeyboardDriver.h
 *
 */

#ifndef KEYBOARDDRIVER_H
#define KEYBOARDDRIVER_H


#include <constants.h>
#include <types.h>
#include <Handler.h>
#include <list.h>
#include <io_utils.h>
#include <Semaphore.h>
#include <Singleton.h>

using k_std::list;

// "ASCII" values for non-ASCII keys.
// Function keys:
/*
#define	KEY_F1		0x80
#define	KEY_F2		(KEY_F1 + 1)
#define	KEY_F3		(KEY_F2 + 1)
#define	KEY_F4		(KEY_F3 + 1)
#define	KEY_F5		(KEY_F4 + 1)
#define	KEY_F6		(KEY_F5 + 1)
#define	KEY_F7		(KEY_F6 + 1)
#define	KEY_F8		(KEY_F7 + 1)
#define	KEY_F9		(KEY_F8 + 1)
#define	KEY_F10		(KEY_F9 + 1)
#define	KEY_F11		(KEY_F10 + 1)
#define	KEY_F12		(KEY_F11 + 1)
*/

#define	KEY_F1		0x3B
#define	KEY_F2		(KEY_F1 + 1)
#define	KEY_F3		(KEY_F2 + 1)
#define	KEY_F4		(KEY_F3 + 1)
#define	KEY_F5		(KEY_F4 + 1)
#define	KEY_F6		(KEY_F5 + 1)
#define	KEY_F7		(KEY_F6 + 1)
#define	KEY_F8		(KEY_F7 + 1)
#define	KEY_F9		(KEY_F8 + 1)
#define	KEY_F10		(KEY_F9 + 1)
#define	KEY_F11		0x57
#define	KEY_F12		0x58

// Cursor keys:
#define	KEY_INS		0x90
#define	KEY_DEL		(KEY_INS + 1)
#define	KEY_HOME	(KEY_DEL + 1)
#define	KEY_END		(KEY_HOME + 1)
#define	KEY_PGUP	(KEY_END + 1)
#define	KEY_PGDN	(KEY_PGUP + 1)
#define	KEY_LFT		(KEY_PGDN + 1)
#define	KEY_UP		(KEY_LFT + 1)
#define	KEY_DN		(KEY_UP + 1)
#define	KEY_RT		(KEY_DN + 1)

// Print screen and pause/break:
#define	KEY_PRNT	(KEY_RT + 1)
#define	KEY_PAUSE	(KEY_PRNT + 1)

// These return a value but they could also act as additional meta keys 
#define	KEY_LWIN	(KEY_PAUSE + 1)
#define	KEY_RWIN	(KEY_LWIN + 1)
#define	KEY_MENU	(KEY_RWIN + 1)

// "meta bits"
// 0x0100 is reserved for non-ASCII keys, so start with 0x200:
#define	KBD_META_ALT	0x0200	// Alt is pressed
#define	KBD_META_CTRL	0x0400	// Ctrl is pressed
#define	KBD_META_SHIFT	0x0800	// Shift is pressed
#define	KBD_META_CAPS	0x1000	// CapsLock is on
#define	KBD_META_NUM	0x2000	// NumLock is on
#define	KBD_META_SCRL	0x4000	// ScrollLock is on
#define	KBD_META_ANY	(KBD_META_ALT | KBD_META_CTRL | KBD_META_SHIFT)

#define	RAW1_CTRL		0x1D
#define	RAW1_ALT		0x38

#define	RAW1_LEFT_SHIFT		0x2A
#define	RAW1_RIGHT_SHIFT	0x36

#define	RAW1_CAPS_LOCK		0x3A
#define	RAW1_SCROLL_LOCK	0x46
#define	RAW1_NUM_LOCK		0x45
#define	RAW1_DEL		0x53


/** @class KeyboardDriver
 *
 * @brief A keyboard driver
 *
 **/
class KeyboardDriver : public Driver, public Singleton<KeyboardDriver>
{
public:
	KeyboardDriver();

	int Startup();
	int IRQSignaled(Registers *regs);	// this should be overloaded like Handle is for interrupts
	int Shutdown();

	/// Gets a character from the keyboard
	uchar GetCharacter();

private:
	inline void SetLEDs()	// set the LEDs on the keyboard
	{
		while((inb(CONTROL_REGISTER) & NOT_BUSY) != 0);	// wait for keyboard to be NOT busy
		outb(DATA_REGISTER, SET_LEDS);
		outb(DATA_REGISTER, ledStatus);
	}

	ushort		status;
	ushort		ledStatus;
	uchar		curChar;	// We should really have a ring buffer here
	Semaphore	waitOnChar;

	static const uchar DATA_REGISTER	= 0x60;
	static const uchar CONTROL_REGISTER	= 0x64;
	
	static const uchar RELEASE_BIT		= 0x80;
	static const uchar NOT_BUSY		= 0x02;
	static const uchar SET_LEDS		= 0xED;
	
	static const uchar SCR_LOCK_LED_BIT	= 0x01;
	static const uchar NUM_LOCK_LED_BIT	= 0x02;
	static const uchar CAP_LOCK_LED_BIT	= 0x04;

	static const uchar KEYBOARD_MAP[90];
	static const uchar SHIFT_KEYBOARD_MAP[90];
};


#endif // KeyboardDriver.h


