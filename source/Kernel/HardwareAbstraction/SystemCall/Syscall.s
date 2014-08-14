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

	push %r10
	push %rdi
	push %rsi
	push %rdx
	push %rcx
	push %r8
	push %r9

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
	pop %r9
	pop %r8
	pop %rcx
	pop %rdx
	pop %rsi
	pop %rdi
	pop %r10
	pop %rbp
	iretq


Fail:
	push %rdi

	mov $FailString, %rdi
	call Syscall_PrintString
	pop %rdi

	jmp CleanUp

ExitProc:
	call Syscall_ExitProc
	jmp CleanUp

PrintChar:
	call Syscall_PrintChar
	jmp CleanUp

PrintString:
	call Syscall_PrintString
	jmp CleanUp

AllocateChunk:
	// deprecated
	jmp CleanUp

FreeChunk:
	// deprecated
	jmp CleanUp

QueryChunkSize:
	// deprecated
	jmp CleanUp

Sleep:
	call Syscall_Sleep
	jmp CleanUp

GetCR3:
	call Syscall_GetCR3
	jmp CleanUp

InvlPg:
	call Syscall_InvlPg
	jmp CleanUp

InstallIRQ:
	call Syscall_InstallIRQHandler
	jmp CleanUp

InstallIRQNoRegs:
	call Syscall_InstallIRQHandlerNoRegs
	jmp CleanUp

ReadKeyboardKey:
	call Syscall_ReadKeyboardKey
	jmp CleanUp

CreateThread:
	call Syscall_CreateThread
	jmp CleanUp

GetFramebufferAddress:
	call Syscall_GetFramebufferAddress
	jmp CleanUp

SetConsoleBackColour:
	call Syscall_SetConsoleBackColour
	jmp CleanUp

GetFramebufferResX:
	call Syscall_GetFramebufferResX
	jmp CleanUp

GetFramebufferResY:
	call Syscall_GetFramebufferResY
	jmp CleanUp

MapVirtualAddress:
	call Syscall_MapVirtualAddress
	jmp CleanUp

UnmapVirtualAddress:
	call Syscall_UnmapVirtualAddress
	jmp CleanUp

AllocatePage:
	call Syscall_AllocatePage
	jmp CleanUp

FreePage:
	call Syscall_FreePage
	jmp CleanUp

GetConsoleBackColour:
	call Syscall_GetConsoleBackColour
	jmp CleanUp

GetConsoleTextColour:
	call Syscall_GetConsoleTextColour
	jmp CleanUp

SetConsoleTextColour:
	call Syscall_SetConsoleTextColour
	jmp CleanUp

OpenFile:
	call Syscall_OpenFile
	jmp CleanUp

CloseFile:
	call Syscall_CloseFile
	jmp CleanUp

ReadFile:
	call Syscall_ReadFile
	jmp CleanUp

WriteFile:
	call Syscall_WriteFile
	jmp CleanUp

OpenFolder:
	call Syscall_OpenFolder
	jmp CleanUp

CloseFolder:
	call Syscall_CloseFolder
	jmp CleanUp

ListObjectsInFolder:
	call Syscall_ListObjectsInFolder
	jmp CleanUp

GetFileSize:
	call Syscall_GetFileSize
	jmp CleanUp

CheckFileExistence:
	call Syscall_CheckFileExistence
	jmp CleanUp

CheckFolderExistence:
	call Syscall_CheckFolderExistence
	jmp CleanUp

MemoryToProcess:
	call IPC_MemoryToProcess
	jmp CleanUp

MemoryToThread:
	call IPC_MemoryToThread
	jmp CleanUp

SimpleToProcesss:
	call IPC_SimpleToProcess
	jmp CleanUp

SimpleToThread:
	call IPC_SimpleToThread
	jmp CleanUp

GetMemoryMessage:
	call IPC_GetMemoryMessage
	jmp CleanUp

GetSimpleMessage:
	call IPC_GetSimpleMessage
	jmp CleanUp

Yield:
	call Syscall_Yield
	jmp CleanUp

Block:
	call Syscall_Block
	jmp CleanUp

ClearScreen:
	call Syscall_ClearScreen
	jmp CleanUp

SpawnProcess:
	call Syscall_SpawnProcess
	jmp CleanUp

GetCursorX:
	call Syscall_GetCursorX
	jmp CleanUp

GetCursorY:
	call Syscall_GetCursorY
	jmp CleanUp

SetCursorPos:
	call Syscall_SetCursorPos
	jmp CleanUp

GetCharWidth:
	call Syscall_GetCharWidth
	jmp CleanUp

GetCharHeight:
	call Syscall_GetCharHeight
	jmp CleanUp

PutPixelAtXY:
	call Syscall_PutPixelAtXY
	jmp CleanUp

PutCharNoMove:
	call Syscall_PutCharNoMove
	jmp CleanUp

OpenIPCSocket:
	call IPC_OpenSocket
	jmp CleanUp

CloseIPCSocket:
	call IPC_CloseSocket
	jmp CleanUp

ReadIPCSocket:
	call IPC_ReadSocket
	jmp CleanUp

WriteIPCSocket:
	call IPC_WriteSocket
	jmp CleanUp

MemoryMapAnonymous:
	call Syscall_MMapAnon
	jmp CleanUp

MemoryMapFile:
	call Syscall_MMapFile
	jmp CleanUp

ReadAnyFD:
	call Syscall_ReadAny
	jmp CleanUp

WriteAnyFD:
	call Syscall_WriteAny
	jmp CleanUp

OpenAnyFD:
	call Syscall_OpenAny
	jmp CleanUp

CloseAnyFD:
	call Syscall_CloseAny
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

.section .data

.align 8
FailString:
	.asciz "\n\nInvalid Syscall number, you fool!\n\n"


.align 8
SyscallTable:
	.quad	ExitProc			// 0
	.quad	PrintChar			// 1
	.quad	PrintString			// 2
	.quad	AllocateChunk			// 3
	.quad	FreeChunk			// 4
	.quad	QueryChunkSize		// 5
	.quad	Sleep				// 6
	.quad	GetCR3			// 7
	.quad	InvlPg				// 8
	.quad	InstallIRQ			// 9
	.quad	InstallIRQNoRegs		// 10
	.quad	ReadKeyboardKey		// 11
	.quad	CreateThread			// 12
	.quad	GetFramebufferAddress	// 13
	.quad	SetConsoleBackColour	// 14
	.quad	GetFramebufferResX		// 15
	.quad	GetFramebufferResY		// 16
	.quad	MapVirtualAddress		// 17
	.quad	UnmapVirtualAddress		// 18
	.quad	AllocatePage			// 19
	.quad	FreePage			// 20
	.quad	GetConsoleBackColour	// 21
	.quad	GetConsoleTextColour	// 22
	.quad	SetConsoleTextColour	// 23
	.quad	OpenFile			// 24	note: DEPRECATED. Prefer OpenAnyFD.
	.quad	CloseFile			// 25	note: DEPRECATED. Prefer CloseAnyFD.
	.quad	ReadFile			// 26	note: DEPRECATED. Prefer ReadAnyFD.
	.quad	WriteFile			// 27	note: DEPRECATED. Prefer WriteAnyFD.
	.quad	OpenFolder			// 28
	.quad	CloseFolder			// 29
	.quad	ListObjectsInFolder		// 30
	.quad	GetFileSize			// 31
	.quad	CheckFileExistence		// 32
	.quad	CheckFolderExistence		// 33
	.quad	MemoryToProcess		// 34
	.quad	MemoryToThread		// 35
	.quad	SimpleToProcesss		// 36
	.quad	SimpleToThread		// 37
	.quad	GetMemoryMessage		// 38
	.quad	GetSimpleMessage		// 39
	.quad	Yield				// 40
	.quad	Block				// 41
	.quad	ClearScreen			// 42
	.quad	SpawnProcess			// 43
	.quad	GetCursorX			// 44
	.quad	GetCursorY			// 45
	.quad	SetCursorPos			// 46
	.quad	GetCharWidth			// 47
	.quad	GetCharHeight		// 48
	.quad	PutPixelAtXY			// 49
	.quad 	PutCharNoMove		// 50
	.quad	OpenIPCSocket		// 51
	.quad	CloseIPCSocket		// 52
	.quad	ReadIPCSocket		// 53
	.quad	WriteIPCSocket		// 54
	.quad	MemoryMapAnonymous	// 55
	.quad	MemoryMapFile		// 56
	.quad	ReadAnyFD			// 57
	.quad	WriteAnyFD			// 58
	.quad	OpenAnyFD			// 59
	.quad	CloseAnyFD			// 60
	.quad	InstallSigHandler		// 61
	.quad	GetPID				// 62
	.quad	GetParentPID			// 63

EndSyscallTable:







