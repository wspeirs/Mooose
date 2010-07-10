# This is the context switch function
# It is tightly coupled with the SetupStack function in Thread.cpp

.text
.globl ContextSwitch

#
# ContextSwitch(oldStackPointer, newStackPointer)
# In this we have to fake an interrupt call
#
ContextSwitch:
	pop	%eax		# grab the return address of right after this function
	pushfl			# save eflags
	push	%cs		# save CS reg
	push	%eax		# save EIP (the return address)

	pusha			# save general regs (EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI)
	push	%ds		# save the data segment registers
	push	%es
	push	%fs
	push	%gs

	movl	%esp, %ebp

	# the location of these args is VERY dependent on what is pushed on the stack

	movl    60(%ebp),%eax	# move the address of the old stack pointer into eax
	movl    %esp,(%eax)	# save the stack pointer register as the old stack pointer
			
	movl    64(%ebp),%eax	# move the address of the new stack pointer into eax
	movl    (%eax),%esp     # set the stack pointer register to this new stack pointer
	
	pop	%gs		# restore the data segment registers
	pop	%fs
	pop	%es
	pop	%ds
	popa			# restore the general registers

	iret		# this will pop: EIP, CS, EFLAGS and get us in the right ring, etc

