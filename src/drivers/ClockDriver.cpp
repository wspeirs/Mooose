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


/** @file ClockDriver.cpp
 *
 */

#include <constants.h>
#include <types.h>
#include <ClockDriver.h>
#include <io_utils.h>
#include <Debug.h>

#include <ProcessManager.h>

// first entry is zero because the days will account for that month... always one month off
const seconds_t	ClockDriver::SECONDS_PER_MONTH[12] =
{ 0, 2678400, 2419200, 2678400, 2592000, 2678400, 2592000, 2678400, 2678400, 2592000, 2678400, 2592000 };	

int ClockDriver::Startup()
{
	// we want to set our clock to 100Hz
	int divisor = INPUT_CLOCK_RATE / CLOCK_RATE;

	// setup our command
	uchar	command = CHANNEL_0 | RW_LSB_MSB | MODE_SQUARE_WAVE | BIT_COUNTER;

	outb(PIT_COMMAND_REGISTER, command);
	outb(PIT_DATA_REGISTER, divisor & 0xFF);	// LSB first
	outb(PIT_DATA_REGISTER, divisor >> 8);	// then MSB

	// reset our tick counter
	ticks = 0;

	//
	// The actual time is read from the BIOS
	//
	uchar	sec, min, hour, dom, month, year;
	
	do{ outb(RTC_COMMAND_REGISTER, STATUS_REGISTER_A); }
	while(inb(RTC_DATA_REGISTER) == BUSY_STATUS);
	
	outb(RTC_COMMAND_REGISTER, SECONDS_COMMAND);
	sec = inb(RTC_DATA_REGISTER);
	
	outb(RTC_COMMAND_REGISTER, MINUTES_COMMAND);
	min = inb(RTC_DATA_REGISTER);
	
	outb(RTC_COMMAND_REGISTER, HOURS_COMMAND);
	hour = inb(RTC_DATA_REGISTER);
	
	outb(RTC_COMMAND_REGISTER, DAY_OF_MONTH_COMMAND);
	dom = inb(RTC_DATA_REGISTER);
	
	outb(RTC_COMMAND_REGISTER, MONTH_COMMAND);
	month = inb(RTC_DATA_REGISTER);
	
	outb(RTC_COMMAND_REGISTER, YEAR_COMMAND);
	year = inb(RTC_DATA_REGISTER);
	
/*	DEBUG("BIOS TIME...\n");
	DEBUG("SEC: %x  %d\n", sec, (sec>>4)*10 + (sec&0x0F));
	DEBUG("MIN: %x  %d\n", min, ( (min>>4)*10 + (min&0x0F) ) * 60);
	DEBUG("HOUR: %x  %d\n", hour, ( (hour>>4)*10 + (hour&0x0F) ) * 60 * 60);
	DEBUG("DOM: %x  %d\n", dom, ( (dom>>4)*10 + (dom&0x0F) ) * 24 * 60 * 60);
	DEBUG("MONTH: %x  %d\n", month, SECONDS_PER_MONTH[ ( (month>>4)*10 + (month&0x0F) ) - 1 ]);
	DEBUG("YEAR: %x  %d\n", year, ( (year>>4)*10 + (year&0x0F) ) * 365 * 24 * 60 * 60);
*/	
	// need to convert from BCD
	seconds = (sec>>4)*10 + (sec&0x0F);
	seconds += ( (min>>4)*10 + (min&0x0F) ) * 60;
	seconds += ( (hour>>4)*10 + (hour&0x0F) ) * 60 * 60;
	seconds += ( (dom>>4)*10 + (dom&0x0F) ) * 24 * 60 * 60;
	seconds += SECONDS_PER_MONTH[ ( (month>>4)*10 + (month&0x0F) ) - 1 ];
	seconds += ( (year>>4)*10 + (year&0x0F) ) * 365 * 24 * 60 * 60;
	
//	DEBUG("SECONDS: %d\n", seconds);

	return(0);
}

int ClockDriver::IRQSignaled(Registers *regs)
{
	(void)regs;

	ticks++;	// increase out tick count
	
	if(ticks % 100 == 0)	// increase our seconds counter
	{
//		DEBUG("SECOND\n");
		seconds++;
	}

	return(0);
}

int ClockDriver::Shutdown()
{
	// at some point this should be setup to reset the clock to the default
	DEBUG("Clock being shutdown...\n");
	return(0);
}

void ClockDriver::GetTime(uchar &sec, uchar &min, uchar &hour, uchar &dom, uchar &month, uint &year)
{
	year = (seconds / (365 * 24 * 60 * 60)) + 2000;
	
	seconds_t	tmp = seconds % (365 * 24 * 60 * 60);
	
	// a bit of work to calculate the month (if seconds isn't correct this won't work!!!)
	for(month = 1; tmp > SECONDS_PER_MONTH[month]; tmp -= SECONDS_PER_MONTH[month], ++month);
	
	dom = tmp / (24 * 60 * 60);
	tmp = tmp % (24 * 60 * 60);
	
	hour = tmp / (60 * 60);
	tmp = tmp % (60 * 60);
	
	min = tmp / 60;
	sec = tmp % 60;
}

