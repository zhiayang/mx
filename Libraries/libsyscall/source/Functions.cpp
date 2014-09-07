// Functions.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "../syscall.h"
#include <errno.h>

namespace Library {
namespace SystemCall
{
	/*
		// misc things, page 0+
		.quad	ExitProc			// 0000
		.quad	InstallIRQ			// 0001
		.quad	InstallIRQNoRegs		// 0002

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
		.quad	ExitThread			// 4012
		.quad	JoinThread			// 4013
		.quad	GetTID				// 4014

		// file io things, page 8000+
		.quad	OpenFile			// 8000
		.quad	OpenIPCSocket		// 8001
		.quad	OpenAnyFD			// 8002
		.quad	CloseAnyFD			// 8003
		.quad	ReadAnyFD			// 8004
		.quad	WriteAnyFD			// 8005
		.quad	MemoryMapAnonymous	// 8006
		.quad	MemoryMapFile		// 8007
	*/


	void ExitProc()
	{
		Syscall0Param(0);
	}

	void InstallIRQHandler(uint64_t irq, uint64_t handleraddr)
	{
		Syscall2Param(irq, handleraddr, 1);
	}

	void InstallIRQHandlerWithRegs(uint64_t irq, uint64_t handleraddr)
	{
		Syscall2Param(irq, handleraddr, 2);
	}



	pthread_t CreateThread(pthread_attr_t* attr, void (*thr)())
	{
		return Syscall2Param((uintptr_t) thr, (uintptr_t) attr, 4000);
	}

	void SpawnProcess(const char* path, const char* name)
	{
		Syscall2Param((uint64_t) path, (uint64_t) name, 4001);
	}

	void SignalProcess(pid_t pid, int signum)
	{
		Syscall2Param(pid, signum, 4002);
	}

	void SignalThread(pid_t tid, int signum)
	{
		Syscall2Param(tid, signum, 4003);
	}

	int SendMessage(const char* path, void* msg, size_t size, uint64_t flags)
	{
		return (int) Syscall4Param((uintptr_t) path, (uint64_t) msg, size, flags, 4004);
	}

	ssize_t ReceiveMessage(const char* path, void* msg, size_t size, uint64_t type, uint64_t flags)
	{
		return (int) Syscall5Param((uintptr_t) path, (uint64_t) msg, size, type, flags, 4005);
	}

	void Sleep(uint64_t ms)
	{
		Syscall1Param(ms, 4006);
	}

	void Yield()
	{
		Syscall0Param(4007);
	}

	void Block()
	{
		Syscall0Param(4008);
	}

	sighandler_t InstallSignalHandler(uint64_t signum, sighandler_t handler)
	{
		return (sighandler_t) Syscall2Param(signum, (uint64_t) handler, 4009);
	}

	uint64_t GetPID()
	{
		return Syscall0Param(4010);
	}

	uint64_t GetParentPID()
	{
		return Syscall0Param(4011);
	}

	void ExitThread()
	{
		Syscall0Param(4012);
	}

	void* JoinThread(uint64_t tid)
	{
		return (void*) Syscall1Param(tid, 4013);
	}

	pthread_t GetTID()
	{
		return (pthread_t) Syscall0Param(4014);
	}

	// 8000, 8001

	uint64_t Open(const char* path, uint64_t flags)
	{
		return Syscall2Param((uint64_t) path, flags, 8002);
	}

	void Close(uint64_t fd)
	{
		Syscall1Param(fd, 8003);
	}

	uint64_t Read(uint64_t sd, void* buffer, uint64_t length)
	{
		return Syscall3Param(sd, (uint64_t) buffer, length, 8004);
	}

	uint64_t Write(uint64_t sd, const void* buffer, uint64_t length)
	{
		return Syscall3Param(sd, (uint64_t) buffer, length, 8005);
	}

	uint64_t MMap_Anonymous(uint64_t addr, uint64_t size, uint64_t prot, uint64_t flags)
	{
		return Syscall4Param(addr, size, prot, flags, 8006);
	}
}
}






extern "C" void* __fetch_errno()
{
	return &errno;
}














