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


/** @file Serial.cpp
 *
 */

#include <constants.h>
#include <types.h>
#include <SerialDriver.h>
#include <Debug.h>


SerialDriver::SerialDriver(ushort port)
{
	if(port != COM1)
		PANIC("Unknown COM port!!!", false);

	this->port = port;
}

int SerialDriver::Startup()
{
	// taken from http://www.osdev.org/wiki/Serial_ports
     // !!! Make sure that your CPU speed isn't too FAST: cpu: count=1, ips=500000
	outb(port + 1, 0x00);    // Disable all interrupts
     outb(port + 3, 0x80);    // Enable DLAB (set baud rate divisor)
     outb(port + 0, 0x01);    // Set divisor to 1 (lo byte) 115200 baud
     outb(port + 1, 0x00);    //                  (hi byte)
	outb(port + 3, 0x03);    // 8 bits, no parity, one stop bit
     outb(port + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
     outb(port + 4, 0x0B);    // IRQs enabled, DTR/RTS set

	return 0;
}

int SerialDriver::Handle(Registers *regs)
{
	(void)regs;
	// TODO: Add code to handle the INTs from the serial port
	return 0;	
}
	
int SerialDriver::GetCharacters(ulong address, int count, void *dest)
{
	(void)address;
     uchar *d = reinterpret_cast<uchar*>(dest);

	while(inb(port + 5) & 0x01 == 0);	// wait for the data
	
	for(int i=0; i < count; ++i)
		*(d+i) = inb(port + 5);
	
	return count;
}

int SerialDriver::PutCharacters(ulong address, int count, void *src)
{
	(void)address;
     uchar *s = reinterpret_cast<uchar*>(src);

	for(int i=0; i < count; ++i)
     {
          while(inb(port + 5) & 0x20 == 0);  // wait to enable write
		outb(port, *(s+i));
     }
	
	return count;
}

int SerialDriver::Write(const string &str)
{
	for(string::iterator it = str.begin(); it != str.end(); ++it)
	{
		while(inb(port + 5) & 0x20 == 0);	// wait to enable write
		outb(port, *it);
	}

	return str.size();	
}

