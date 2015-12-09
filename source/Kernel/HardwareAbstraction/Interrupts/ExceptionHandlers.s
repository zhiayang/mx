// Interrupts.s
// Copyright (c) 2013 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


// Defines nice ISRs.

.section .text

.global HAL_AsmLoadIDT
.type HAL_AsmLoadIDT, @function
HAL_AsmLoadIDT:
	lidt (%rdi)
	ret







.global Fault0
.type Fault0, @function
Fault0:
	// mov $0x00600000, %rsp
	pushq $0	// err_code
	pushq $0	// int_no
	jmp GlobalHandler

.global Fault1
.type Fault1, @function
Fault1:
	pushq $0	// err_code
	pushq $1	// int_no
	jmp GlobalHandler

.global Fault2
.type Fault2, @function
Fault2:
	pushq $0	// err_code
	pushq $2	// int_no
	jmp GlobalHandler

.global Fault3
.type Fault3, @function
Fault3:
	pushq $0	// err_code
	pushq $3	// int_no
	jmp GlobalHandler

.global Fault4
.type Fault4, @function
Fault4:
	pushq $0	// err_code
	pushq $4	// int_no
	jmp GlobalHandler

.global Fault5
.type Fault5, @function
Fault5:
	pushq $0	// err_code
	pushq $5	// int_no
	jmp GlobalHandler

.global Fault6
.type Fault6, @function
Fault6:
	pushq $0	// err_code
	pushq $6	// int_no
	jmp GlobalHandler

.global Fault7
.type Fault7, @function
Fault7:
	pushq $0	// err_code
	pushq $7	// int_no
	jmp GlobalHandler

.global Fault8
.type Fault8, @function
Fault8:
				// err_code pushed by CPU
	pushq $8	// int_no
	jmp GlobalHandler

.global Fault9
.type Fault9, @function
Fault9:
	pushq $0	// err_code
	pushq $9	// int_no
	jmp GlobalHandler

.global Fault10
.type Fault10, @function
Fault10:
				// err_code pushed by CPU
	pushq $10	// int_no
	jmp GlobalHandler

.global Fault11
.type Fault11, @function
Fault11:
				// err_code pushed by CPU
	pushq $11	// int_no
	jmp GlobalHandler

.global Fault12
.type Fault12, @function
Fault12:
				// err_code pushed by CPU
	pushq $12	// int_no
	jmp GlobalHandler

.global Fault13
.type Fault13, @function
Fault13:
				// err_code pushed by CPU
	pushq $13	// int_no
	jmp GlobalHandler

.global Fault14
.type Fault14, @function
Fault14:
				// err_code pushed by CPU
	pushq $14	// int_no
	jmp GlobalHandler

.global Fault15
.type Fault15, @function
Fault15:
	pushq $0	// err_code
	pushq $15	// int_no
	jmp GlobalHandler

.global Fault16
.type Fault16, @function
Fault16:
	pushq $0	// err_code
	pushq $16	// int_no
	jmp GlobalHandler

.global Fault17
.type Fault17, @function
Fault17:
	pushq $0	// err_code
	pushq $17	// int_no
	jmp GlobalHandler

.global Fault18
.type Fault18, @function
Fault18:
	pushq $0	// err_code
	pushq $18	// int_no
	jmp GlobalHandler

.global Fault19
.type Fault19, @function
Fault19:
	pushq $0	// err_code
	pushq $19	// int_no
	jmp GlobalHandler

.global Fault20
.type Fault20, @function
Fault20:
	pushq $0	// err_code
	pushq $20	// int_no
	jmp GlobalHandler

.global Fault21
.type Fault21, @function
Fault21:
	pushq $0	// err_code
	pushq $21	// int_no
	jmp GlobalHandler

.global Fault22
.type Fault22, @function
Fault22:
	pushq $0	// err_code
	pushq $22	// int_no
	jmp GlobalHandler

.global Fault23
.type Fault23, @function
Fault23:
	pushq $0	// err_code
	pushq $23	// int_no
	jmp GlobalHandler

.global Fault24
.type Fault24, @function
Fault24:
	pushq $0	// err_code
	pushq $24	// int_no
	jmp GlobalHandler

.global Fault25
.type Fault25, @function
Fault25:
	pushq $0	// err_code
	pushq $25	// int_no
	jmp GlobalHandler

.global Fault26
.type Fault26, @function
Fault26:
	pushq $0	// err_code
	pushq $26	// int_no
	jmp GlobalHandler

.global Fault27
.type Fault27, @function
Fault27:
	pushq $0	// err_code
	pushq $27	// int_no
	jmp GlobalHandler

.global Fault28
.type Fault28, @function
Fault28:
	pushq $0	// err_code
	pushq $28	// int_no
	jmp GlobalHandler

.global Fault29
.type Fault29, @function
Fault29:
	pushq $0	// err_code
	pushq $29	// int_no
	jmp GlobalHandler

.global Fault30
.type Fault30, @function
Fault30:
	pushq $0	// err_code
	pushq $30	// int_no
	jmp GlobalHandler

.global Fault31
.type Fault31, @function
Fault31:
	pushq $0	// err_code
	pushq $31	// int_no
	jmp GlobalHandler




GlobalHandler:
	xchg %bx, %bx
	pushq %r15
	pushq %r14
	pushq %r13
	pushq %r12
	pushq %r11
	pushq %r10
	pushq %r9
	pushq %r8
	pushq %rdx
	pushq %rcx
	pushq %rbx
	pushq %rax
	pushq %rbp
	pushq %rsi
	pushq %rdi
	pushq %rsp


	// Push CR2 in case of page faults
	movq %cr2, %rbp
	pushq %rbp



	// Now call the interrupt handler.

	// pass the stack pointer as an argument, aka pointer to structure.
	movq %rsp, %rdi

	call ExceptionHandler_C
	hlt

	addq $8, %rsp	// remove cr2
	addq $8, %rsp	// Don't pop %rsp, may not be defined.

	popq %rdi
	popq %rsi
	popq %rbp
	popq %rax
	popq %rbx
	popq %rcx
	popq %rdx
	popq %r8
	popq %r9
	popq %r10
	popq %r11
	popq %r12
	popq %r13
	popq %r14
	popq %r15

	// Remove int_no and err_code
	addq $16, %rsp

	// Return to where we came from.
	iretq




