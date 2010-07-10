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


/** @file Thread.h
 *
 */

#ifndef THREAD_H
#define THREAD_H


#include <constants.h>
#include <types.h>
#include <list.h>
#include <Semaphore.h>

using k_std::list;

typedef void (*ThreadFunction)(void *);

/** @struct Registers
 *
 * @brief This defines what the stack looks like after an interrupt
 *
 **/
struct Registers
{
	Registers()
		: ds(0x0), es(0x0), fs(0x0), gs(0x0),
		edi(0x0), esi(0x0), ebp(0x0), esp(0x0), ebx(0x0), edx(0x0), ecx(0x0), eax(0x0),
		int_no(0x0), err_code(0x0),
		eip(0x0), cs(0x0), eflags(0x0), useresp(0x0), ss(0x0),
		v86_es(0x0), v86_ds(0x0), v86_fs(0x0), v86_gs(0x0)
	{ ; }
	
	uint ds, es, fs, gs;      		///< pushed the segs last
	uint edi, esi, ebp, esp, ebx, edx, ecx, eax;	///< pushed by pusha
	uint int_no, err_code;			///< the int number and error code pushed on the stack
	uint eip, cs, eflags, useresp, ss;	///< pushed by the processor automatically
	uint v86_es, v86_ds, v86_fs, v86_gs;	///< Used for V86 Monitor threads
};

/** @class Thread
 *
 * @brief This is a kernel level thread. Each process has at least 1, but possibly more.
 *
 **/
class Thread
{
	friend class ProcessManager;
	friend class Process;
	friend class UserThread;
	friend class KernelThread;
	friend class V86Thread;

public:
	/**
	 * Joins a thread once it has finished.
	 */
	void Join();

	/**
	 * Destroys a thread.
	 * This will remove it from any of the lists it might be in, and also relase it's memory.
	 */
	void Destroy();
	
private:
	/**
	 * Creates a thread for a given process.
	 * @param functionAddress The address of the function to run in the thread.
	 * @param arg A pointer to the arguments for this thread.
	 * @param stackSize The size of the stack for this thread.
	 */
	Thread(ThreadFunction functionAddress,
		void *arg,
		ulong stackSize);
	
	/**
	 * The copy constructor for the thread.
	 * A call to operator= is made.
	 * @param arg The thread to copy.
	 */
	inline Thread(const Thread &arg)
	{ *this = arg; }
		
	/**
	 * The assignment operator.
	 * @param right The thread to assign from.
	 * @return A reference to the thread.
	 */
	Thread &operator=(const Thread &right);
	
	/**
	 * Set the location of the thread.
	 * @param locList The list the thread is located in.
	 * @param loc An iterator into the locList that points to the thread.
	 */
	void SetLocation(const list<Thread*> *locList, const list<Thread*>::iterator &loc);
	
protected:	
	
	virtual ~Thread() { ; }
	
private:
	list<Thread*>			*theList;	///< The list the thread is in (runQueue or semaphore list)
	list<Thread*>::iterator		myLocation;	///< An iterator that points to the thread in theList
	
	uchar		*stackMemory;	///< A pointer to the thread's stack
	ulong		espReg;		///< The stack pointer of the thread
	uint		procID;		///< The ID of the process the thread belongs to
	uint		stackEnd;	///< The end of the stack, used for updating TSS esp0 field
	
	Semaphore	joiningThreads;	///< A semaphore of threads waiting for this one to finish
	
public:
	static const uint	DEFAULT_STACK_SIZE = 0x1000;	// 1 page of memory
	
	enum { KERNEL, USER, V86 };	///< Thread types
};


#endif // Thread.h


