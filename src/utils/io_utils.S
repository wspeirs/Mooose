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


# For reading in from I/O ports

        .globl  inb
inb:    movl    4(%esp),%edx
        xorl    %eax,%eax       # clr eax
        inb     %dx,%al
        ret

        .globl  inw
inw:    movl    4(%esp),%edx
        xorl    %eax,%eax       # clr eax
        inw     %dx,%ax
        ret

        .globl  inl
inl:    movl    4(%esp),%edx
        xorl    %eax,%eax
        inl     %dx,%eax
        ret
			
	.globl	insb
insb:	pushl   %edi
	movw    8(%esp),%dx
	movl    12(%esp),%edi
	movl    16(%esp),%ecx
	cld
	rep	insb
	movl	%edi, %eax
	popl	%edi
	ret

	.globl	insw
insw:	pushl   %edi
	movw    8(%esp),%dx
	movl    12(%esp),%edi
	movl    16(%esp),%ecx
	cld
	rep	insw
	movl	%edi, %eax
	popl	%edi
	ret


# For writing out to I/O ports

        .globl  outb
outb:   movl    4(%esp),%edx
        movl    8(%esp),%eax
        outb    %al,%dx
        ret

        .globl  outw
outw:   movl    4(%esp),%edx
        movl    8(%esp),%eax
        outw    %ax,%dx
        ret

        .globl  outl
outl:   movl    4(%esp),%edx
        movl    8(%esp),%eax
        outl    %eax,%dx
        ret

       .globl  outsb
outsb:
        pushl   %esi
        movw    8(%esp),%dx
        movl    12(%esp),%esi
        movl    16(%esp),%ecx
        cld
        rep	outsb
        movl    %esi,%eax
        popl    %esi
        ret

       .globl  outsw
outsw:
        pushl   %esi
        movw    8(%esp),%dx
        movl    12(%esp),%esi
        movl    16(%esp),%ecx
        cld
        rep	outsw
        movl    %esi,%eax
        popl    %esi
        ret


