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


/** @file interrupts.S
 *
 */

#define ASM

#include <i386.h>

# This will define all of the interrupt service routines
# They all use the stub, so in certain cases I push 0 onto the stack as the "error code"
	.global	isr0
	.global	isr1
	.global	isr2
	.global	isr3
	.global	isr4
	.global	isr5
	.global	isr6
	.global	isr7
	.global	isr8
	.global	isr9
	.global	isr10
	.global	isr11
	.global	isr12
	.global	isr13
	.global	isr14
	.global	isr15
	.global	isr16
	.global	isr17
	.global	isr18
	.global	isr19
	.global	isr20
	.global	isr21
	.global	isr22
	.global	isr23
	.global	isr24
	.global	isr25
	.global	isr26
	.global	isr27
	.global	isr28
	.global	isr29
	.global	isr30
	.global	isr31

	.global irq0
        .global irq1
        .global irq2
        .global irq3
        .global irq4
        .global irq5
        .global irq6
        .global irq7
        .global irq8
        .global irq9
        .global irq10
        .global irq11
        .global irq12
        .global irq13
        .global irq14
        .global irq15

	.global s_irq0
	.global s_irq1
	.global s_irq2
	.global s_irq3
	.global s_irq4
	.global s_irq5
	.global s_irq6
	.global s_irq7
	.global s_irq8
	.global s_irq9

	.text

#
# These are for the reserved interrupts, faults, and exceptions
#
isr0:
	push $0
	push $1
	jmp IsrCommonStub
	
isr1:
	push $0
	push $1
	jmp IsrCommonStub

isr2:
	push $0
	push $2
	jmp IsrCommonStub

isr3:
	push $0
	push $3
	jmp IsrCommonStub

isr4:
	push $0
	push $4
	jmp IsrCommonStub

isr5:
	push $0
	push $5
	jmp IsrCommonStub

isr6:
	push $0
	push $6
	jmp IsrCommonStub

isr7:
	push $0
	push $7
	jmp IsrCommonStub

isr8:
	push $8
	jmp IsrCommonStub

isr9:
	push $0
	push $9
	jmp IsrCommonStub

isr10:
	push $10
	jmp IsrCommonStub

isr11:
	push $11
	jmp IsrCommonStub

isr12:
	push $12
	jmp IsrCommonStub

isr13:
	push $13
	jmp IsrCommonStub

isr14:
	push $14
	jmp IsrCommonStub

isr15:
	push $0
	push $15
	jmp IsrCommonStub

isr16:
	push $0
	push $16
	jmp IsrCommonStub

isr17:
	push $0
	push $17
	jmp IsrCommonStub

isr18:
	push $0
	push $18
	jmp IsrCommonStub

isr19:
	push $0
	push $19
	jmp IsrCommonStub

isr20:
	push $0
	push $20
	jmp IsrCommonStub

isr21:
	push $0
	push $21
	jmp IsrCommonStub

isr22:
	push $0
	push $22
	jmp IsrCommonStub

isr23:
	push $0
	push $23
	jmp IsrCommonStub

isr24:
	push $0
	push $24
	jmp IsrCommonStub

isr25:
	push $0
	push $25
	jmp IsrCommonStub

isr26:
	push $0
	push $26
	jmp IsrCommonStub

isr27:
	push $0
	push $27
	jmp IsrCommonStub

isr28:
	push $0
	push $28
	jmp IsrCommonStub

isr29:
	push $0
	push $29
	jmp IsrCommonStub

isr30:
	push $0
	push $30
	jmp IsrCommonStub

isr31:
	push $0
	push $31
	jmp IsrCommonStub

#
# These are for the IRQs
#

#
# This interrupt is special because it is the clock interrupt.
# It needs to be fast and also deals with context switches, so it is written uniquely from the others.
#
irq0:
	pusha			# save general regs (EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI)
	push	%ds		# save the data segment registers
	push	%es
	push	%fs
	push	%gs

	mov $0x10, %ax   # Load the Kernel Data Segment descriptor
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	#
	# NEED TO CHANGE THIS SO WE CAN CALL THE FAULT HANDLER
	# JUST NEED TO SETUP THE CONTEXT DIFFERENTLY FOR THE THREADS AND HAVE THE FAULT HANDLER
	# RETURN THE NEW ESP... ESSENTIALLY DO IT LIKE EVAN DID :-(
	#
	push	%esp			# provide as an argument the current ESP

	call	PerformPreempt		# call the preempt function

	movl	%eax, %esp		# EAX will hold the new esp

	movl	$0x20, %eax
	movl	$0x20, %edx
	outb	%al, %dx

	pop	%gs		# restore the data segment registers
	pop	%fs
	pop	%es
	pop	%ds
	popa			# restore the general registers

	iret		# this will pop: EIP, CS, EFLAGS and get us in the right ring, etc

irq1:
        push $0
        push $33
        jmp IsrCommonStub

irq2:
        push $0
        push $34
        jmp IsrCommonStub

irq3:
        push $0
        push $35
        jmp IsrCommonStub

irq4:
        push $0
        push $36
        jmp IsrCommonStub

irq5:
        push $0
        push $37
        jmp IsrCommonStub

irq6:
        push $0
        push $38
        jmp IsrCommonStub

irq7:
        push $0
        push $39
        jmp IsrCommonStub

irq8:
        push $0
        push $40
        jmp IsrCommonStub

irq9:
        push $0
        push $41
        jmp IsrCommonStub

irq10:
        push $0
        push $42
        jmp IsrCommonStub

irq11:
        push $0
        push $43
        jmp IsrCommonStub

irq12:
        push $0
        push $44
        jmp IsrCommonStub

irq13:
        push $0
        push $45
        jmp IsrCommonStub

irq14:
        push $0
        push $46
        jmp IsrCommonStub

irq15:
        push $0
        push $47
        jmp IsrCommonStub
        
#
# These are for software interrupts
#

s_irq0:
        push $0
        push $S_IRQ_0
        jmp IsrCommonStub

s_irq1:
        push $0
        push $49
        jmp IsrCommonStub

s_irq2:
        push $0
        push $50
        jmp IsrCommonStub

s_irq3:
        push $0
        push $51
        jmp IsrCommonStub

s_irq4:
        push $0
        push $52
        jmp IsrCommonStub

s_irq5:
        push $0
        push $53
        jmp IsrCommonStub

s_irq6:
        push $0
        push $54
        jmp IsrCommonStub

s_irq7:
        push $0
        push $55
        jmp IsrCommonStub

s_irq8:
        push $0
        push $56
        jmp IsrCommonStub

s_irq9:
        push $0
        push $57
        jmp IsrCommonStub

#
# This is the common stub for all INTs and IRQs with a few exceptions
#
IsrCommonStub:
    pusha
    push %ds
    push %es
    push %fs
    push %gs
    
    mov $0x10, %ax   # Load the Kernel Data Segment descriptor
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    
    push %esp		# Push the Register pointer onto the stack
    
    call fault_handler	# call the fault handler code
    
    pop %eax
    
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popa
    
    add $8, %esp   # Cleans up the pushed error code and pushed ISR number
		    
    iret           # pops in this order: EIP, CS, EFLAGS

