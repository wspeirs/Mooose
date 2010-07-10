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


/** @file ClockDriver.h
 *
 */

#ifndef CLOCKDRIVER_H
#define CLOCKDRIVER_H


#include <constants.h>
#include <types.h>
#include <Handler.h>
#include <Singleton.h>

/** @class ClockDriver
 *
 * @brief Driver for the real-time clock
 *
 **/
class ClockDriver : public Driver, public Singleton<ClockDriver>
{
public:
	int Startup();
	int IRQSignaled(Registers *regs);	// this should be overloaded like Handle is for interrupts
	int Shutdown();
	
	inline seconds_t GetTimeInSeconds() { return(seconds); }
	void GetTime(uchar &sec, uchar &min, uchar &hour, uchar &dom, uchar &month, uint &year);
	
private:
	ulong		ticks;	// CLOCK_RATE ticks = 1 second
	seconds_t	seconds;	
	
	//
	// These are all for the PIT clock
	//
	static const uchar	PIT_COMMAND_REGISTER = 0x43;
	static const uchar	PIT_DATA_REGISTER    = 0x40;
	
	static const int	INPUT_CLOCK_RATE = 1193180;
	static const int	CLOCK_RATE = 100;
	
	static const uchar	CHANNEL_0 = 0x00;
	static const uchar	CHANNEL_1 = 0x40;
	static const uchar	CHANNEL_2 = 0x80;
	
	static const uchar	RW_LSB     = 0x10;
	static const uchar	RW_MSB     = 0x20;
	static const uchar	RW_LSB_MSB = 0x30;
	
	static const uchar	MODE_TERM_COUNT  = 0x00;
	static const uchar	MODE_ONE_SHOT    = 0x02;
	static const uchar	MODE_RATE_GEN    = 0x04;
	static const uchar	MODE_SQUARE_WAVE = 0x06;
	static const uchar	MODE_SOFT_STROBE = 0x08;
	static const uchar	MODE_HARD_STROBE = 0x0A;
	
	static const uchar	BIT_COUNTER	= 0x00;
	static const uchar	DECADE_COUNTER	= 0x01;

	//
	// These are all for the RTC
	//
	static const uchar	RTC_COMMAND_REGISTER = 0x70;
	static const uchar	RTC_DATA_REGISTER    = 0x71;
	
	static const uchar	STATUS_REGISTER_A	= 0x0A;
	static const uchar	STATUS_REGISTER_B	= 0x0B;
	static const uchar	STATUS_REGISTER_C	= 0x0C;
	static const uchar	BUSY_STATUS		= 0x80;
	
	static const uchar	TIME_FORMAT		= 0x06;
	
	static const uchar	SECONDS_COMMAND		= 0x00;
	static const uchar	MINUTES_COMMAND		= 0x02;
	static const uchar	HOURS_COMMAND		= 0x04;
	static const uchar	DAY_OF_MONTH_COMMAND	= 0x07;
	static const uchar	MONTH_COMMAND		= 0x08;
	static const uchar	YEAR_COMMAND		= 0x09;
	
	static const seconds_t	SECONDS_PER_MONTH[12];
	
};


#endif // ClockDriver.h


