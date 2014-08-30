// Functions.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "../syscall.h"

namespace Library {
namespace SystemCall
{
	// .quad	ExitProc			// 0
	// .quad	InstallIRQ			// 1
	// .quad	InstallIRQNoRegs		// 2

	// .quad	CreateThread			// 3
	// .quad	SpawnProcess			// 4
	// .quad	SendSignalToProcess		// 5
	// .quad	SendSignalToThread		// 6
	// .quad	SendMessage			// 7
	// .quad	ReceiveMessage		// 8
	// .quad	Sleep				// 9
	// .quad	Yield				// 10
	// .quad	Block				// 11
	// .quad	InstallSigHandler		// 12
	// .quad	GetPID				// 13
	// .quad	GetParentPID			// 14

	// .quad	OpenFile			// 15
	// .quad	OpenIPCSocket		// 16
	// .quad	OpenAnyFD			// 17
	// .quad	CloseAnyFD			// 18
	// .quad	ReadAnyFD			// 19
	// .quad	WriteAnyFD			// 20
	// .quad	MemoryMapAnonymous	// 21
	// .quad	MemoryMapFile		// 22

	void ExitProc()
	{
		Syscall0Param(0);
	}

	void InstallIRQHandler(uint64_t irq, uint64_t handleraddr)
	{
		Syscall2Param(1, irq, handleraddr);
	}

	void InstallIRQHandlerWithRegs(uint64_t irq, uint64_t handleraddr)
	{
		Syscall2Param(2, irq, handleraddr);
	}

	void CreateThread(void (*thr)())
	{
		Syscall1Param(3, (uint64_t) thr);
	}

	void SpawnProcess(const char* path, const char* name)
	{
		Syscall2Param(4, (uint64_t) path, (uint64_t) name);
	}

	void SignalProcess(pid_t pid, int signum)
	{
		Syscall2Param(5, pid, signum);
	}

	void SignalThread(pid_t tid, int signum)
	{
		Syscall2Param(6, tid, signum);
	}

	int SendMessage(key_t key, void* msg, size_t size, uint64_t flags)
	{
		return (int) Syscall4Param(7, key, (uint64_t) msg, size, flags);
	}

	ssize_t ReceiveMessage(key_t key, void* msg, size_t size, uint64_t type, uint64_t flags)
	{
		return (int) Syscall5Param(8, key, (uint64_t) msg, size, type, flags);
	}

	void Sleep(uint64_t ms)
	{
		Syscall1Param(9, ms);
	}

	void Yield()
	{
		Syscall0Param(10);
	}

	void Block()
	{
		Syscall0Param(11);
	}

	sighandler_t InstallSignalHandler(uint64_t signum, sighandler_t handler)
	{
		return (sighandler_t) Syscall2Param(12, signum, (uint64_t) handler);
	}

	uint64_t GetPID()
	{
		return Syscall0Param(13);
	}

	uint64_t GetParentPID()
	{
		return Syscall0Param(14);
	}

	// 15, 16

	uint64_t Open(const char* path, uint64_t flags)
	{
		return Syscall2Param(17, (uint64_t) path, flags);
	}

	void Close(uint64_t fd)
	{
		Syscall1Param(18, fd);
	}

	uint64_t Read(uint64_t sd, void* buffer, uint64_t length)
	{
		return Syscall3Param(19, sd, (uint64_t) buffer, length);
	}

	uint64_t Write(uint64_t sd, const void* buffer, uint64_t length)
	{
		return Syscall3Param(20, sd, (uint64_t) buffer, length);
	}

	uint64_t MMap_Anonymous(uint64_t addr, uint64_t size, uint64_t prot, uint64_t flags)
	{
		return Syscall4Param(21, addr, size, prot, flags);
	}
}
}
