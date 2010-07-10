// multiboot.h - the header for Multiboot 
/*
 * Copyright (C) 1999, 2001  Free Software Foundation, Inc.
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

/** @file multiboot.h
 * This file contains all of the structures and info for using multiboot.
 */

#ifndef MULTIBOOT_H
#define MULTIBOOT_H

// Macros. 
#define MULTIBOOT_HEADER_MAGIC		0x1BADB002	// The magic number for the Multiboot header 
#define MULTIBOOT_HEADER_FLAGS		0x00000003	// The flags for the Multiboot header
#define MULTIBOOT_BOOTLOADER_MAGIC	0x2BADB002	// The magic number passed by a Multiboot-compliant boot loader
#define STACK_SIZE                      0x80000		// The size of our stack (32KB) CHANGED TO 512KB

// Do not include in boot.S
#ifndef ASM

// The Multiboot header. 
struct MultibootHeader
{
	unsigned long magic;
	unsigned long flags;
	unsigned long checksum;
	unsigned long header_addr;
	unsigned long load_addr;
	unsigned long load_end_addr;
	unsigned long bss_end_addr;
	unsigned long entry_addr;
};

// The symbol table for a.out. (NOT USED)
struct AOutSymbolTable
{
	unsigned long tabsize;
	unsigned long strsize;
	unsigned long addr;
	unsigned long reserved;
};

// The section header table for ELF. 
struct ELFSectionHeaderTable
{
	unsigned long num;
	unsigned long size;
	unsigned long addr;
	unsigned long shndx;
};

// The Multiboot information. 
struct MultibootInfo
{
	unsigned long flags;
	unsigned long mem_lower;
	unsigned long mem_upper;
	unsigned long boot_device;
	unsigned long cmdline;
	unsigned long mods_count;
	unsigned long mods_addr;
	union
	{
		AOutSymbolTable aout_sym;
		ELFSectionHeaderTable elf_sec;
	} u;
	unsigned long mmap_length;
	unsigned long mmap_addr;
};

// The module structure. 
struct ModuleInfo
{
	unsigned long mod_start;
	unsigned long mod_end;
	unsigned long string;
	unsigned long reserved;
};

// The memory map. Be careful that the offset 0 is base_addr_low but no size. 
struct MemoryMap
{
	unsigned long size;
	unsigned long base_addr_low;
	unsigned long base_addr_high;
	unsigned long length_low;
	unsigned long length_high;
	unsigned long type;
};

#endif	// ASM

#endif	// MULTIBOOT_H


