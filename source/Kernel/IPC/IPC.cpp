// IPC.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under Creative Commons Attribution ShareAlike 3.0 Unported.

#include <Kernel.hpp>
#include <IPC.hpp>
#include <HardwareAbstraction/MemoryManager.hpp>
#include <HardwareAbstraction/Multitasking.hpp>
#include <rdestl/rdestl.h>
#include <errno.h>
#include <sys/ipc.h>

using namespace Kernel::HardwareAbstraction::MemoryManager;
using namespace Kernel::HardwareAbstraction;

namespace Kernel {
namespace IPC
{
	struct defaultmsg
	{
		long type;
		char data[1];
	};

	// message queue array
	rde::hash_map<key_t, rde::list<uintptr_t>*>* messagequeue = nullptr;


	sighandler_t GetHandler(int sig);
	extern "C" void _sighandler();

	extern "C" void IPC_SignalThread(pid_t tid, int signum)
	{
		if(signum >= __SIGCOUNT)
		{
			Log(1, "Invalid signal number, ignoring");
			return;
		}

		Multitasking::Thread* thread = Multitasking::GetThread(tid);

		if(!thread || !thread->Parent)
		{
			Log(1, "Invalid target thread - %d", tid);
			return;
		}

		Multitasking::Process* tproc = thread->Parent;

		// ignored
		// cannot signal the kernel.
		if(tid < 2 || tproc->SignalHandlers[signum] == (sighandler_t) 1 || GetHandler(signum) == (sighandler_t) 1)
			return;

		// gotta find a way to wake up the process, but throw it into the signal handler.
		// also need to find a way to get it to return properly.

		/*
			Thread's stack:
			rdi		<-- offset 0.
			rsi
			...
			r8
			r9
			r10
			...
			r14
			r15

			-> RIP		<-- this is where we stuff the address of the handler.
			cs
			rflags
			stackptr
			ss

			1. map target thread's stack to current process space if required
			2. update RIP in stack. (patch iret)
			3. update RDI in stack. (patch first argument)
			4. todo: figure out a way to preserve contents of %rdi to avoid clobbering it.
			5. push (read: *--stack = return address) (ie. saved RIP)

			then we wake the thread. (multitasking.wakeformessage)
		*/

		uint64_t ptr = thread->StackPointer;
		uint64_t ustack = 0;
		if(thread->Parent != Multitasking::GetCurrentProcess())
		{
			// map kernel stack
			{
				// map.
				uint64_t s = thread->StackPointer;
				uint64_t diff = s - (s & ~0xFFF);

				uint64_t p1 = Virtual::GetMapping(s & ~0xFFF, thread->Parent->VAS->PML4);
				ptr = Virtual::AllocateVirtual();
				Virtual::MapAddress(ptr, p1, 0x7);

				ptr += diff;
			}

			// also need to map the user stack.
			{
				// map.
				uint64_t s = *((uint64_t*) (ptr + 144));
				uint64_t diff = s - (s & ~0xFFF);

				uint64_t p1 = Virtual::GetMapping(s & ~0xFFF, thread->Parent->VAS->PML4);
				ustack = Virtual::AllocateVirtual();
				Virtual::MapAddress(ustack, p1, 0x7);

				ustack += diff;
			}
		}

		sighandler_t handler = tproc->SignalHandlers[signum];
		if(handler == (sighandler_t) 0)
			handler = GetHandler(signum);


		if(thread != Multitasking::GetCurrentThread())
		{
			uint64_t oldrip					= *((uint64_t*) (ptr + 120));

			*((uint64_t*) (ptr + 120))			= (uint64_t) handler;
			*((uint64_t*) (ptr + 0))				= signum;		// signum

			*((uint64_t*) (ptr + 144))			-= 0x8;
			*((uint64_t*) (*((uint64_t*) (ptr + 144))))	= oldrip;



			// should be done, time to clean up.
			if(thread->Parent != Multitasking::GetCurrentProcess())
			{
				Virtual::FreeVirtual(ustack & ~0xFFF);
				Virtual::FreeVirtual(ptr & ~0xFFF);

				Virtual::UnMapAddress(ptr & ~0xFFF);
				Virtual::UnMapAddress(ustack & ~0xFFF);
			}

			Multitasking::WakeForMessage(thread);
		}
		else
		{
			using namespace Library::StandardIO;
			// fix iret to throw us into the sighandler, then return etc.
			// fetch our current stackpointer.
			uint64_t stackptr = 0;
			asm volatile("mov %%rsp, %[sp]" : [sp]"=m"(stackptr) :: "memory");

			uint64_t* rbp = (uint64_t*) stackptr;

			while(*rbp != 0xFFFFFFFFFFFFFFFF)
				rbp++;

			assert(*rbp == 0xFFFFFFFFFFFFFFFF);

			/*
				from syscall.s:
				CPU pushed:
				%ss
				%usp			(+12)
				%rflags
				%cs
				%rip			(+9)

				pushed by us:
				push %rbp		(+8)
				mov %rsp, %rbp

				push %r10		(+7)
				push %rdi
				push %rsi
				push %rdx
				push %rcx
				push %r8
				push %r9		(+1)

				// push a constant, so we know where to stop on stack backtrace.
				pushq $0xFFFFFFFFFFFFFFFF


				rbp now points to the -1 constant.
				rbp + 1 is %r9
				rbp + 7 is %r10.
				rbp + 8 is %rbp that we pushed

				finally, rbp + 9 is the return RIP pushed by the CPU, and we want to modify that.
				also, rbp + 12 is the return stack. we want to push a return address to that stack, such that
				control goes back to the interrupted function. (ie where we came from)

				this means decrementing the value in (rbp + 12) by 8, then putting the return address there.

				edit: fix it so function returns to _sighandler function in asm.
				this small function will restore the %rdi register that we clobbered.

				after the handler calls 'return', it goes to _sighandler.
				so, just push the actual return address, then push %rdi, then push the address of _sighandler.
			*/

			// store where we were interrupted.
			uint64_t oldrip			= *(rbp + 9);

			// modify.
			*(rbp + 9)			= (uint64_t) handler;

			*(rbp + 12)			-= 0x8;
			*((uint64_t*) (*(rbp + 12)))	= oldrip;

			*(rbp + 12)			-= 0x8;
			*((uint64_t*) (*(rbp + 12)))	= *(rbp + 6);

			*(rbp + 12)			-= 0x8;
			*((uint64_t*) (*(rbp + 12)))	= (uint64_t) _sighandler;


			// modify rdi (signum)
			*(rbp + 6)			= signum;
		}
	}

	extern "C" void IPC_SignalProcess(pid_t pid, int signum)
	{
		Multitasking::Process* proc = Multitasking::GetProcess(pid);
		assert(proc);
		IPC_SignalThread(proc->Threads->Get(0)->ThreadID, signum);
	}

	extern "C" int IPC_SendMessage(key_t key, void* msg, size_t size, uint64_t flags)
	{
		if(messagequeue == nullptr)
		{
			if(flags & IPC_CREAT)
				messagequeue = new rde::hash_map<key_t, rde::list<uintptr_t>*>();

			else
				return -1;	// todo: errno
		}

		// copy the message into the kernel heap.
		uint8_t* kernmsg = new uint8_t[size];
		memcpy(kernmsg, msg, size);

		// todo: something about flags
		rde::list<uintptr_t>* queue = (*messagequeue)[key];
		if(queue == nullptr)
			queue = new rde::list<uintptr_t>();

		// todo: block properly.
		if(queue->size() >= MaxMessages && flags & IPC_NOWAIT)
			return -1;	// todo: set errno to EAGAIN

		while(queue->size() >= MaxMessages);

		// add to the queue.
		queue->push_back((uintptr_t) kernmsg);

		// done.
		return 0;
	}

	extern "C" ssize_t IPC_ReceiveMessage(key_t key, void* msg, size_t size, uint64_t type, uint64_t flags)
	{
		if(messagequeue == nullptr)
			messagequeue = new rde::hash_map<key_t, rde::list<uintptr_t>*>();
	}














	// void SendSimpleMessageToProcess(uint64_t TargetPID, MessageTypes Type, uint64_t Data1, uint64_t Data2, uint64_t Data3, void (*Callback)())
	// {
	// 	Multitasking::Process* process = Multitasking::GetProcess(TargetPID);
	// 	if(!process)
	// 		return;

	// 	SendSimpleMessage(process->Threads->Get(0)->ThreadID, Type, Data1, Data2, Data3, Callback);
	// }

	// void SendSimpleMessage(uint64_t TargetThreadID, MessageTypes Type, uint64_t Data1, uint64_t Data2, uint64_t Data3, void (*Callback)())
	// {
	// 	Multitasking::Process* process = Multitasking::GetCurrentProcess();
	// 	Multitasking::Thread* thread = Multitasking::GetThread(TargetThreadID);
	// 	if(!thread || !thread->Parent)
	// 	{
	// 		Log(1, "Invalid target thread - %d", TargetThreadID);
	// 		return;
	// 	}

	// 	thread->SimpleMessageQueue->InsertFront(new SimpleMessage(process->ProcessID, Multitasking::GetCurrentThread()->ThreadID, Type, Data1, Data2, Data3, Callback));

	// 	// wake the thread for message.
	// 	Multitasking::WakeForMessage(thread);
	// }






	// SimpleMessage* HandleSimpleMessage(SimpleMessage* m)
	// {
	// 	// we can't be sending ACKs for ACK messages.
	// 	if(m->MessageType != MessageTypes::Acknowledge && m->MessageType != MessageTypes::AcknowledgeWithData)
	// 	{
	// 		// send an async message to the sender.
	// 		// SendMessage(m.sender, msgtype_ACK, 0, 0, 0)
	// 		return m;
	// 	}
	// 	else
	// 	{
	// 		// if it is an ACK message, we just have to run the callback function
	// 		// it will work, since ReceiveMessage will be called in the target process.
	// 		if(m->Callback)
	// 			m->Callback();


	// 		// certain message receivers, like the central dispatch thread, will
	// 		// respond to a request (ie. dispatch plug) with an ACK_DATA
	// 		// this essentially ACKs the sender, but also sends data.
	// 		// note we don't send an ACK to ACK that.

	// 		// therefore if it contains data, the receiver will want to handle it.
	// 		if(m->MessageType == MessageTypes::AcknowledgeWithData)
	// 			return m;

	// 		else
	// 			return 0;
	// 	}
	// }


	// SimpleMessage* GetSimpleMessage()
	// {
	// 	if(Multitasking::GetCurrentProcess()->SimpleMessageQueue->Size() > 0)
	// 	{
	// 		return HandleSimpleMessage(Multitasking::GetCurrentProcess()->SimpleMessageQueue->RemoveFront());
	// 	}
	// 	else if(Multitasking::GetCurrentThread()->SimpleMessageQueue->Size() > 0)
	// 	{
	// 		return HandleSimpleMessage(Multitasking::GetCurrentThread()->SimpleMessageQueue->RemoveFront());
	// 	}
	// 	else
	// 		return 0;
	// }


	// extern "C" void IPC_SimpleToProcess(uint64_t TargetPID, MessageTypes Type, uint64_t D1, uint64_t D2, uint64_t D3, void (*Callback)())
	// {
	// 	SendSimpleMessageToProcess(TargetPID, Type, D1, D2, D3, Callback);
	// }

	// extern "C" void IPC_SimpleToThread(uint64_t ThreadID, MessageTypes Type, uint64_t D1, uint64_t D2, uint64_t D3, void (*Callback)())
	// {
	// 	SendSimpleMessage(ThreadID, Type, D1, D2, D3, Callback);
	// }

	// extern "C" SimpleMessage* IPC_GetSimpleMessage()
	// {
	// 	return GetSimpleMessage();
	// }



	extern "C" sighandler_t Syscall_InstallSigHandler(uint64_t signum, sighandler_t handler)
	{
		if(signum >= __SIGCOUNT)
		{
			Log(1, "Error: invalid signal number %d", signum);
			return SIG_ERR;
		}

		else if(signum == SIGKILL || signum == SIGSTOP)
		{
			Log(1, "Error: cannot override handler for signal %d, which is either SIGKILL or SIGSTOP", signum);
			return SIG_ERR;
		}

		sighandler_t ret = Multitasking::GetCurrentProcess()->SignalHandlers[signum];
		Multitasking::GetCurrentProcess()->SignalHandlers[signum] = handler;
		return ret;
	}
}
}















