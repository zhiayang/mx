// SecondStage.s
// Copyright (c) 2011-2013, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.



// Sets up stack, pushes MBT info to KernelMain
// Calls KernelMain (kernel.c)

// Referenced From:
// src/loader/boot.s

// Referenced Files:
// src/kernel/kernel.c


.section .text

.global KernelBootStrap
.global GetReturnAddress

.type KernelBootStrap, @function
.type GetReturnAddress, @function

.code64



KernelBootStrap:

	// Push stack pointer
	movq $0x60000, %rsp




	mov $0, %rbp


	mov $StartConstructors, %rbx
	jmp 2f

1:
	call *(%rbx)
	add $8, %rbx
2:
	cmp $EndConstructors, %rbx
	jb 1b


	// in boot.s we memcpy()'d the boot struct to 0x40000 to avoid trashing it
	mov $0x40000, %ebx
	mov 0x0500, %eax		// magic number

	movl $0x40200, 48(%ebx)



	// Push MBT struct pointer
	mov %rbx, %rsi

	// Push Magic value (0x2BADB002)
	mov %rax, %rdi



	// Setup SSE

	mov %cr0, %rax
	and $0xFFFB, %ax		// clear coprocessor emulation CR0.EM
	or $0x02, %ax			// set coprocessor monitoring  CR0.MP
	mov %rax, %cr0
	mov %cr4, %rax
	or $(3 << 9), %ax		// set CR4.OSFXSR and CR4.OSXMMEXCPT at the same time
	mov %rax, %cr4



	// setup MSRs for syscall/sysret

	// modify STAR
	mov $0xC0000081, %ecx
	rdmsr
	// msr is edx:eax

	// simultaneously setup sysret CS and syscall CS
	mov $0x001B0008, %edx
	xor %eax, %eax
	wrmsr


	// now we modify LSTAR to hold the address of HandleSyscall
	xor %edx, %edx

	// TODO: Write handler for SYSCALL instruction instead of interrupt
	// keep both options available.
	// fill in address when ready
	// mov $HandleSyscallInstruction, %eax
	mov $0x00, %eax
	mov $0xC0000082, %ecx
	wrmsr


	// set SFMASK to 0.
	mov $0xC0000084, %ecx
	xor %eax, %eax
	wrmsr








	// Call our kernel.
	call KernelInit


	mov $EndDestructors, %rbx
	jmp 4f

3:
	sub $8, %rbx
	call *(%rbx)
4:
	cmp $StartDestructors, %rbx
	ja 3b



	cli
halt:
	hlt
	jmp halt












/*
.section .bss
.align 16
TheStack:
	.lcomm Stack, 0x8000				// Reserve stack
*/
