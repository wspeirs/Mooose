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


/** @file Handler.h
 *
 */

#ifndef HANDLER_H
#define HANDLER_H

#include <constants.h>
#include <types.h>
#include <Thread.h>

/** @class Handler
 *
 * @brief Purely virtual class for interrupt handlers.
 *
 **/
class Handler
{
public:
	/**
	 * This function is called when the handler is installed into the system.
	 * @return Return a zero on sucess, or a handler specific error code.
	 */
	virtual int Startup() = 0;
	
	/**
	 * This function is called whenever an interrupt is triggered for this handler.
	 * @return Return a zero on sucess, or a handler specific error code.
	 */
	virtual int Handle(Registers *regs) = 0;
	
	/**
	 * This function is called when the handler is removed from the system.
	 * @return Return a zero on sucess, or a handler specific error code.
	 */
	virtual int Shutdown() = 0;
	
	virtual ~Handler() { ; }
};

/** @class Driver
 *
 * @brief A virtual class that acts as the base for all drivers.
 *
 **/
class Driver : public Handler
{
public:
	/**
	 * This function is called when the driver is installed into the system.
	 * @return Return a zero on sucess, or a handler specific error code.
	 */
	virtual int Startup() = 0;
	
	/**
	 * This function is called whenever an interrupt is triggered for this driver.
	 * 
	 * This function usually should <b>NOT</b> be overwritten. You really want to
	 * overwrite IRQSignaled. If you overwrite this you have to cleanup the interrupt.
	 */
	virtual int Handle(Registers *regs);
	
	/**
	 * This function is called whenever an interrupt is triggered for this driver.
	 * 
	 * This is the function which should perform the action for the interrupt.
	 * @return Return a zero on sucess, or a handler specific error code.
	 */
	virtual int IRQSignaled(Registers *regs) = 0;
	
	/**
	 * This function is called when the driver is removed from the system.
	 * @return Return a zero on sucess, or a handler specific error code.
	 */
	virtual int Shutdown() = 0;
	
	virtual ~Driver() { ; }
};

/**
 * Virtual 86 handler
 */
void V86Handler(Registers *regs);


/** @class NullHandler
 *
 * @brief This class is a null interrupt handler and does nothing.
 *
 **/
class NullHandler : public Handler
{
public:
	NullHandler();	// constructor
	int Startup()  { return(0); }
	int Shutdown() { return(0); }
	int Handle(Registers *regs);

private:
	const char *exception_messages[20];
};

/** @class NullDriver
 *
 * @brief This class is a null driver and does nothing but return.
 *
 **/
class NullDriver : public Driver
{
public:
	int Startup()  { return(0); }
	int Shutdown() { return(0); }
	int IRQSignaled(Registers *regs);
};

#endif
