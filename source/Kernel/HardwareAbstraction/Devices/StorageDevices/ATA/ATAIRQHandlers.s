// ATAIRQHandlers.s
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

.global ATA_HandleIRQ14
.global ATA_HandleIRQ15

ATA_HandleIRQ14:
	push %rax
	push %rbx
	push %rcx
	push %rdx

	call IRQHandler14

	xor %rax, %rax
	mov $0x20, %al
	outb %al, $0x20
	mov $0x20, %al
	outb %al, $0xA0

	pop %rdx
	pop %rcx
	pop %rbx
	pop %rax
	iretq


ATA_HandleIRQ15:
	push %rax
	push %rbx
	push %rcx
	push %rdx

	call IRQHandler15

	xor %rax, %rax
	mov $0x20, %al
	outb %al, $0x20
	mov $0x20, %al
	outb %al, $0xA0

	pop %rdx
	pop %rcx
	pop %rbx
	pop %rax
	iretq
