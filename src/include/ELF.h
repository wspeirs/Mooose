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


/** @file ELF.h
 *
 */

#ifndef ELF_H
#define ELF_H

#include <types.h>
#include <constants.h>

#include <Debug.h>

/** @class ELF
 *
 * @brief 
 *
 **/

class ELF
{
public:
	ELF(uchar *buff);	// takes a buffer of memory that is the ELF file header
	~ELF();
	
	bool IsValidFileHeader();
	
	void ReadProgramHeaders(uchar *buff);
	
	template<typename Container>
	void GetProgramHeaderList(Container &theList)
	{
		ProgramHeaderInfo	tmpInfo;
	
		if(theProgHeaders == NULL)
			PANIC("HAVEN'T READ HEADERS YET\n", false);
	
		for(int i=0; i < theFileHeader.phCount; ++i)
		{
			if(theProgHeaders[i].memsz == 0)
				continue;
		
			tmpInfo.addr = theProgHeaders[i].vaddr;
			tmpInfo.size = theProgHeaders[i].memsz;
			tmpInfo.offset = theProgHeaders[i].offset;
			
			theList.push_back(tmpInfo);
		}
	}

	inline static int GetFileHeaderSize() { return sizeof(FileHeader); }
	inline int GetProgramHeadersSize() { return sizeof(ProgramHeader) * theFileHeader.phCount; }
	inline ulong GetProcessEntryPoint() { return theFileHeader.addr; }

	
	struct ProgramHeaderInfo
	{
		ulong addr;
		ulong size;
		ulong offset;	
	};
	
	//
	// DEBUG
	//
	void PrintProgHeadInfo();
			
private:
	struct FileHeader
	{
		uchar	ident[16];		// Magic number and other info
		ushort	type;			// Object file type
		ushort	machine;		// Architecture
		ulong	version;		// Object file version
		ulong	addr;			// Entry point virtual address
		ulong	phOffset;		// Program header table file offset
		ulong	shOffset;		// Section header table file offset
		ulong	flags;			// Processor-specific flags
		ushort	hSize;			// ELF header size in bytes
		ushort	phEntSize;		// Program header table entry size
		ushort	phCount;		// Program header table entry count
		ushort	shEntSize;		// Section header table entry size
		ushort	shCount;		// Section header table entry count
		ushort	shStrIndex;		// Section header string table index
		
	} __attribute__((packed));
	
	struct ProgramHeader
	{
		ulong    type;		// Segment type
		ulong    offset;	// Segment file offset
		ulong    vaddr;		// Segment virtual address
		ulong    paddr;		// Segment physical address
		ulong    filesz;	// Segment size in file
		ulong    memsz;		// Segment size in memory
		ulong    flags;		// Segment flags
		ulong    align;		// Segment alignment
	} __attribute__((packed));

	FileHeader	theFileHeader;
	ProgramHeader	*theProgHeaders;

};

#endif // ELF_H


