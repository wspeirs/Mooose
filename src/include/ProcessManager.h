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


/** @file ProcessManager.h
 *
 */

#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H


#include <constants.h>
#include <types.h>
#include <vector.h>
#include <list.h>
#include <Thread.h>
#include <Process.h>
#include <VirtualConsoleManager.h>
#include <Singleton.h>

#define KERNEL_PID	0

using k_std::vector;
using k_std::list;

// function prototype for the assembly function to do context switching
extern "C" void ContextSwitch(ulong *oldStackPointer, ulong *newStackPointer);

// this is the function threads return to
void ThreadReturn();

/** @class ProcessManager
 *
 * @brief The process manager that takes care of scheduling the next process, process creation, deletion, etc.
 *
 **/
class ProcessManager : public Singleton<ProcessManager>
{
	// allow semaphores to modify the runQueue
	friend class Semaphore;
	friend void ThreadReturn();
	
public:
	/**
	 * The default constructor.
	 * Sets up the process manager and creates a null process.
	 */
	ProcessManager();
	
	/**
	 * Creates a user process in the system and also a user thread for that process.
	 * @param name The name of the process.
	 * @param parentID The ID of the parent process.
	 * @param functionAddress The address of the function to run in the main thread.
	 * @param arg A pointer to the arguments for the thread.
	 * @param console A pointer to the virtual console for this process. Defaults to the current console.
	 * @param stackSize The size of the stack for this thread. Defaults to DEFAULT_STACK_SIZE.
	 * @return Returns the newly created process's ID or a negative number as an error.
	 */
	int CreateProcess(string name,
			  uint parentID,
			  ThreadFunction functionAddress,
			  void *arg,
			  VirtualConsole *console = VirtualConsoleManager::GetInstance().GetCurrentConsole(),
			  ulong stackSize = Thread::DEFAULT_STACK_SIZE);	
	
	/**
	 * Creates a thread for a given process.
	 * @param functionAddress The address of the function to run in the main thread.
	 * @param arg A pointer to the arguments for the thread.
	 * @param procID The process to add this thread to. Defaults to the current process.
	 * @param isUser A bool indicating if this is a user thread. Defaults to true.
	 * @param stackSize The size of the stack for this thread. Defaults to DEFAULT_STACK_SIZE.
	 * @param regs The default registers.
	 * @return Returns the newly created thread's pointer.
	 */
	Thread *CreateThread(ThreadFunction functionAddress,
			     void *arg,
			     uchar type = Thread::USER,
			     uint procID = GetInstance().GetCurrentProcID(),
			     ulong stackSize = Thread::DEFAULT_STACK_SIZE,
			     Registers *regs = NULL);
			     
	/**
	 * Exits a process (SYSCALL_exit)
	 * @param status The value status is returned to the parent process
	 */
	static void Exit(int status);
	
	/**
	 * Forks a new process (SYSCALL_fork)
	 * The new process is exactly the same as the old one,
	 * but with a new virtual memory space.
	 * @return The new process ID.
	 */
	static uint Fork(void);
	
	/**
	 * Destroys a process
	 * @param procID The process ID to destroy
	 */
	void DestroyProcess(uint procID);
	
	/// Schedule and actually switch the context
	void PerformTaskSwitch();
	
	/**
	 * Schedule the next process to run
	 * The task switch should ocurr right after this call
	 * @param oldESP The stack pointer of the old process
	 * @param newESP The stack pointer of the new process
	 */
	void ScheduleNextProcess(ulong **oldESP, ulong **newESP);
	
	/// Returns the currently executing thread
	inline Thread *GetCurrentThread() { return *curThreadIterator; }
	
	/// Returns the currently running process ID
	inline uint GetCurrentProcID() { return curProcID; }
	
	/// Returns the currently running process
	inline Process *GetCurrentProc() { return theProcs[curProcID]; }
	
	/// Returns the address of a process's page directory
	ulong GetProcPageDir(uint procID);
	
	/**
	 * Inserts a file descriptor into the process.
	 * @param ptr A pointer to the file descriptor.
	 * @return The index of the file descriptor.
	 */
	int InsertFileDescriptor(FileDescriptorBase* ptr);
	
	/**
	 * Returns a file descriptor pointer for a given index.
	 * @param fd The index of a file descriptor.
	 * @return The file descriptor pointer.
	 */
	FileDescriptorBase *GetFileDescriptor(int fd);
	
	/**
	 * Removes a file descriptor from the process.
	 * @param fd The index of a file descriptor.
	 */
	void RemoveFileDescriptor(int fd);
	
	/**
	 * Returns the size of the run queue.
	 * @return The size of the run queue.
	 */
	uint GetRunQueueSize()
	{ return runQueue.size(); }

private:
	
	vector<Process*>		theProcs;
	list<Thread*>			runQueue;
	list<Thread*>::iterator		curThreadIterator;
	ulong				*curStackPointer;
	uint				curProcID;

	struct IfNotNull
	{
		bool operator()(const Thread *arg)
		{ return arg != reinterpret_cast<Thread*>(NULL); }
	};
};


#endif // ProcessManager.h


