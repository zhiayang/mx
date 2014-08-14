// SerialPortIRQ.s
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

.global SerialPort_HandleIRQ4

SerialPort_HandleIRQ4:
	push %rax
	call IRQ_BufferEmpty

	xor %rax, %rax

	mov $0x20, %al
	outb %al, $0x20

	mov $0x20, %al
	outb %al, $0xA0

	pop %rax
	iretq

