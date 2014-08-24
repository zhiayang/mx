// syscall.h
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

// userspace/Syscall.hpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <stdint.h>
#include <signal.h>
#pragma once

#ifdef __cplusplus
namespace Library
{
	namespace SystemCall
	{
		enum class MessageClassTypes
		{
			T_SimpleMessage,
			T_MemoryMessage
		};

		enum MessageTypes
		{
			Acknowledge,
			AcknowledgeWithData,
			RequestServiceInitialise,
			RequestServiceDispatch,
			ServiceData,
			MutexWakeup,
			PosixSignal
		};

		class IMessage
		{
			public:
				MessageClassTypes Type;
		};

		class SimpleMessage : public IMessage
		{
			public:
				SimpleMessage(uint64_t sp, uint64_t st, MessageTypes t, uint64_t d1, uint64_t d2, uint64_t d3, void (*cb)())
				{
					this->SenderPID = sp;
					this->SenderTID = st;
					this->MessageType = t;
					this->Callback = cb;

					this->Data1 = d1;
					this->Data2 = d2;
					this->Data3 = d3;

					this->Type = MessageClassTypes::T_SimpleMessage;
				}

				uint64_t Data1;
				uint64_t Data2;
				uint64_t Data3;

				MessageTypes MessageType;
				uint64_t SenderPID;
				uint64_t SenderTID;
				void (*Callback)();
		};

		extern "C" uint64_t Syscall0Param(uint64_t scvec);
		extern "C" uint64_t Syscall1Param(uint64_t scvec, uint64_t p1);
		extern "C" uint64_t Syscall2Param(uint64_t scvec, uint64_t p1, uint64_t p2);
		extern "C" uint64_t Syscall3Param(uint64_t scvec, uint64_t p1, uint64_t p2, uint64_t p3);
		extern "C" uint64_t Syscall4Param(uint64_t scvec, uint64_t p1, uint64_t p2, uint64_t p3, uint64_t p4);
		extern "C" uint64_t Syscall5Param(uint64_t scvec, uint64_t p1, uint64_t p2, uint64_t p3, uint64_t p4, uint64_t p5);

		void ExitProc();
		void Sleep(uint64_t);
		void Yield();
		void Block();

		void CreateThread(void (*)());
		void SpawnProcess(const char* path, const char* name);

		void InstallIRQHandler(uint64_t irq, uint64_t handleraddr);
		void InstallIRQHandlerWithRegs(uint64_t irq, uint64_t handleraddr);

		void SendSimpleMessageToProcess(uint64_t TargetPID, MessageTypes Type, uint64_t D1, uint64_t D2, uint64_t D3, void (*Callback)());
		void SendSimpleMessage(uint64_t TargetThreadID, MessageTypes Type, uint64_t D1, uint64_t D2, uint64_t D3, void (*Callback)());

		SimpleMessage* GetSimpleMessage();

		uint64_t MMap_Anonymous(uint64_t addr, uint64_t size, uint64_t prot, uint64_t flags);
		uint64_t Read(uint64_t sd, uint8_t* buffer, uint64_t length);
		uint64_t Write(uint64_t sd, uint8_t* buffer, uint64_t length);
		uint64_t Open(const char* path, uint64_t flags);
		void Close(uint64_t fd);

		sighandler_t InstallSignalHandler(uint64_t signum, sighandler_t handler);

		uint64_t GetPID();
		uint64_t GetParentPID();


	}
}
#else
#error unsupported
#endif




