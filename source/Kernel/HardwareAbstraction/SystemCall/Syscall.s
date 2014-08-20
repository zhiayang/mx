// Syscall.s
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.




.global HandleSyscall
.type HandleSyscall, @function

.section .text
HandleSyscall:
	/*
		Documentation:
		%rdi
		%rsi
		%rdx
		%rcx
		%r8
		%r9
		--> stack.

		Syscall number in %r10
		Parameters in order of ABI.
	*/

	push %rbp
	mov %rsp, %rbp

	// push a constant, so we know where to stop on stack backtrace.
	pushq $0xFFFFFFFFFFFFFFFF


	// multiply %r10 by 8
	// shift left by 3	(2^3 = 8)
	salq $3, %r10
	movq SyscallTable(%r10), %r10
	cmp $EndSyscallTable, %r10

	jge Fail


	jmp *%r10


CleanUp:
	addq $8, %rsp
	pop %rbp
	iretq


Fail:
	// push %rdi

	// mov $FailString, %rdi
	// call Syscall_PrintString
	// pop %rdi

	jmp CleanUp



ExitProc:
	// call Syscall_ExitProc
	jmp CleanUp

PrintChar:
	// call Syscall_PrintChar
	jmp CleanUp

PrintString:
	// call Syscall_PrintString
	jmp CleanUp

InstallIRQ:
	// call Syscall_InstallIRQHandler
	jmp CleanUp

InstallIRQNoRegs:
	// call Syscall_InstallIRQHandlerNoRegs
	jmp CleanUp


CreateThread:
	// call Syscall_CreateThread
	jmp CleanUp

SpawnProcess:
	// call Syscall_SpawnProcess
	jmp CleanUp

SimpleToProcesss:
	// call IPC_SimpleToProcess
	jmp CleanUp

SimpleToThread:
	// call IPC_SimpleToThread
	jmp CleanUp

GetSimpleMessage:
	// call IPC_GetSimpleMessage
	jmp CleanUp

Sleep:
	// call Syscall_Sleep
	jmp CleanUp

Yield:
	// call Syscall_Yield
	jmp CleanUp

Block:
	// call Syscall_Block
	jmp CleanUp

InstallSigHandler:
	// call Syscall_InstallSigHandler
	jmp CleanUp

GetPID:
	// call Syscall_GetPID
	jmp CleanUp

GetParentPID:
	// call Syscall_GetParentPID
	jmp CleanUp







OpenFile:
	// call Syscall_OpenFile
	jmp CleanUp

OpenIPCSocket:
	// call IPC_OpenSocket
	jmp CleanUp

OpenAnyFD:
	// call Syscall_OpenAny
	jmp CleanUp

CloseAnyFD:
	// call Syscall_CloseAny
	jmp CleanUp

ReadAnyFD:
	// call Syscall_ReadAny
	jmp CleanUp

WriteAnyFD:
	// call Syscall_WriteAny
	jmp CleanUp

MemoryMapAnonymous:
	// call Syscall_MMapAnon
	jmp CleanUp

MemoryMapFile:
	// call Syscall_MMapFile
	jmp CleanUp


.section .data

.align 8
FailString:
	.asciz "\n\nInvalid Syscall number, you fool!\n\n"


.align 8
SyscallTable:
	.quad	ExitProc			// 0
	.quad	PrintChar			// 1
	.quad	PrintString			// 2
	.quad	InstallIRQ			// 3
	.quad	InstallIRQNoRegs		// 4

	.quad	CreateThread			// 5
	.quad	SpawnProcess			// 6
	.quad	SimpleToProcesss		// 7
	.quad	SimpleToThread		// 8
	.quad	GetSimpleMessage		// 9
	.quad	Sleep				// 10
	.quad	Yield				// 11
	.quad	Block				// 12
	.quad	InstallSigHandler		// 13
	.quad	GetPID				// 14
	.quad	GetParentPID			// 15

	.quad	OpenFile			// 16	note: DEPRECATED. Prefer OpenAnyFD.
	.quad	OpenIPCSocket		// 17
	.quad	OpenAnyFD			// 18
	.quad	CloseAnyFD			// 19
	.quad	ReadAnyFD			// 20
	.quad	WriteAnyFD			// 21
	.quad	MemoryMapAnonymous	// 22
	.quad	MemoryMapFile		// 23

EndSyscallTable:






