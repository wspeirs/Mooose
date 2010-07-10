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


/** @file InterruptManager.h
 *
 */

#ifndef INTERRUPTMANAGER_H
#define INTERRUPTMANAGER_H


#include <constants.h>
#include <types.h>
#include <map.h>
#include <Handler.h>
#include <Singleton.h>

using k_std::map;
using k_std::pair;

extern "C"
{
	// for reserved interrupts
	extern void isr0();
	extern void isr1();
	extern void isr2();
	extern void isr3();
	extern void isr4();
	extern void isr5();
	extern void isr6();
	extern void isr7();
	extern void isr8();
	extern void isr9();
	extern void isr10();
	extern void isr11();
	extern void isr12();
	extern void isr13();
	extern void isr14();
	extern void isr15();
	extern void isr16();
	extern void isr17();
	extern void isr18();
	extern void isr19();
	extern void isr20();
	extern void isr21();
	extern void isr22();
	extern void isr23();
	extern void isr24();
	extern void isr25();
	extern void isr26();
	extern void isr27();
	extern void isr28();
	extern void isr29();
	extern void isr30();
	extern void isr31();
	
	// For IRQs
	extern void irq0();
	extern void irq1();
	extern void irq2();
	extern void irq3();
	extern void irq4();
	extern void irq5();
	extern void irq6();
	extern void irq7();
	extern void irq8();
	extern void irq9();
	extern void irq10();
	extern void irq11();
	extern void irq12();
	extern void irq13();
	extern void irq14();
	extern void irq15();

	// For software IRQs
	extern void s_irq0();
	extern void s_irq1();
	extern void s_irq2();
	extern void s_irq3();
	extern void s_irq4();
	extern void s_irq5();
	extern void s_irq6();
	extern void s_irq7();
	extern void s_irq8();
	extern void s_irq9();
	
}

/** @class InterruptManager
 *
 * @brief This class manages registering and calling the proper interrupt handlers and device drivers.
 *
 **/

class InterruptManager : public Singleton<InterruptManager>
{
public:
	/**
	 * The constructor for the interrupt handler.
	 */
	InterruptManager();
	
	/**
	 * Installs all of the assembly stubs.
	 */
	static void InstallStubs();

	/**
	 * Given a Registers structure, calls the handler.
	 * @param regs The registers pushed on the stack during an INT.
	 * @return The return value from the interrupt handler.
	 */
	int Dispatcher(Registers *regs);
	
	/**
	 * Install an interrupt handler.
	 * 
	 * This in function calls the handler's startup function.
	 * @param theHandler A pointer the the interrupt handler.
	 * @param interruptNumber The interrupt number to install the handler for.
	 * @return The return status of the startup call.
	 */
	int InstallHandler(Handler *theHandler, uchar interruptNumber);

	/**
	 * Install an interrupt handler.
	 * 
	 * This in function does <b>NOT</b> call the handler's startup function.
	 * @param theHandler A pointer the the interrupt handler.
	 * @param interruptNumber The interrupt number to install the handler for.
	 * @return The return status of the startup call.
	 */
	void InstallHandlerNoStartup(Handler *theHandler, uchar interruptNumber);

	/**
	 * Removes an interrupt handler.
	 * 
	 * This removes a handler and calls the handler's shutdown function.
	 * @param interruptNumber The interrupt number to install the handler for.
	 * @return The return status of the shutdown call.
	 */
	int RemoveHandler(uchar interruptNumber);

private:
	static void SetStub(uchar num, ulong addr, uint desPrivLevel, uint present);

	map<uchar, Handler*>	interruptHandlers;	///< Holds all the interrupt handlers
};

#endif // InterruptManager.h


