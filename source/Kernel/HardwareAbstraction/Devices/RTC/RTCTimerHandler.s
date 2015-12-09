// RTCTimerHandler.s
// Copyright (c) 2013 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


.global RTCHandler


RTCHandler:
	push %rax
	push %rbx
	push %rcx
	push %rdx

	call RTCTimerHandler
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
