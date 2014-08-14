// IPC.hpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under Creative Commons Attribution ShareAlike 3.0 Unported.

#include <stdint.h>
#include <HardwareAbstraction/Filesystems.hpp>
#include <CircularBuffer.hpp>
#pragma once

#define MaxMessages 256

namespace Kernel {
namespace IPC
{
	class IPCSocket : public HardwareAbstraction::Filesystems::VFS::FSObject
	{
		public:
			IPCSocket(uint64_t pid, uint64_t buffersize = 256);
			~IPCSocket();
			void SetDescriptors(uint64_t one, uint64_t two);

			virtual const char* Name() override;
			virtual const char* Path() override;
			virtual FSObject* Parent() override;
			virtual HardwareAbstraction::Filesystems::VFS::Filesystem* RootFS() override;
			virtual uint8_t Attributes() override;
			virtual bool Exists() override;

			virtual uint64_t Read(uint8_t* buffer, uint64_t bytes);
			virtual uint64_t Write(uint8_t* buffer, uint64_t bytes);

			uint64_t pid1;
			uint64_t pid2;

			uint64_t sd1;
			uint64_t sd2;

			uint64_t bs;

		private:
			Library::CircularMemoryBuffer* buffer;
	};

	class IPCSocketEndpoint : public IPCSocket
	{
		public:
			IPCSocketEndpoint(uint64_t sd);
			virtual ~IPCSocketEndpoint() { }

			virtual uint64_t Read(uint8_t* buffer, uint64_t bytes) override;
			virtual uint64_t Write(uint8_t* buffer, uint64_t bytes) override;

			uint64_t desc;
	};

	uint64_t OpenIPCSocket(uint64_t pid, uint64_t size = 256);
	void CloseIPCSocket(uint64_t sd);
	uint64_t WriteToIPCSocket(uint64_t sd, uint8_t* data, uint64_t bytes);
	uint64_t ReadFromIPCSocket(uint64_t sd, uint8_t* data, uint64_t bytes);












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

	class MemoryMessage : public IMessage
	{
		public:
			MemoryMessage(uint64_t sp, uint64_t st, MessageTypes t, void* data, uint64_t size, void (*cb)())
			{
				this->SenderPID = sp;
				this->SenderTID = st;
				this->MessageType = t;
				this->Callback = cb;

				this->DataPointer = data;
				this->DataSize = size;

				this->Type = MessageClassTypes::T_MemoryMessage;
			}

			void* DataPointer;
			uint64_t DataSize;

			MessageTypes MessageType;
			uint64_t SenderPID;
			uint64_t SenderTID;
			void (*Callback)();
	};

	namespace CentralDispatch
	{
		void Initialise();
	}



	void SendMemoryMessageToProcess(uint64_t TargetPID, MessageTypes Type, void* Data, uint64_t DataSize, void (*Callback)());
	void SendMemoryMessage(uint64_t TargetThreadID, MessageTypes Type, void* Data, uint64_t DataSize, void (*Callback)());

	void SendSimpleMessageToProcess(uint64_t TargetPID, MessageTypes Type, uint64_t Data1, uint64_t Data2, uint64_t Data3, void (*Callback)());
	void SendSimpleMessage(uint64_t TargetThreadID, MessageTypes Type, uint64_t Data1, uint64_t Data2, uint64_t Data3, void (*Callback)());


	void SendMemoryMessageToProcess_Sync(uint64_t TargetPID, MessageTypes Type, void* Data, uint64_t DataSize, void (*Callback)());
	void SendMemoryMessage_Sync(uint64_t ThreadID, MessageTypes Type, void* Data, uint64_t DataSize, void (*Callback)());

	void SendSimpleMessageToProcess_Sync(uint64_t TargetPID, MessageTypes Type, uint64_t D1, uint64_t D2, uint64_t D3, void (*Callback)());
	void SendSimpleMessage_Sync(uint64_t ThreadID, MessageTypes Type, uint64_t D1, uint64_t D2, uint64_t D3, void (*Callback)());


	SimpleMessage* GetSimpleMessage();
	MemoryMessage* GetMemoryMessage();
}
}











