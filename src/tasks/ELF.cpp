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


/** @file ELF.cpp
 *
 */

#include <constants.h>
#include <types.h>
#include <ELF.h>
#include <mem_utils.h>
#include <Debug.h>

ELF::ELF(uchar *buff)
{
	// copy over into the file's header
	MemCopy(&theFileHeader, buff, GetFileHeaderSize());
	
	theProgHeaders = NULL;
}

ELF::~ELF()
{
	if(theProgHeaders != NULL)
		delete [] theProgHeaders;
}

bool ELF::IsValidFileHeader()
{
	// run through a list of checks to make sure this is valid
	
	if(theFileHeader.hSize != GetFileHeaderSize())
		return false;
	
	if(theFileHeader.ident[0] != 0x7F ||
	   theFileHeader.ident[1] != 'E' ||
	   theFileHeader.ident[2] != 'L' ||
	   theFileHeader.ident[3] != 'F')
		return false;
	
	return true;	// if it falls through, then everything is OK
}

void ELF::ReadProgramHeaders(uchar *buff)
{
	if(theProgHeaders != NULL)	// already read for some reason
		delete [] theProgHeaders;
	
	theProgHeaders = new ProgramHeader[theFileHeader.phCount];
	
	MemCopy(theProgHeaders, buff, GetProgramHeadersSize());
}


/*****************************
Just rip through the program headers and load those segments into memory based on the offset and the size. 
So just read in that data, put it into a buffer and then make a function that will copy that buffer
into that process's memory space and sets the thread's start address
******************************/

void ELF::PrintProgHeadInfo()
{
	if(theProgHeaders == NULL)
		PANIC("HAVEN'T READ HEADERS YET\n", false);
	
	DEBUG("ENTRY: 0x%x\n", theFileHeader.addr);
	
	for(int i=0; i < theFileHeader.phCount; ++i)
	{
		if(theProgHeaders[i].type == 0x00)
			continue;
		DEBUG("TYPE: 0x%x\n", theProgHeaders[i].type);
//		DEBUG("OFFSET: %u\n", theProgHeaders[i].offset);
//		DEBUG("V ADDR: 0x%x\n",  theProgHeaders[i].vaddr);
//		DEBUG("P ADDR: 0x%x\n",  theProgHeaders[i].paddr);
		DEBUG("SEG SIZE: %u\n", theProgHeaders[i].filesz);
		DEBUG("MEM SIZE: %u\n", theProgHeaders[i].memsz);
//		printf("FLAGS: %x\n", theProgHeaders[i].flags);
//		printf("ALIGN: %u\n", theProgHeaders[i].align);
	}
}

