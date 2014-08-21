// helper.s
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

.global Syscall0Param
.global Syscall1Param
.global Syscall2Param
.global Syscall3Param
.global Syscall4Param
.global Syscall5Param

.type Syscall0Param, @function
.type Syscall1Param, @function
.type Syscall2Param, @function
.type Syscall3Param, @function
.type Syscall4Param, @function
.type Syscall5Param, @function

Syscall0Param:
	mov %rdi, %r10
	int $0xF8
	ret

Syscall1Param:
	mov %rdi, %r10
	mov %rsi, %rdi
	int $0xF8
	ret

Syscall2Param:
	mov %rdi, %r10
	mov %rsi, %rdi
	mov %rdx, %rsi
	int $0xF8
	ret

Syscall3Param:
	mov %rdi, %r10
	mov %rsi, %rdi
	mov %rdx, %rsi
	mov %rcx, %rdx
	int $0xF8
	ret

Syscall4Param:
	mov %rdi, %r10
	mov %rsi, %rdi
	mov %rdx, %rsi
	mov %rcx, %rdx
	mov %r8, %rcx
	int $0xF8
	ret

Syscall5Param:
	mov %rdi, %r10
	mov %rsi, %rdi
	mov %rdx, %rsi
	mov %rcx, %rdx
	mov %r8, %rcx
	mov %r9, %r8
	int $0xF8
	ret



