// HandlePS2Interrupt.s
// Copyright (c) 2013 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

.type ASM_HandlePS2IRQ1, @function
.type ASM_HandlePS2IRQ12, @function

.global ASM_HandlePS2IRQ1
.global ASM_HandlePS2IRQ12


ASM_HandlePS2IRQ1:
	push %r15
	push %r14
	push %r13
	push %r12
	push %r11
	push %r10
	push %r9
	push %r8
	push %rdx
	push %rcx
	push %rbx
	push %rax
	push %rbp
	push %rsi
	push %rdi


	call HandleIRQ1

	mov $0x20, %al
	outb %al, $0x20

	pop %rdi
	pop %rsi
	pop %rbp
	pop %rax
	pop %rbx
	pop %rcx
	pop %rdx
	pop %r8
	pop %r9
	pop %r10
	pop %r11
	pop %r12
	pop %r13
	pop %r14
	pop %r15
	iretq

ASM_HandlePS2IRQ12:
	push %r15
	push %r14
	push %r13
	push %r12
	push %r11
	push %r10
	push %r9
	push %r8
	push %rdx
	push %rcx
	push %rbx
	push %rax
	push %rbp
	push %rsi
	push %rdi

	call HandleIRQ12

	mov $0xA0, %al
	outb %al, $0x20

	mov $0x20, %al
	outb %al, $0x20

	pop %rdi
	pop %rsi
	pop %rbp
	pop %rax
	pop %rbx
	pop %rcx
	pop %rdx
	pop %r8
	pop %r9
	pop %r10
	pop %r11
	pop %r12
	pop %r13
	pop %r14
	pop %r15
	iretq
