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


/** @file kmain.cpp
 *
 */

#include <constants.h>
#include <types.h>
#include <i386.h>
#include <ELF.h>
#include <string.h>
#include <ext2.h>
// managers
#include <InterruptManager.h>
#include <VirtualConsoleManager.h>
#include <ATAManager.h>
#include <ProcessManager.h>
#include <PhysicalMemManager.h>
// drivers
#include <ATADriver.h>
#include <ClockDriver.h>
#include <KeyboardDriver.h>
#include <Preempt.h>
#include <SystemCallHandler.h>
#include <mem_manage.h>
#include <VesaVideo.h>

using k_std::string;

// debug
#include <mem_manage.h>

// Protos for internal threads
void ShellMain(void *arg);
void BlankConsole();
void CreateProcFromFile(string path, string procName);

void LoadProcess(void *test);

/// This is the main function of the kernel.
/// Final setup is performed here and then threads are run via the process scheduler
extern "C" void kmain(void)
{
	InterruptManager	&theInterruptManager = InterruptManager::GetInstance();
	
	//
	// Create and install all of the standard drivers
	// All of these drivers are singletons because it doesn't make sense 
	// to have more then 1 of them
	// 
/*	printf("Installing clock driver...");
	// install at IRQ 0
	theInterruptManager.InstallHandler(&ClockDriver::GetInstance(), IRQ_0);
	printf("DONE\n");
*/	
	printf("Installing keyboard driver...");
	// install at IRQ 1
	theInterruptManager.InstallHandler(&KeyboardDriver::GetInstance(), IRQ_1);
	printf("DONE\n");
	
/*	printf("Installing ATA driver...");
	// install at IRQ 14 & IRQ 15, so don't call startup on the first one
	theInterruptManager.InstallHandlerNoStartup(&ATAManager::GetInstance(), IRQ_14);
	theInterruptManager.InstallHandler(&ATAManager::GetInstance(), IRQ_15);
	printf("DONE\n");
	
	printf("Installing System Call driver...");
	// install at SOFT IRQ 0
	theInterruptManager.InstallHandler(&SystemCallHandler::GetInstance(), S_IRQ_0);	
	printf("DONE\n");
*/	
	//
	// Create the managers
	//
	printf("Creating the virtual console manager...");
	VirtualConsoleManager	&theConsoleManager = VirtualConsoleManager::GetInstance();
	printf("DONE\n");
	
	
/*	printf("Creating the file system manager...");
	FileSystemManager	&theFileSystemManager = FileSystemManager::GetInstance();
	printf("DONE\n");
	
	// install the ext2 file system
	theFileSystemManager.InstallFileSystem(string("ext2"), new FileSystemFactoryT<ext2>);

	// mount the first partition on the first device as ext2
	theFileSystemManager.MountFileSystem("/dev/hda1", "/", "ext2", 0, NULL);
// 	theFileSystemManager.MountFileSystem("/dev/hdb1", "/", "ext2", 0, NULL);
*/	

	asm("cli");	// so we don't preempt

	printf("Creating the process manager...");
	ProcessManager		&theProcessManager = ProcessManager::GetInstance();
	printf("DONE\n");
	
	theConsoleManager.GetVirtualConsole(1)->SignalConsoleSemaphore();
	
	//
	// VESA Test
	//
	string	*testString = new string("*** Vesa Test ***");
	
	theProcessManager.CreateThread(VesaTest, testString, Thread::KERNEL);
	
	asm("sti");	// turn on interrupts
	
	theProcessManager.PerformTaskSwitch();
	
	while(1);
	
	
	
	//
	// User-app test
	//
	CreateProcFromFile(string("/bin/fork_test"), string("fork_test")); // loads an app from disk
		
	//
	// KERNEL SHELL
	//
/*	theProcessManager.CreateThread(ShellMain,
			NULL,
			Thread::KERNEL);
*/
		
	
	asm("sti");	// turn on interrupts
	
	theProcessManager.PerformTaskSwitch();
	
	// we should never get back here
	Panic::PrintMessage("GOT BACK TO KMAIN\n");
}

void LoadProcess(void *test)
{
	;
}

//
// This should be put into some type of loader class or something
//
void CreateProcFromFile(string path, string procName)
{
	//
	// Read in the INIT ELF and then load it as a proc
	//
	FileSystemManager	&fsMan = FileSystemManager::GetInstance();
	
	// open the ELF file... always located in the same spot
	FileDescriptorBase *fd = fsMan.kOpen(path, 0);
	
	if(fd < NULL)
	{
		printf("COULDN'T OPEN %s\n", path.c_str());
		Panic::HaltMachine();
	}
	
	printf("OPENED FILE\n");
	
	// get the file's size
	int	fileSize = fd->GetFileSystem()->Seek(fd, 0, ext2::SEEK_END);
	
	printf("FILE SIZE: %d\n", fileSize);
	
	// move back to the start
	fd->GetFileSystem()->Seek(fd, 0, ext2::SEEK_SET);
	
	// make memory for the file's header
	uchar	*buff = new uchar[ELF::GetFileHeaderSize()];
	
	// read the header into memory
	fd->GetFileSystem()->Read(fd, buff, ELF::GetFileHeaderSize());
	
	// make an ELF object
	ELF	progELF(buff);
	
	if(!progELF.IsValidFileHeader())
	{
		// close the file
		fsMan.kClose(fd);
		
		Panic::PrintMessage("Not a valid ELF file\n", false);
	}
	
	printf("Valid ELF file\n");
	
	// make memory for all the program headers
	delete [] buff;
	
	buff = new uchar[progELF.GetProgramHeadersSize()];
	
	printf("About to read: 0x%x\n", buff);
	
	fd->GetFileSystem()->Read(fd, buff, progELF.GetProgramHeadersSize());
	
	progELF.ReadProgramHeaders(buff);
	
	//
	// Load all of the program sectors into memory
	//
	list<ELF::ProgramHeaderInfo>	progSectors;
	
	progELF.GetProgramHeaderList(progSectors);
	
	asm("cli");
	
	printf("PROG ENTRY POINT: %x\n", progELF.GetProcessEntryPoint());
	
	// need to change the parent ID for this at some point
	uint procID = ProcessManager::GetInstance().CreateProcess(procName, 0,
			reinterpret_cast<ThreadFunction>(progELF.GetProcessEntryPoint()),
			NULL);
	
	printf("CREATED PROC: %u\n", procID);
	
	ulong pageDirAddr = ProcessManager::GetInstance().GetProcPageDir(procID);
	
	printf("PAGE DIR ADDR: 0x%x\n", pageDirAddr);
		
	for(list<ELF::ProgramHeaderInfo>::iterator it = progSectors.begin();
		   it != progSectors.end();
		   ++it)
	{
		PhysicalMemManager::GetInstancePtr()->MapMemoryFromFile(fd,
								(*it).offset,
								(*it).size,
								(*it).addr,
								pageDirAddr);
	}
	
	// close the file
	fsMan.kClose(fd);
	
	printf("FILE CLOSED\n");
}

