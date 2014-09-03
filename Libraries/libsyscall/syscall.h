// syscall.h
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

// userspace/Syscall.hpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <stdint.h>
#include <signal.h>
#include <pthread.h>
#pragma once

#ifdef __cplusplus
namespace Library
{
	namespace SystemCall
	{


		extern "C" uint64_t Syscall0Param(uint64_t scvec);
		extern "C" uint64_t Syscall1Param(uint64_t scvec, uint64_t p1);
		extern "C" uint64_t Syscall2Param(uint64_t scvec, uint64_t p1, uint64_t p2);
		extern "C" uint64_t Syscall3Param(uint64_t scvec, uint64_t p1, uint64_t p2, uint64_t p3);
		extern "C" uint64_t Syscall4Param(uint64_t scvec, uint64_t p1, uint64_t p2, uint64_t p3, uint64_t p4);
		extern "C" uint64_t Syscall5Param(uint64_t scvec, uint64_t p1, uint64_t p2, uint64_t p3, uint64_t p4, uint64_t p5);


		void ExitProc();
		void InstallIRQHandler(uint64_t irq, uint64_t handleraddr);
		void InstallIRQHandlerWithRegs(uint64_t irq, uint64_t handleraddr);
		pthread_t CreateThread(pthread_attr_t* attribs, void (*thr)());
		void SpawnProcess(const char* path, const char* name);
		void SignalProcess(pid_t pid, int signum);
		void SignalThread(pid_t tid, int signum);
		int SendMessage(const char* path, void* msg, size_t size, uint64_t flags);
		ssize_t ReceiveMessage(const char* path, void* msg, size_t size, uint64_t type, uint64_t flags);
		void Sleep(uint64_t ms);
		void Yield();
		void Block();
		sighandler_t InstallSignalHandler(uint64_t signum, sighandler_t handler);
		uint64_t GetPID();
		uint64_t GetParentPID();
		uint64_t Open(const char* path, uint64_t flags);
		void Close(uint64_t fd);
		uint64_t Read(uint64_t sd, void* buffer, uint64_t length);
		uint64_t Write(uint64_t sd, const void* buffer, uint64_t length);
		uint64_t MMap_Anonymous(uint64_t addr, uint64_t size, uint64_t prot, uint64_t flags);
	}
}
#else
#error unsupported
#endif




