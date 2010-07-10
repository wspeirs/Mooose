
init:     file format elf32-i386

Disassembly of section .text:

00500000 <_start>:
  500000:	pop    %ecx
  500001:	mov    %esp,%esi
  500003:	push   %ecx
  500004:	lea    0x4(%esi,%ecx,4),%eax
  500008:	push   %eax
  500009:	push   %esi
  50000a:	push   %ecx
  50000b:	mov    %eax,0x501108
  500010:	mov    %eax,%edx
  500012:	xor    %esi,%esi
  500014:	add    $0x4,%edx
  500017:	cmp    %esi,0xfffffffc(%edx)
  50001a:	jne    500014 <_start+0x14>
  50001c:	mov    (%edx),%edi
  50001e:	test   %edi,%edi
  500020:	je     500033 <_start+0x33>
  500022:	add    $0x8,%edx
  500025:	cmp    $0x20,%edi
  500028:	jne    50001c <_start+0x1c>
  50002a:	mov    0xfffffffc(%edx),%edi
  50002d:	mov    %edi,0x501104
  500033:	call   500040 <main>
  500038:	push   %eax
  500039:	call   50008d <_exit>
  50003e:	hlt    
  50003f:	nop    

00500040 <main>:
  500040:	push   %ebp
  500041:	mov    %esp,%ebp
  500043:	sub    $0x18,%esp
  500046:	and    $0xfffffff0,%esp
  500049:	mov    $0x0,%eax
  50004e:	add    $0xf,%eax
  500051:	add    $0xf,%eax
  500054:	shr    $0x4,%eax
  500057:	shl    $0x4,%eax
  50005a:	sub    %eax,%esp
  50005c:	movl   $0x5000ec,(%esp)
  500063:	call   5000d8 <strlen>
  500068:	mov    %eax,0x8(%esp)
  50006c:	movl   $0x5000ec,0x4(%esp)
  500074:	movl   $0x1,(%esp)
  50007b:	call   5000c8 <__libc_write>
  500080:	mov    $0x0,%eax
  500085:	leave  
  500086:	ret    
  500087:	nop    

00500088 <__unified_syscall_256>:
  500088:	movzwl %ax,%eax
  50008b:	jmp    500092 <__unified_syscall+0x3>

0050008d <_exit>:
  50008d:	mov    $0x1,%al

0050008f <__unified_syscall>:
  50008f:	movzbl %al,%eax
  500092:	push   %edi
  500093:	push   %esi
  500094:	push   %ebx
  500095:	mov    %esp,%edi
  500097:	mov    0x10(%edi),%ebx
  50009a:	mov    0x14(%edi),%ecx
  50009d:	mov    0x18(%edi),%edx
  5000a0:	mov    0x1c(%edi),%esi
  5000a3:	mov    0x20(%edi),%edi
  5000a6:	call   *0x501104
  5000ac:	cmp    $0xffffff84,%eax
  5000af:	jb     5000bf <__unified_syscall+0x30>
  5000b1:	neg    %eax
  5000b3:	mov    %eax,%ebx
  5000b5:	call   5000d0 <__errno_location>
  5000ba:	mov    %ebx,(%eax)
  5000bc:	or     $0xffffffff,%eax
  5000bf:	pop    %ebx
  5000c0:	pop    %esi
  5000c1:	pop    %edi

005000c2 <__you_tried_to_link_a_dietlibc_object_against_glibc>:
  5000c2:	ret    
  5000c3:	int    $0x80
  5000c5:	ret    
  5000c6:	nop    
  5000c7:	nop    

005000c8 <__libc_write>:
  5000c8:	mov    $0x4,%al
  5000ca:	jmp    50008f <__unified_syscall>
  5000cf:	nop    

005000d0 <__errno_location>:
  5000d0:	mov    $0x501110,%eax
  5000d5:	ret    
  5000d6:	nop    
  5000d7:	nop    

005000d8 <strlen>:
  5000d8:	mov    0x4(%esp),%ecx
  5000dc:	xor    %eax,%eax
  5000de:	jecxz  5000e8 <strlen+0x10>
  5000e0:	dec    %eax
  5000e1:	inc    %eax
  5000e2:	cmpb   $0x0,(%ecx,%eax,1)
  5000e6:	jne    5000e1 <strlen+0x9>
  5000e8:	ret    
  5000e9:	nop    
  5000ea:	nop    
  5000eb:	nop    
