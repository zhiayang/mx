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
	// .quad	SimpleToProcesss		// 5
	// .quad	SimpleToThread		// 6
	// .quad	GetSimpleMessage		// 7
	// .quad	Sleep				// 8
	// .quad	Yield				// 9
	// .quad	Block				// 10
	// .quad	InstallSigHandler		// 11
	// .quad	GetPID				// 12
	// .quad	GetParentPID			// 13

	// .quad	OpenFile			// 14
	// .quad	OpenIPCSocket		// 15
	// .quad	OpenAnyFD			// 16
	// .quad	CloseAnyFD			// 17
	// .quad	ReadAnyFD			// 18
	// .quad	WriteAnyFD			// 19
	// .quad	MemoryMapAnonymous	// 20
	// .quad	MemoryMapFile		// 21

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

	void SendSimpleMessageToProcess(uint64_t TargetPID, MessageTypes Type, uint64_t D1, uint64_t D2, uint64_t D3, void (*Callback)())
	{
		(void) Callback;
		Syscall5Param(5, TargetPID, (uint64_t) Type, D1, D2, D3);
	}

	void SendSimpleMessage(uint64_t TargetThreadID, MessageTypes Type, uint64_t D1, uint64_t D2, uint64_t D3, void (*Callback)())
	{
		(void) Callback;
		Syscall5Param(6, TargetThreadID, (uint64_t) Type, D1, D2, D3);
	}

	SimpleMessage* GetSimpleMessage()
	{
		return (SimpleMessage*) Syscall0Param(7);
	}


	void Sleep(uint64_t ms)
	{
		Syscall1Param(8, ms);
	}

	void Yield()
	{
		Syscall0Param(9);
	}

	void Block()
	{
		Syscall0Param(10);
	}

	sighandler_t InstallSignalHandler(uint64_t signum, sighandler_t handler)
	{
		return (sighandler_t) Syscall2Param(11, signum, (uint64_t) handler);
	}

	uint64_t GetPID()
	{
		return Syscall0Param(12);
	}

	uint64_t GetParentPID()
	{
		return Syscall0Param(13);
	}

	// 14, 15

	uint64_t Open(const char* path, uint64_t flags)
	{
		return Syscall2Param(16, (uint64_t) path, flags);
	}

	void Close(uint64_t fd)
	{
		Syscall1Param(17, fd);
	}

	uint64_t Read(uint64_t sd, uint8_t* buffer, uint64_t length)
	{
		return Syscall3Param(18, sd, (uint64_t) buffer, length);
	}

	uint64_t Write(uint64_t sd, uint8_t* buffer, uint64_t length)
	{
		return Syscall3Param(19, sd, (uint64_t) buffer, length);
	}

	uint64_t MMap_Anonymous(uint64_t addr, uint64_t size, uint64_t prot, uint64_t flags)
	{
		return Syscall4Param(20, addr, size, prot, flags);
	}
}
}
