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


/** @file KeyboardDriver.cpp
 *
 */

#include <constants.h>
#include <types.h>
#include <AutoDisable.h>
#include <KeyboardDriver.h>
#include <io_utils.h>
#include <VirtualConsoleManager.h>
#include <ProcessManager.h>


const uchar KeyboardDriver::KEYBOARD_MAP[90] =
{
/* 00 */	0,	0x1B,	'1',	'2',	'3',	'4',	'5',	'6',
/* 08 */	'7',	'8',	'9',	'0',	'-',	'=',	'\b',	'\t',
/* 10 */	'q',	'w',	'e',	'r',	't',	'y',	'u',	'i',
		/* 1Dh is left Ctrl */
/* 18 */	'o',	'p',	'[',	']',	'\n',	0,	'a',	's',
/* 20 */	'd',	'f',	'g',	'h',	'j',	'k',	'l',	';',
		/* 2Ah is left Shift */
/* 28 */	'\'',	'`',	0,	'\\',	'z',	'x',	'c',	'v',
		/* 36h is right Shift */
/* 30 */	'b',	'n',	'm',	',',	'.',	'/',	0,	0,
		/* 38h is left Alt, 3Ah is Caps Lock */
/* 38 */	0,	' ',	0,	KEY_F1,	KEY_F2,	KEY_F3,	KEY_F4,	KEY_F5,
		/* 45h is Num Lock, 46h is Scroll Lock */
/* 40 */	KEY_F6,	KEY_F7,	KEY_F8,	KEY_F9,	KEY_F10,0,	0,	KEY_HOME,
/* 48 */	KEY_UP,	KEY_PGUP,'-',	KEY_LFT,'5',	KEY_RT,	'+',	KEY_END,
/* 50 */	KEY_DN,	KEY_PGDN,KEY_INS,KEY_DEL,0,	0,	0,	KEY_F11,
/* 58 */	KEY_F12
};

const uchar KeyboardDriver::SHIFT_KEYBOARD_MAP[90] =
{
/* 00 */	0,	0x1B,	'!',	'@',	'#',	'$',	'%',	'^',
/* 08 */	'&',	'*',	'(',	')',	'_',	'+',	'\b',	'\t',
/* 10 */	'Q',	'W',	'E',	'R',	'T',	'Y',	'U',	'I',
		/* 1Dh is left Ctrl */
/* 18 */	'O',	'P',	'{',	'}',	'\n',	0,	'A',	'S',
/* 20 */	'D',	'F',	'G',	'H',	'J',	'K',	'L',	':',
		/* 2Ah is left Shift */
/* 28 */	'"',	'~',	0,	'|',	'Z',	'X',	'C',	'V',
		/* 36h is right Shift */
/* 30 */	'B',	'N',	'M',	'<',	'>',	'?',	0,	0,
		/* 38h is left Alt, 3Ah is Caps Lock */
/* 38 */	0,	' ',	0,	KEY_F1,	KEY_F2,	KEY_F3,	KEY_F4,	KEY_F5,
		/* 45h is Num Lock, 46h is Scroll Lock */
/* 40 */	KEY_F6,	KEY_F7,	KEY_F8,	KEY_F9,	KEY_F10,0,	0,	KEY_HOME,
/* 48 */	KEY_UP,	KEY_PGUP,'-',	KEY_LFT,'5',	KEY_RT,	'+',	KEY_END,
/* 50 */	KEY_DN,	KEY_PGDN,KEY_INS,KEY_DEL,0,	0,	0,	KEY_F11,
/* 58 */	KEY_F12
};



KeyboardDriver::KeyboardDriver() : waitOnChar(0)
{
	curChar = 0x0;
}

int KeyboardDriver::Startup()
{
// 	printf("Keyboard being started...\n");
	
	status = ledStatus = 0;
	
	// turn off all LEDs at the start
//	SetLEDs();

	return(0);
}

uchar KeyboardDriver::GetCharacter()
{
	uchar		retVal;
	AutoDisable	lock;	

	
	retVal = curChar;	// atomic check
	
	lock.Enable();
	
	// nothing to read
	if(retVal == 0x0)
		waitOnChar.Wait();	// so wait
	
	retVal = curChar;	// now get the char
	curChar = 0x0;		// so now we're done with it
	
	return(retVal);
}


int KeyboardDriver::IRQSignaled(Registers *regs)
{
	(void)regs;
	
	uchar		scanCode;
	AutoDisable	lock;
	
	scanCode = inb(DATA_REGISTER);	// read and capture the scan code

	lock.Enable();
	
// 	DEBUG("SCAN CODE: %x\n", scanCode);
	
	if(scanCode == 0xFA)	// this is the ACK scan code
		return(0);

	VirtualConsoleManager	&vcm = VirtualConsoleManager::GetInstance();

	if(scanCode & RELEASE_BIT)	// a key was just released
	{
		// turn off this bit, so we get the right index
		scanCode = scanCode & ~RELEASE_BIT;
		
// 		DEBUG("GOT A RELEASE (%x)\n", scanCode);
		
		switch(scanCode)
		{
			case RAW1_CTRL:
				status &= ~KBD_META_CTRL;
				break;
				
			case RAW1_LEFT_SHIFT:
			case RAW1_RIGHT_SHIFT:
				status &= ~KBD_META_SHIFT;
				break;
				
			case RAW1_ALT:
				status &= ~KBD_META_ALT;
				break;
		}
		
// 		DEBUG("STATUS: %x\n", status);
	}
	
	else	// got a key pressed
	{
// 		DEBUG("GOT A KEY PRESS (%x)\n", scanCode);
		
		switch(scanCode)
		{
			case RAW1_CTRL:
				status |= KBD_META_CTRL;
				break;
				
			case RAW1_LEFT_SHIFT:
 			case RAW1_RIGHT_SHIFT:
				status |= KBD_META_SHIFT;
				break;
				
			case RAW1_ALT:
				status |= KBD_META_ALT;
				break;
			
			case RAW1_CAPS_LOCK:
			case RAW1_SCROLL_LOCK:
			case RAW1_NUM_LOCK:
				break;
							
/*			case RAW1_CAPS_LOCK:
				status ^= KBD_META_CAPS;
				ledStatus ^= CAP_LOCK_LED_BIT;
				SetLEDs();
				break;
				
			case RAW1_SCROLL_LOCK:
				status ^= KBD_META_SCRL;
				ledStatus ^= SCR_LOCK_LED_BIT;
				SetLEDs();
				break;
				
			case RAW1_NUM_LOCK:
				status ^= KBD_META_NUM;
				ledStatus ^= NUM_LOCK_LED_BIT;
				SetLEDs();
				break;
*/			
			case KEY_F1:
			case KEY_F2:
			case KEY_F3:
			case KEY_F4:
			case KEY_F5:
			case KEY_F6:
			case KEY_F7:
			case KEY_F8:
			case KEY_F9:
			case KEY_F10:
				// check that both Ctrl & Alt are held down
//				if(!(status & KBD_META_CTRL && status & KBD_META_ALT))
//					break;
		
				vcm.SwitchConsole((scanCode - KEY_F1) + 1);
				break;
			
			case KEY_F11:
				// check that both Ctrl & Alt are held down
//				if(!(status & KBD_META_CTRL && status & KBD_META_ALT))
//					break;
		
				vcm.SwitchConsole(11);
				break;
			
			case KEY_F12:
				// check that both Ctrl & Alt are held down
//				if(!(status & KBD_META_CTRL && status & KBD_META_ALT))
//					break;
		
				vcm.SwitchConsole(12);
				break;
				
			default:	// these characters are buffered and then read from a proc
				if(status & KBD_META_SHIFT || status & KBD_META_CAPS)
					curChar = SHIFT_KEYBOARD_MAP[scanCode];

				else
					curChar = KEYBOARD_MAP[scanCode];
				
				DEBUG_NL("%c", curChar);
				asm("cli");
				
				// wake up everyone waiting on the character
				while(waitOnChar.GetValue() <= 0)
					waitOnChar.Signal();
				
				asm("sti");
				
				break;
		}
		
//		printf("STATUS: %x\n", status);
	}
	
	return(0);
}

int KeyboardDriver::Shutdown()
{
	return(0);
}




