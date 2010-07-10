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


/** @file Serial.h
 *
 */

#ifndef SERIAL_H
#define SERIAL_H


#include <constants.h>
#include <types.h>
#include <string.h>
#include <io_utils.h>
#include <Handler.h>
#include <Devices.h>

using k_std::string;

/** @class SerialDriver
 *
 * @brief 
 *
 **/

class SerialDriver : public CharacterDevice, public Handler
{
public:
	/**
	 * Setup the serial device for the specified port.
	 * @param port One of the specified COM ports.
	 */
	SerialDriver(ushort port);
	
	/**
	 * Inits the COM device.
	 * @return Zero on success.
	 */
	int Startup();
	
	/**
	 * This function is called whenever an interrupt is triggered for this handler.
	 * @return Zero on sucess.
	 */
	int Handle(Registers *regs);
	
	/**
	 * This function is called when the handler is removed from the system.
	 * @return Zero on sucess.
	 */
	int Shutdown() { return 0; }

	/**
	 * Reads bytes from the serial device.
	 * @param address Unused, pass 0;
	 * @param count The number of bytes to read.
	 * @param dest The location to store the bytes that were read.
	 * @return The number of bytes read or -1 on error.
	 */
	int GetCharacters(ulong address, int count, void *dest);
	
	/**
	 * Writes bytes to the serial device.
	 * @param address Unused, pass 0;
	 * @param count The number of bytes to write.
	 * @param dest The location to read the bytes from.
	 * @return The number of bytes written or -1 on error.
	 */
	int PutCharacters(ulong address, int count, void *src);

	/**
	 * Writes a string to the serial device.
	 * @param str The string to write.
	 * @return The number of characters written or -1 on error.
	 */
	int Write(const string &str);

	// CONSTANTS
	static const ushort COM1 = 0x3F8;	///< The COM1 port
	static const ushort COM2 = 0x2F8;	///< The COM2 port
	static const ushort COM3 = 0x3E8;	///< The COM3 port
	static const ushort COM4 = 0x2E8;	///< The COM4 port
	
private:
	ushort	port;
	
};


#endif // Serial.h


