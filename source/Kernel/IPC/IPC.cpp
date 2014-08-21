// IPC.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under Creative Commons Attribution ShareAlike 3.0 Unported.

#include <Kernel.hpp>
#include <IPC.hpp>
#include <HardwareAbstraction/MemoryManager.hpp>
#include <HardwareAbstraction/Multitasking.hpp>

using namespace Kernel::HardwareAbstraction::MemoryManager;
using namespace Kernel::HardwareAbstraction;

namespace Kernel {
namespace IPC
{
	sighandler_t GetHandler(int sig);
	extern "C" void _sighandler();


	void SendSimpleMessageToProcess(uint64_t TargetPID, MessageTypes Type, uint64_t Data1, uint64_t Data2, uint64_t Data3, void (*Callback)())
	{
		Multitasking::Process* process = Multitasking::GetProcess(TargetPID);
		if(!process)
			return;

		SendSimpleMessage(process->Threads->Get(0)->ThreadID, Type, Data1, Data2, Data3, Callback);
	}

	void SendSimpleMessage(uint64_t TargetThreadID, MessageTypes Type, uint64_t Data1, uint64_t Data2, uint64_t Data3, void (*Callback)())
	{
		Multitasking::Process* process = Multitasking::GetCurrentProcess();
		Multitasking::Thread* thread = Multitasking::GetThread(TargetThreadID);
		if(!thread || !thread->Parent)
		{
			Log(1, "Invalid target thread - %d", TargetThreadID);
			return;
		}

		Multitasking::Process* tproc = thread->Parent;

		if(Type != MessageTypes::PosixSignal)
		{
			thread->SimpleMessageQueue->InsertFront(new SimpleMessage(process->ProcessID, Multitasking::GetCurrentThread()->ThreadID, Type, Data1, Data2, Data3, Callback));

			// wake the thread for message.
			Multitasking::WakeForMessage(thread);
		}
		else
		{
			if(Data1 >= __SIGCOUNT)
			{
				Log(1, "Invalid signal number, ignoring");
				return;
			}

			// ignored
			// cannot signal the kernel.
			if(TargetThreadID < 2 || tproc->SignalHandlers[Data1] == (sighandler_t) 1 || GetHandler((int) Data1) == (sighandler_t) 1)
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
			if(thread->Parent != process)
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

			sighandler_t handler = tproc->SignalHandlers[Data1];
			if(handler == (sighandler_t) 0)
				handler = GetHandler((int) Data1);


			if(thread != Multitasking::GetCurrentThread())
			{
				uint64_t oldrip				= *((uint64_t*) (ptr + 120));

				*((uint64_t*) (ptr + 120))			= (uint64_t) handler;
				*((uint64_t*) (ptr + 0))			= Data1;		// signum

				*((uint64_t*) (ptr + 144))			-= 0x8;
				*((uint64_t*) (*((uint64_t*) (ptr + 144))))	= oldrip;

				// check: redzone thing (edit: bad idea)
				// *((uint64_t*) (ptr + 144))			-= 128;




				// should be done, time to clean up.
				if(thread->Parent != process)
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
				uint64_t oldrip		= *(rbp + 9);

				// modify.
				*(rbp + 9)			= (uint64_t) handler;

				*(rbp + 12)			-= 0x8;
				*((uint64_t*) (*(rbp + 12)))	= oldrip;

				*(rbp + 12)			-= 0x8;
				*((uint64_t*) (*(rbp + 12)))	= *(rbp + 6);

				*(rbp + 12)			-= 0x8;
				*((uint64_t*) (*(rbp + 12)))	= (uint64_t) _sighandler;


				// modify rdi (signum)
				*(rbp + 6)			= Data1;

				// check: redzone thing
				// *(rbp + 12)			-= 128;
			}
		}
	}






	SimpleMessage* HandleSimpleMessage(SimpleMessage* m)
	{
		// we can't be sending ACKs for ACK messages.
		if(m->MessageType != MessageTypes::Acknowledge && m->MessageType != MessageTypes::AcknowledgeWithData)
		{
			// send an async message to the sender.
			// SendMessage(m.sender, msgtype_ACK, 0, 0, 0)
			return m;
		}
		else
		{
			// if it is an ACK message, we just have to run the callback function
			// it will work, since ReceiveMessage will be called in the target process.
			if(m->Callback)
				m->Callback();


			// certain message receivers, like the central dispatch thread, will
			// respond to a request (ie. dispatch plug) with an ACK_DATA
			// this essentially ACKs the sender, but also sends data.
			// note we don't send an ACK to ACK that.

			// therefore if it contains data, the receiver will want to handle it.
			if(m->MessageType == MessageTypes::AcknowledgeWithData)
				return m;

			else
				return 0;
		}
	}


	SimpleMessage* GetSimpleMessage()
	{
		if(Multitasking::GetCurrentProcess()->SimpleMessageQueue->Size() > 0)
		{
			return HandleSimpleMessage(Multitasking::GetCurrentProcess()->SimpleMessageQueue->RemoveFront());
		}
		else if(Multitasking::GetCurrentThread()->SimpleMessageQueue->Size() > 0)
		{
			return HandleSimpleMessage(Multitasking::GetCurrentThread()->SimpleMessageQueue->RemoveFront());
		}
		else
			return 0;
	}


	extern "C" void IPC_SimpleToProcess(uint64_t TargetPID, MessageTypes Type, uint64_t D1, uint64_t D2, uint64_t D3, void (*Callback)())
	{
		SendSimpleMessageToProcess(TargetPID, Type, D1, D2, D3, Callback);
	}

	extern "C" void IPC_SimpleToThread(uint64_t ThreadID, MessageTypes Type, uint64_t D1, uint64_t D2, uint64_t D3, void (*Callback)())
	{
		SendSimpleMessage(ThreadID, Type, D1, D2, D3, Callback);
	}

	extern "C" SimpleMessage* IPC_GetSimpleMessage()
	{
		return GetSimpleMessage();
	}



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















