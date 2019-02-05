// StreamingIPC.cpp
// Copyright (c) 2014 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <IPC.hpp>
#include <HardwareAbstraction/Filesystems.hpp>
#include <String.hpp>

using namespace Library;
using namespace Kernel::HardwareAbstraction;
using namespace Kernel::HardwareAbstraction::Filesystems::VFS;

namespace Kernel {
namespace IPC
{
	// IPCSocket* VerfiySocket(Multitasking::Process* proc, uint64_t sd)
	// {
	// 	if(sd >= MaxDescriptors ||  proc->FileDescriptors[sd].Pointer == 0)
	// 	{
	// 		Log(1, "Error: Descriptor %d does not exist, or has not been opened.", sd);
	// 		return 0;
	// 	}

	// 	FSObject* fso = proc->FileDescriptors[sd].Pointer;
	// 	if(fso->Type != FSObjectTypes::IPCSocket)
	// 	{
	// 		Log(1, "Error: Object %s(%d) is not a socket, but is %d.", fso->Name(), sd, fso->Type);
	// 		return 0;
	// 	}

	// 	return (IPCSocket*) fso;
	// }

	// uint64_t OpenIPCSocket(uint64_t pid, uint64_t size)
	// {
	// 	// Multitasking::Process* proc = Multitasking::GetCurrentProcess();
	// 	// Multitasking::Process* rem = Multitasking::GetProcess(pid);

	// 	// IPCSocket* socket = new IPCSocket(pid, size);

	// 	// // create a new entry in the current process's fd list.
	// 	// uint64_t locald = GetAndIncrementDescriptor(proc);
	// 	// proc->FileDescriptors[locald].Pointer = socket;

	// 	// // create a new entry in the remote process's fd list.
	// 	// uint64_t remoted = GetAndIncrementDescriptor(rem);
	// 	// rem->FileDescriptors[remoted].Pointer = socket;

	// 	// socket->SetDescriptors(locald, remoted);

	// 	// return locald;
	// }

	// void CloseIPCSocket(uint64_t sd)
	// {
	// 	// Multitasking::Process* proc = Multitasking::GetCurrentProcess();
	// 	// IPCSocket* socket = VerfiySocket(proc, sd);

	// 	// Multitasking::Process* rem = Multitasking::GetProcess(proc->ProcessID == socket->pid1 ? socket->pid2 : socket->pid1);
	// 	// assert(socket == VerfiySocket(rem, sd));

	// 	// uint64_t thispid = (proc->ProcessID == socket->pid1 ? 1 : 2);
	// 	// if(thispid == 1)
	// 	// {
	// 	// 	proc->FileDescriptors[socket->sd1].Pointer = 0;
	// 	// 	rem->FileDescriptors[socket->sd2].Pointer = 0;
	// 	// }
	// 	// else if(thispid == 2)
	// 	// {
	// 	// 	proc->FileDescriptors[socket->sd2].Pointer = 0;
	// 	// 	rem->FileDescriptors[socket->sd1].Pointer = 0;
	// 	// }

	// 	// // remove from both sides, but only delete once.
	// 	// delete socket;
	// }

	// uint64_t WriteToIPCSocket(uint64_t sd, uint8_t* data, uint64_t bytes)
	// {
	// 	// IPCSocket* socket = VerfiySocket(Multitasking::GetCurrentProcess(), sd);
	// 	// if(!socket)
	// 	// 	return 0;

	// 	// return socket->Write(data, bytes);
	// }

	// uint64_t ReadFromIPCSocket(uint64_t sd, uint8_t* data, uint64_t bytes)
	// {
	// 	// IPCSocket* socket = VerfiySocket(Multitasking::GetCurrentProcess(), sd);
	// 	// if(!socket)
	// 	// 	return 0;

	// 	// return socket->Read(data, bytes);
	// }












	// // IPCSocket::IPCSocket(uint64_t p, uint64_t size) : FSObject(FSObjectTypes::IPCSocket)
	// // {
	// // 	// this->pid1 = Multitasking::GetCurrentProcess()->ProcessID;
	// // 	// this->pid2 = p;
	// // 	// this->buffer = new CircularMemoryBuffer(size);

	// // 	// this->sd1 = 0;
	// // 	// this->sd2 = 0;

	// // 	// this->bs = size;

	// // 	// this->Type = FSObjectTypes::IPCSocket;
	// // }

	// // void IPCSocket::SetDescriptors(uint64_t one, uint64_t two)
	// // {
	// // 	this->sd1 = one;
	// // 	this->sd2 = two;
	// // }

	// // IPCSocket::~IPCSocket()
	// // {
	// // 	delete this->buffer;
	// // }

	// // const char* IPCSocket::Name()
	// // {
	// // 	string* s = new string;
	// // 	StandardIO::PrintToString(s, "IPC socket between PID %d (%s) and PID %d (%s)", this->pid1, Multitasking::GetProcess(this->pid1)->Name, this->pid2, Multitasking::GetProcess(this->pid2)->Name);

	// // 	char* str = new char[s->Length()];
	// // 	String::Copy(str, s->CString());

	// // 	delete s;
	// // 	return str;
	// // }

	// // const char* IPCSocket::Path()
	// // {
	// // 	return this->Name();
	// // }

	// // FSObject* IPCSocket::Parent()
	// // {
	// // 	return nullptr;
	// // }

	// // Filesystem* IPCSocket::RootFS()
	// // {
	// // 	return nullptr;
	// // }

	// // uint64_t IPCSocket::Read(uint8_t* data, uint64_t bytes)
	// // {
	// // 	uint64_t read = math::min(bytes, this->buffer->ByteCount());
	// // 	this->buffer->Read(data, bytes);
	// // 	return read;
	// // }

	// // uint64_t IPCSocket::Write(uint8_t* data, uint64_t bytes)
	// // {
	// // 	uint64_t wrote = math::min(bytes, this->buffer->ByteCount());
	// // 	this->buffer->Write(data, bytes);
	// // 	return wrote;
	// // }

	// // uint8_t IPCSocket::Attributes()
	// // {
	// // 	return 0;
	// // }

	// // bool IPCSocket::Exists()
	// // {
	// // 	return true;
	// // }


















	// // IPCSocketEndpoint::IPCSocketEndpoint(uint64_t sd) : IPCSocket(0, 0)
	// // {
	// // 	this->desc = sd;
	// // }

	// // uint64_t IPCSocketEndpoint::Read(uint8_t* buffer, uint64_t bytes)
	// // {
	// // 	UNUSED(bytes);
	// // 	switch(this->desc)
	// // 	{
	// // 		case 0:
	// // 		case 1:
	// // 			if(Kernel::KernelKeyboard->ItemsInBuffer())
	// // 			{
	// // 				*buffer = Kernel::KernelKeyboard->ReadBuffer();
	// // 				return 1;
	// // 			}
	// // 			break;

	// // 		case 2:
	// // 		case 3:
	// // 			break;
	// // 	}

	// // 	return 0;
	// // }

	// // uint64_t IPCSocketEndpoint::Write(uint8_t* buffer, uint64_t bytes)
	// // {
	// // 	switch(this->desc)
	// // 	{
	// // 		case 0:
	// // 		case 1:
	// // 			StandardIO::PrintString((const char*) buffer, bytes);
	// // 			return bytes;

	// // 		case 2:
	// // 		case 3:
	// // 			Log((const char*) buffer);
	// // 			return bytes;
	// // 	}

	// // 	return 0;
	// // }


	// extern "C" uint64_t IPC_OpenSocket(uint64_t pid, uint64_t size)
	// {
	// 	return OpenIPCSocket(pid, size);
	// }
	// extern "C" void IPC_CloseSocket(uint64_t sd)
	// {
	// 	CloseIPCSocket(sd);
	// }

	// extern "C" uint64_t IPC_WriteSocket(uint64_t sd, uint64_t data, uint64_t bytes)
	// {
	// 	return (uint64_t) WriteToIPCSocket(sd, (uint8_t*) data, bytes);
	// }

	// extern "C" uint64_t IPC_ReadSocket(uint64_t sd, uint64_t data, uint64_t bytes)
	// {
	// 	return (uint64_t) ReadFromIPCSocket(sd, (uint8_t*) data, bytes);
	// }
}
}













