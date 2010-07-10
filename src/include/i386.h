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

/** @file i386.h
 *
 */

#ifndef I386_H
#define I386_H

// Global descriptor table defines
#define	NUM_SEG_DESC	6	//  1 null, 2 for kernel, 2 for user, 1 TSS
#define SEG_DESC_SIZE	8

// Interrupt descriptor table defines
#define INT_DESC_SIZE		8
#define	NUM_RESERVED_INTERRUPTS	32
#define NUM_IRQS		16
#define NUM_SOFT_IRQS		10
//#define NUM_INT_DESC		(NUM_RESERVED_INTERRUPTS + NUM_IRQS + NUM_SOFT_IRQS)	// this is the max we can have
#define NUM_INT_DESC		0xFF	// this is the max we can have

// Interrupt entriest
#define INT_DIV_ZERO		0
#define INT_DEBUG		1
#define INT_NON_MASKABLE	2
#define INT_BREAKPOINT		3
#define INT_INTO_OVERFLOW	4
#define INT_OUT_OF_BOUNDS	5
#define INT_INVALID_OPCODE	6
#define INT_NO_COPROCESSOR	7
#define INT_DOUBLE_FAULT	8
#define INT_COPROC_SEG_OVERRUN	9
#define INT_BAD_TSS		10
#define INT_SEGMENT_FAULT	11
#define INT_STACK_FAULT		12
#define INT_GEN_PROTECT_FAULT	13
#define INT_PAGE_FAULT		14
#define INT_UNKNOWN		15
#define INT_COPROCESSOR_FAULT	16
#define INT_ALIGN_CHECK		17
#define INT_MACHINE_CHECK	18

// IRQs
#define IRQ_0		NUM_RESERVED_INTERRUPTS	// 32
#define IRQ_1		(IRQ_0 + 1)	// 33
#define IRQ_2		(IRQ_0 + 2)	// 34
#define IRQ_3		(IRQ_0 + 3)	// 35
#define IRQ_4		(IRQ_0 + 4)	// 36
#define IRQ_5		(IRQ_0 + 5)	// 37
#define IRQ_6		(IRQ_0 + 6)	// 38
#define IRQ_7		(IRQ_0 + 7)	// 39
#define IRQ_8		(IRQ_0 + 8)	// 40
#define IRQ_9		(IRQ_0 + 9)	// 41
#define IRQ_10		(IRQ_0 + 10)	// 42
#define IRQ_11		(IRQ_0 + 11)	// 43
#define IRQ_12		(IRQ_0 + 12)	// 44
#define IRQ_13		(IRQ_0 + 13)	// 45
#define IRQ_14		(IRQ_0 + 14)	// 46
#define IRQ_15		(IRQ_0 + 15)	// 47

// Software IRQs
#define S_IRQ_0		0x80
#define S_IRQ_1		(S_IRQ_0 + 1)
#define S_IRQ_2		(S_IRQ_0 + 2)
#define S_IRQ_3		(S_IRQ_0 + 3)
#define S_IRQ_4		(S_IRQ_0 + 4)
#define S_IRQ_5		(S_IRQ_0 + 5)
#define S_IRQ_6		(S_IRQ_0 + 6)
#define S_IRQ_7		(S_IRQ_0 + 7)
#define S_IRQ_8		(S_IRQ_0 + 8)
#define S_IRQ_9		(S_IRQ_0 + 9)


// virtual memory defines
#define PAGE_DIR_SIZE		4	// in bytes
#define PAGE_TABLE_SIZE		4
#define NUM_PAGE_DIR_ENTRIES	1024	// for all page directory entries
#define NUM_PAGE_TABLE_ENTRIES	1024	// for all page table entries
#define VIRTUAL_OFFSET		0xC0000000
#define	PAGE_SIZE		0x1000
#define KERNEL_BASE_ADDR	0xC0000000
#define PAGE_TABLES_MAPPING	0xFFC00000	// 4 GB - 4 MB
#define PAGE_DIRECTORY_MAPPING	0xFFFFF000	// 4 GB - 4096




#ifndef ASM

#include <types.h>

// See page 5-14 of Intel Vol. 3
struct InterruptDescriptor
{
        ushort  funPtrLow;
        ushort  segSelector;
        uint    reserved 	: 5;
        uint    zero	 	: 3;
        uint    type     	: 5;
        uint    desPrivLevel    : 2;
        uint    present		: 1;
        ushort  funPtrHigh;
} __attribute__((packed));

// See page 3-11 of Intel Vol. 3
struct CodeDataSegmentDescriptor
{
	ushort  segLimit;
	ushort  lowBaseAddr;
	uchar   midBaseAddr;
	uint    accessed      : 1;	// see page 3-13 Intel Vol. 3
	uint    rwEnable      : 1;
	uint    expandConform : 1;
	uint    isCode	      : 1;
	uint    desType	      : 1;
	uint    desPrivLevel  : 2;
	uint    present	      : 1;
	uint    hiLimit	      : 4;
	uint    available     : 1;
	uint    zero          : 1;	// must be '0' 
	uint    opSize        : 1;	
	uint    granularity   : 1;
	uchar   hiBaseAddr;
} __attribute__((packed));

// See page 6-7 of Intel Vol. 3
struct TaskSwitchingSegmentDescriptor
{
	ushort  segLimit;
	ushort  lowBaseAddr;
	uchar   midBaseAddr;
	uint    type          : 4;	
	uint	zero          : 1;
	uint    desPrivLevel  : 2;
	uint    present	      : 1;
	uint    hiLimit	      : 4;
	uint    available     : 1;
	uint    zero2          : 2;	// must be '0' 
	uint    granularity   : 1;
	uchar   hiBaseAddr;
} __attribute__((packed));

// See page 6-5 of Intel Vol. 3
struct TaskStateSegment
{
	ushort	backlink, __blh;
	uint	esp0;
	ushort	ss0, __ss0h;
	uint	esp1;
	ushort	ss1, __ss1h;
	uint	esp2;
	ushort	ss2, __ss2h;
	uint	cr3;
	uint	eip;
	uint	eflags;
	uint	eax;
	uint	ecx;
	uint	edx;
	uint	ebx;
	uint	esp;
	uint	ebp;
	uint	esi;
	uint	edi;
	ushort	es, __esh;
	ushort	cs, __csh;
	ushort	ss, __ssh;
	ushort	ds, __dsh;
	ushort	fs, __fsh;
	ushort	gs, __gsh;
	ushort	ldt, __ldth;
	ushort	trace;
	ushort	iomapbase;
} __attribute__((packed));

// See page 2-10 of Intel Vol. 3
struct SystemTableRegister
{
        uint    len  : 16;
        uint    addr : 32;
} __attribute__((packed));

// See page 3-24 of Intel Vol. 3
struct PageDirectoryEntry
{
	uint	present	      : 1;
	uint	readWrite     : 1;	///< 0 = read-only, 1 = read-write
	uint	userSuper     : 1;	///< 0 = kernel, 1 = user-land
	uint	writeThrough  : 1;
	uint	cacheDis      : 1;
	uint	accessed      : 1;
	uint	reserved      : 1;	///< Set to 0
	uint	pageSize      : 1;	///< 0 indicates 4KBytes
	uint	globalPage    : 1;	///< ignored
	uint	sysProgUse    : 3;	///< avaliable for system programmer's use
	uint	pageTableAddr :	20;
} __attribute__((packed));

// See page 3-24 of Intel Vol. 3
struct PageTableEntry
{
	uint	present	      : 1;
	uint	readWrite     : 1;	///< 0 = read-only, 1 = read-write
	uint	userSuper     : 1;	///< 0 = kernel, 1 = user-land
	uint	writeThrough  : 1;
	uint	cacheDis      : 1;
	uint	accessed      : 1;
	uint	dirty         : 1;	
	uint	reserved      : 1;	///< set to 0
	uint	globalPage    : 1;	
	uint	sysProgUse    : 3;	///< avaliable for system programmer's use
	uint	pageAddr      : 20;
} __attribute__((packed));

#endif

#endif
