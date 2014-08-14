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


		void PrintChar(uint8_t ch);
		void PrintString(const char* str);

		void Sleep(uint64_t);
		void CreateThread(void (*)());

		void InstallIRQHandler(uint64_t irq, uint64_t handleraddr);
		void InstallIRQHandlerWithRegs(uint64_t irq, uint64_t handleraddr);

		uint64_t GetFramebufferAddress();
		void SetConsoleBackColour(uint32_t c);
		uint64_t GetFramebufferResX();
		uint64_t GetFramebufferResY();

		void MapVirtualAddress(uint64_t v, uint64_t p, uint64_t f);
		void UnmapVirtualAddress(uint64_t v);
		uint64_t AllocatePage();
		void FreePage(uint64_t p);

		uint32_t GetBackgroundColour();
		uint32_t GetTextColour();
		void SetTextColour(uint32_t c);


		uint64_t OpenFile(const char* path, uint8_t mode);
		void CloseFile(uint64_t fd);
		uint8_t* ReadFile(uint64_t fd, uint8_t* buffer, uint64_t length);
		uint8_t* WriteFile(uint64_t fd, uint8_t* buffer, uint64_t length);

		uint64_t OpenFolder(const char* path);
		void CloseFolder(uint64_t fd);
		void* ListObjects(uint64_t fd, void* output, uint64_t* items);

		uint64_t GetFileSize(uint64_t fd);

		bool CheckFileExistence(const char* path);
		bool CheckFolderExistence(const char* path);

		void SendSimpleMessageToProcess(uint64_t TargetPID, MessageTypes Type, uint64_t D1, uint64_t D2, uint64_t D3, void (*Callback)());
		void SendSimpleMessage(uint64_t TargetThreadID, MessageTypes Type, uint64_t D1, uint64_t D2, uint64_t D3, void (*Callback)());

		SimpleMessage* GetSimpleMessage();


		void Yield();
		void Block();


		void ClearScreen();
		void SpawnProcess(const char* path, const char* name);

		uint64_t GetCursorX();
		uint64_t GetCursorY();
		void SetCursorPos(uint64_t x, uint64_t y);
		uint64_t GetCharWidth();
		uint64_t GetCharHeight();

		void PutPixelAtXY(uint64_t x, uint64_t y, uint64_t Colour);
		void PutCharNoMove(uint8_t c);

		void OpenIPCSocket(uint64_t pid, uint64_t size = 256);
		void CloseIPCSocket(uint64_t sd);
		uint8_t* ReadIPCSocket(uint64_t sd, uint8_t* buffer, uint64_t length);
		uint8_t* WriteIPCSocket(uint64_t sd, uint8_t* buffer, uint64_t length);

		uint64_t MMap_Anonymous(uint64_t addr, uint64_t size, uint64_t prot, uint64_t flags);
		uint64_t ReadFromAny(uint64_t sd, uint8_t* buffer, uint64_t length);
		uint64_t WriteToAny(uint64_t sd, uint8_t* buffer, uint64_t length);
		uint64_t OpenAny(const char* path, uint64_t flags);
		void CloseAny(uint64_t fd);

		sighandler_t InstallSignalHandler(uint64_t signum, sighandler_t handler);

		uint64_t GetPID();
		uint64_t GetParentPID();

		struct RegisterStruct_type
		{
			uint64_t cr2;
			uint64_t rsp;
			uint64_t rdi, rsi, rbp;
			uint64_t rax, rbx, rcx, rdx, r8, r9, r10, r11, r12, r13, r14, r15;

			uint64_t InterruptID, ErrorCode;
			uint64_t rip, cs, rflags, useresp, ss;
		};

	}
}
#else
#error unsupported
#endif




