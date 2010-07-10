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


/** @file SystemCallHandler.h
 *
 */

#ifndef SYSTEMCALLHANDLER_H
#define SYSTEMCALLHANDLER_H


#include <constants.h>
#include <types.h>
#include <Handler.h>
#include <vector.h>
#include <Singleton.h>
#include <syscalls.h>

using k_std::vector;

#define SYSTEM_CALL_INT		S_IRQ_0	// register system calls as the first software IRQ
#define MAX_SYS_CALL_NUM	SYSCALL_inotify_rm_watch + 1

typedef void(*VoidFunPtr)();

/** @class SystemCallHandler
 *
 * @brief Handles the interrupts that occur from system calls.
 *
 **/
class SystemCallHandler : public Handler, public Singleton<SystemCallHandler>
{
private:
	typedef int(*Fun0ArgPtr_t)();
	typedef int(*Fun1ArgPtr_t)(uint);
	typedef int(*Fun2ArgPtr_t)(uint, uint);
	typedef int(*Fun3ArgPtr_t)(uint, uint, uint);
	typedef int(*Fun4ArgPtr_t)(uint, uint, uint, uint);
	typedef int(*Fun5ArgPtr_t)(uint, uint, uint, uint, uint);
	typedef int(*Fun6ArgPtr_t)(uint, uint, uint, uint, uint, uint);

public:
	SystemCallHandler();	// default constructor
	
	/**
	 * Installs a system call.
	 * 
	 * @param num The system call number... should start with SYSCALL_
	 * @param add The address of the system call
	 * @param numArgs The number of arguments for the system call
	 */
	void InstallSystemCall(uint num, VoidFunPtr addr, int numArgs);
	
	/**
	 * Removes a system call.
	 * 
	 * @param num System call number... should start with SYSCALL_
	 */
	void RemoveSystemCall(uint num);
	
	int Startup();
	int Shutdown();
	int Handle(Registers *regs);
	
	static void NullSystemCall();

private:
	
	class SystemCall
	{
	public:
		SystemCall() { ; }
		SystemCall(VoidFunPtr a, int n) : addr(a), numArgs(n) { ; }
		
		VoidFunPtr	addr;		// address of the actual call
		int		numArgs;	// number of arguments for this call
	};
	
	SystemCall		nullCall;
	vector<SystemCall>	systemCallTable;
};

class NullSystemCall
{
public:
	static void NullCall();
	
private:
		
};

#endif // SystemCallHandler.h


