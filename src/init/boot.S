// boot.S - bootstrap the kernel 
/* Copyright (C) 1999, 2001  Free Software Foundation, Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. 
 */

/**@file boot.S
 * This file contains all of the bootstrap code needed to get the kernel going
 */

#define ASM     1
#include <multiboot.h>
#include <i386.h>

# these are the global vars for the global descriptor table, etc...
	.data 	# this MUST be data because values are set at compile time

	.globl	gdt	# global descriptor table
	.globl	gdtr	# global descriptor table register

	.globl	idt	# interrupt descriptor table
	.globl	idtr	# interrupt descriptor table register

#	.align 4

gdt:	.space	(SEG_DESC_SIZE*NUM_SEG_DESC)	# must equal number of descriptors times the size of a single descriptor
gdtr:	.word	(SEG_DESC_SIZE*NUM_SEG_DESC)-1	# sizeof _gdt - 1 (in bytes)
	.long	gdt

idt:	.space	(INT_DESC_SIZE*NUM_INT_DESC)	# must equal number of descriptors times the size of a single descriptor
idtr:	.word	(INT_DESC_SIZE*NUM_INT_DESC)-1	# size of _idt -1 (in bytes)
	.long	idt


	.bss	# this is set as bss to save memory, all setup is done at runtime

	.globl	pageDir		# page directory
	.globl	kPageTable	# kernel page table
	.globl 	vPgTable	# virtual mapping of the page tables

	.align	4096		# align on page boundary
pageDir:	.space (PAGE_DIR_SIZE*NUM_PAGE_DIR_ENTRIES)	# size of an entry times the number of entries
kPageTable:	.space (PAGE_TABLE_SIZE*NUM_PAGE_TABLE_ENTRIES)	# first 4 MB of memory
vPgTable:	.space (PAGE_TABLE_SIZE*NUM_PAGE_TABLE_ENTRIES)


	.text	# this is the start of the text segment

	.globl  start, _start

	.align  4 	// Align to 32 bit boundary

multiboot_header:	# multiboot header
	.long   MULTIBOOT_HEADER_MAGIC	# magic
	.long   MULTIBOOT_HEADER_FLAGS	# flags
	.long   -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)	# checksum

#ifndef __ELF__	
	.long   multiboot_header	# header_addr
	.long   _start			# load_addr
	.long   _edata			# load_end_addr
	.long   _end			# bss_end_addr
	.long   multiboot_entry		# entry_addr
#endif

start:
_start:
multiboot_entry:	# multiboot entry point

	pushl   $0	# reset the EFLAGS register
	popf
	
	# need to save the magic number & pointer to multiboot info
	movl	%eax, %esi	# magic number
	movl	%ebx, %edi	# pointer to the multiboot info struct

	# we want to get VM going as soon as possible
	lea	pageDir, %eax		# load the page dir address into eax
	subl	$0xC0000000, %eax	# use the non-VM address
	xor	%ecx, %ecx		# set our counter to zero

pgdirloop:	# zero out the page dir
	movl	$0, (%eax)		# zero out the entry
	add	$0x4, %eax		# increase our page dir entry pointer
	incl	%ecx			# increment the counter
	cmpl	$0x400, %ecx		# see if we did all 1024 = 0x400 entries
	jne	pgdirloop		# jump back if not (can do this because its a relative offset)
	
	lea	pageDir, %eax		# load the page dir address into eax
	subl	$0xC0000000, %eax	# use the non-VM address

	lea	kPageTable, %ebx	# load the page table address into ebx
	subl	$0xC0000000, %ebx	# use the non-VM address

	orl	$0x3, %ebx		# setup the bits for the first page dir entry
	movl	%ebx, (%eax)		# set the first page dir entry

	add	$0xC00, %eax		# move to the 3 GB page dir entry
	movl	%ebx, (%eax)		# set the 3 GB page dir entry

	lea	pageDir, %eax		# load the page dir address into eax
	subl	$0xC0000000, %eax	# use the non-VM address
	movl	%eax, %cr3		# set control register 3 to the page dir

	# now setup our page table
	lea	kPageTable, %ebx	# load the page table address into ebx
	subl	$0xC0000000, %ebx	# use the non-VM address
	xor	%ecx, %ecx		# set our counter to zero

pgtbloop:
	movl	%ecx, %eax		# move the counter into eax
	imul	$0x1000, %eax, %eax	# multiply the counter by the size of a page, to get the physical page address
	orl	$0x3, %eax		# set the bits we want for the page (present, rw)
	movl	%eax, (%ebx)		# move into the proper slot in the table
	add	$0x4, %ebx		# move our page table "pointer" by 4 bytes, one entry
	incl	%ecx			# increment our counter by one
	cmpl	$0x400, %ecx		# see if we did all 1024 = 0x400 entries
	jne	pgtbloop		# jump back if not (can do this because its a relative offset)

	movl	%cr0, %ecx		# read the value from control register 0
	orl	$0x80000000, %ecx	# set the bit to enable paging
	movl	%ecx, %cr0		# enable paging

	lea	stack_set, %ecx		# load the VM address of stack_set
	jmp	*%ecx			# jump to that address
					# once we make this jump we are up at the 3 GB mark and running in high memory
stack_set:

	# set the top of the stack to the memory we allocated
	movl    $(stack + STACK_SIZE), %esp	# set the stack pointer
#	movl    $0xC0300000, %esp	# set the stack pointer to the top of the 4 MB we just allocated

	pushl   %edi	# push the pointer to the multiboot info struct
	pushl   %esi	# push the magic value
	cli
	call    InitMemAndInts	# call init to finish the setup stuff

	# this is delayed so that we can still read the multiboot struct
	lea	pageDir, %eax		# get our address (VM address from here on out)
	movl	$0, (%eax)		# zero out the first page dir entry
	subl	$0xC0000000, %eax	# use the physical address
	movl	%eax, %cr3		# set control register 3 to the page dir
					# this will flush the whole TLB setting things right

	# the global descriptor table is now setup
	mov	$0x10, %ax	# load the index (2) into eax
	mov	%ax, %ds	# load the DS descriptor
	mov	%ax, %es	# load the ES descriptor
	mov	%ax, %fs	# load the FS descriptor
	mov	%ax, %gs	# load the GS descriptor
	mov	%ax, %ss	# load the SS descriptor

	ljmp	$0x8,$gdt1	# load the CS descriptor index (1), this is done by a jump

gdt1:
 	# the interrupt descriptor table is configured in setup
	lidt	idtr		# load the interrupt descriptor table

	call	InitTasks	# This is where tasks are setup and the final drivers installed
				# we should never return from here

loop:   hlt	# create an infinite loop to "halt" the machine
	jmp     loop

	.comm   stack, STACK_SIZE	# our stack area


