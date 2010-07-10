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


/** @file gcc_utils.cpp
 *
 */

#include <constants.h>
#include <types.h>
#include <screen_utils.h>

//
// These functions all implement internal functions for G++
//
extern "C" void __cxa_pure_virtual()
{
	// This should never happen...
	vga_printf("Purely virtual function called\n");
	asm("cli");
	while(1);
}

extern "C" int __cxa_atexit(void (*func) (void *), void *arg, void *d)
{
	// don't use these
	(void)func;
	(void)arg;
	(void)d;
	
	return(0);
}

extern "C" void *__dso_handle()
{
	return(NULL);
}



//
// These functions are for setting up the C++ environment
// (Provided by Evan Teran)
//
typedef void(*func_ptr)();

// references to the CTOR and DTOR lists
extern func_ptr __CTOR_LIST__[];
extern func_ptr __DTOR_LIST__[];
extern func_ptr __CTOR_END__[];
extern func_ptr __DTOR_END__[];

void __do_global_ctors()
{
	ulong nptrs = reinterpret_cast<ulong>(__CTOR_LIST__[0]);
	
	if (nptrs == ulong(-1))
	{
		for (nptrs = 0; __CTOR_LIST__[nptrs + 1] != 0; ++nptrs);
	}
	
	for (ulong i = nptrs; i >= 1; i--)
	{
		(__CTOR_LIST__[i])();
	}
}

void __do_global_dtors()
{
	func_ptr f;
	
	for (const func_ptr *p = __DTOR_LIST__ + 1; (f = *p); ++p)
	{
		f();
	}
}
