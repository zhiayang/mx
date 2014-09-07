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


		Clobbering:
		*NEVER* *EVER* *EVER* do int $0xF8 directly.
		This *WILL* clobber registers that you do not expect.

		The Syscall[0-5]Param() functions are declared with C linkage (ie. extern "C"), so ALWAYS call that if you're working
		directly in ASM, eg.

		mov <param1>, %rdi
		mov <param2>, %rsi
		mov <syscallvec>, %rdx
		call Syscall2Param

		mov %rax, <somewhere>
		<etc>

		the Syscall* functions take the syscall parameter *LAST* in order to reduce work and register shifting.
		If you're going to program in ASM, you should be smart enough to figure out the calling convention.
	*/

	push %rbp
	mov %rsp, %rbp

	push %r10
	push %rdi
	push %rsi
	push %rdx
	push %rcx
	push %r8
	push %r9

	// push a constant, so we know where to stop on stack backtrace.
	pushq $0xFFFFFFFFFFFFFFFF


	// since we have syscall numbers in the range of 4000+, 8000+, we can't use a simple jump table anymore.
	// rather, we subtract each 'page' from the incoming syscall.
	// for instance:
	// if we get 8014, we first check if it's greater than 8000. it is, so we subtract 8000 and call '14'.
	// if we get 4044, it's not greater than 8000, but greater than 4000, so we subtract 4000.
	// simple stuff.

	cmp $8000, %r10
	jge Page2

	cmp $4000, %r10
	jge Page1



	jmp Page0



Page1:
	sub $4000, %r10
	salq $3, %r10
	movq SyscallTable1(%r10), %r10
	cmp $EndSyscallTable1, %r10

	jge Fail
	jmp DoCall

Page2:
	sub $8000, %r10
	salq $3, %r10
	movq SyscallTable2(%r10), %r10
	cmp $EndSyscallTable2, %r10

	jge Fail
	jmp DoCall

Page0:
	// multiply %r10 by 8
	// shift left by 3	(2^3 = 8)
	salq $3, %r10
	movq SyscallTable0(%r10), %r10
	cmp $EndSyscallTable0, %r10

	jge Fail


DoCall:
	jmp *%r10


CleanUp:
	// remove the constant we pushed
	addq $8, %rsp

	pop %r9
	pop %r8
	pop %rcx
	pop %rdx
	pop %rsi
	pop %rdi
	pop %r10

	// any errno set by a syscall is stored in 0x2610 and preserved across context switches.
	// since all accesses to this are done via asm, we can just fetch the value out in userspace.
	movq 0x2610, %r13

	pop %rbp
	iretq


Fail:
	jmp CleanUp



ExitProc:
	call Syscall_ExitProc
	jmp CleanUp

InstallIRQ:
	call Syscall_InstallIRQHandler
	jmp CleanUp

InstallIRQNoRegs:
	call Syscall_InstallIRQHandlerNoRegs
	jmp CleanUp



// page 1
CreateThread:
	call Syscall_CreateThread
	jmp CleanUp

SpawnProcess:
	call Syscall_SpawnProcess
	jmp CleanUp

SendSignalToProcess:
	call IPC_SignalProcess
	jmp CleanUp

SendSignalToThread:
	call IPC_SignalThread
	jmp CleanUp

SendMessage:
	call IPC_SendMessage
	jmp CleanUp

ReceiveMessage:
	call IPC_ReceiveMessage
	jmp CleanUp

Sleep:
	call Syscall_Sleep
	jmp CleanUp

Yield:
	call Syscall_Yield
	jmp CleanUp

Block:
	call Syscall_Block
	jmp CleanUp

InstallSigHandler:
	call Syscall_InstallSigHandler
	jmp CleanUp

GetPID:
	call Syscall_GetPID
	jmp CleanUp

GetParentPID:
	call Syscall_GetParentPID
	jmp CleanUp

__ExitThread:
	call ExitThread
	jmp CleanUp

JoinThread:
	call Syscall_JoinThread
	jmp CleanUp

GetThisTID:
	call Syscall_GetTID
	jmp CleanUp

LockMutex:
	// call Syscall_LockMutex
	jmp CleanUp

UnlockMutex:
	// call Syscall_UnlockMutex
	jmp CleanUp

TryLockMutex:
	// call Syscall_TryLockMutex
	jmp CleanUp










// page 2
OpenFile:
	// call Syscall_OpenFile
	jmp CleanUp

OpenIPCSocket:
	// call IPC_OpenSocket
	jmp CleanUp

OpenAnyFD:
	call Syscall_OpenAny
	jmp CleanUp

CloseAnyFD:
	call Syscall_CloseAny
	jmp CleanUp

ReadAnyFD:
	call Syscall_ReadAny
	jmp CleanUp

WriteAnyFD:
	call Syscall_WriteAny
	jmp CleanUp

MemoryMapAnonymous:
	call Syscall_MMapAnon
	jmp CleanUp

MemoryMapFile:
	// call Syscall_MMapFile
	jmp CleanUp


.section .data

.align 8
FailString:
	.asciz "\n\nInvalid Syscall number, you fool!\n\n"


.align 8
SyscallTable0:
	// misc things, page 0+
	.quad	ExitProc			// 0000
	.quad	InstallIRQ			// 0001
	.quad	InstallIRQNoRegs		// 0002

EndSyscallTable0:


.align 8
SyscallTable1:

	// process related things, page 4000+
	.quad	CreateThread			// 4000
	.quad	SpawnProcess			// 4001
	.quad	SendSignalToProcess		// 4002
	.quad	SendSignalToThread		// 4003
	.quad	SendMessage			// 4004
	.quad	ReceiveMessage		// 4005
	.quad	Sleep				// 4006
	.quad	Yield				// 4007
	.quad	Block				// 4008
	.quad	InstallSigHandler		// 4009
	.quad	GetPID				// 4010
	.quad	GetParentPID			// 4011
	.quad	__ExitThread			// 4012
	.quad	JoinThread			// 4013
	.quad	GetThisTID			// 4014
	.quad	LockMutex			// 4015
	.quad	UnlockMutex			// 4016
	.quad	TryLockMutex			// 4017
EndSyscallTable1:


.align 8
SyscallTable2:

	// file io things, page 8000+
	.quad	OpenFile			// 8000
	.quad	OpenIPCSocket		// 8001
	.quad	OpenAnyFD			// 8002
	.quad	CloseAnyFD			// 8003
	.quad	ReadAnyFD			// 8004
	.quad	WriteAnyFD			// 8005
	.quad	MemoryMapAnonymous	// 8006
	.quad	MemoryMapFile		// 8007
EndSyscallTable2:






