// Interrupts.s
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


// Defines nice ISRs.

.section .text


.global Interrupt0
.type Interrupt0, @function
Interrupt0:
	pushq $32	// int_no
	jmp GlobalHandler

.global Interrupt1
.type Interrupt1, @function
Interrupt1:
	pushq $33	// int_no
	jmp GlobalHandler

.global Interrupt2
.type Interrupt2, @function
Interrupt2:
	pushq $34	// int_no
	jmp GlobalHandler

.global Interrupt3
.type Interrupt3, @function
Interrupt3:
	pushq $35	// int_no
	jmp GlobalHandler

.global Interrupt4
.type Interrupt4, @function
Interrupt4:
	pushq $36	// int_no
	jmp GlobalHandler

.global Interrupt5
.type Interrupt5, @function
Interrupt5:
	pushq $37	// int_no
	jmp GlobalHandler

.global Interrupt6
.type Interrupt6, @function
Interrupt6:
	pushq $38	// int_no
	jmp GlobalHandler

.global Interrupt7
.type Interrupt7, @function
Interrupt7:
	pushq $39	// int_no
	jmp GlobalHandler

.global Interrupt8
.type Interrupt8, @function
Interrupt8:
	pushq $40	// int_no
	jmp GlobalHandler

.global Interrupt9
.type Interrupt9, @function
Interrupt9:
	pushq $41	// int_no
	jmp GlobalHandler

.global Interrupt10
.type Interrupt10, @function
Interrupt10:
	pushq $42	// int_no
	jmp GlobalHandler

.global Interrupt11
.type Interrupt11, @function
Interrupt11:
	pushq $43	// int_no
	jmp GlobalHandler

.global Interrupt12
.type Interrupt12, @function
Interrupt12:
	pushq $44	// int_no
	jmp GlobalHandler

.global Interrupt13
.type Interrupt13, @function
Interrupt13:
	pushq $45	// int_no
	jmp GlobalHandler

.global Interrupt14
.type Interrupt14, @function
Interrupt14:
	pushq $46	// int_no
	jmp GlobalHandler

.global Interrupt15
.type Interrupt15, @function
Interrupt15:
	pushq $47	// int_no
	jmp GlobalHandler




GlobalHandler:
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

	movq 120(%rsp), %rdi
	call InterruptHandler_C


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

	addq $8, %rsp
	// Return to where we came from.

	iretq






