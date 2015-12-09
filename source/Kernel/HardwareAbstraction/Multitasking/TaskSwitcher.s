// TaskSwitcher.s
// Copyright (c) 2013 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

.global ProcessTimerInterrupt
.type ProcessTimerInterrupt, @function

.global TaskSwitcherCoOp
.type TaskSwitcherCoOp, @function

.section .text
.code64


/*
	yield:
		push regs
		change segments
		call switchprocess
		change cr3
		change segments
		iret

	timer:
		push regs
		call timer
		if(rax == 0)
		{
			pop regs
			iretq
		}

		mov rax, rsp
		pop regs
		change segments
		iretq


*/

TaskSwitcherCoOp:
	movq $1, IsYieldMode
	jmp StageOne

ProcessTimerInterrupt:
	movq $0, IsYieldMode
	jmp StageOne

StageOne:
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

	// this is where we diverge.
	cmpq $0, IsYieldMode
	je CallTimer


	// Load the kernel data segment
	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es




	movq %rsp, %rdi

	call SwitchProcess
	movq %rax, %rsp





StageTwo:
	cmpq $0xFADE, 0x2608
	je DoRing3



ChangeCR3:
	cmpq $0x0, 0x2600
	je PopReg


	// if not zero, do a CR3 switch.
	movq 0x2600, %rax
	mov %rax, %cr3
	jmp PopReg


PopReg:
	call VerifySchedule
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


DoRing3:
	// Load the user data segment
	mov $0x23, %rbp
	mov %rbp, %ds
	mov %rbp, %es

	jmp ChangeCR3


CallTimer:
	mov %rsp, %rdi
	call ProcessTimerInterrupt_C

	// check if we need to switch
	push %rax

	mov $0x20, %al
	outb %al, $0x20

	pop %rax


	cmpq $0, %rax

	// if we don't, pop and iret.
	je PopReg


	mov %rax, %rsp
	// else, we need to.
	jmp StageTwo






.section .data
IsYieldMode:
	.quad		0



