// Signals.cpp
// Copyright (c) 2014 - 2016, zhiayang@gmail.com
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
	// message queue array

	sighandler_t GetHandler(int sig);
	extern "C" void _sighandler();

	extern "C" void IPC_SignalThread(pid_t tid, int signum)
	{
		if(signum >= __SIGCOUNT)
		{
			Log(1, "Invalid signal number, ignoring");
			Multitasking::SetThreadErrno(EINVAL);
			return;
		}

		Multitasking::Thread* thread = Multitasking::GetThread(tid);

		if(!thread || !thread->Parent)
		{
			Log(1, "Invalid target thread - %d", tid);
			Multitasking::SetThreadErrno(ESRCH);
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
				uint64_t diff = s - (s & ((uint64_t) ~0xFFF));

				uint64_t p1 = Virtual::GetMapping(s & ((uint64_t) ~0xFFF), thread->Parent->VAS.PML4);
				ptr = Virtual::AllocateVirtual();
				Virtual::MapAddress(ptr, p1, 0x7);

				ptr += diff;
			}

			// also need to map the user stack.
			{
				// map.
				uint64_t s = *((uint64_t*) (ptr + 144));
				uint64_t diff = s - (s & ((uint64_t) ~0xFFF));

				uint64_t p1 = Virtual::GetMapping(s & ((uint64_t) ~0xFFF), thread->Parent->VAS.PML4);
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

			*((uint64_t*) (ptr + 120))		= (uint64_t) handler;
			*((uint64_t*) (ptr + 0))		= (uint64_t) signum;		// signum

			*((uint64_t*) (ptr + 144))		-= 0x8;
			*((uint64_t*) (*((uint64_t*) (ptr + 144))))	= oldrip;



			// should be done, time to clean up.
			if(thread->Parent != Multitasking::GetCurrentProcess())
			{
				Virtual::FreeVirtual(ustack & ((uint64_t) ~0xFFF), 1);
				Virtual::FreeVirtual(ptr & ((uint64_t) ~0xFFF), 1);

				Virtual::UnmapAddress(ptr & ((uint64_t) ~0xFFF));
				Virtual::UnmapAddress(ustack & ((uint64_t) ~0xFFF));
			}

			Multitasking::WakeForMessage(thread);
		}
		else
		{
			// fix iret to throw us into the sighandler, then return etc.
			// fetch our current stackpointer.
			uint64_t stackptr = 0;
			asm volatile("mov %%rsp, %[sp]" : [sp]"=m"(stackptr) :: "memory");

			uint64_t* rbp = (uint64_t*) stackptr;
			if(rbp == 0)
				HALT("stackptr is null");

			#define STACK_MAGIC_CANARY 0xFFFFFFFFDEADBEEF

			while(*rbp != STACK_MAGIC_CANARY)
				rbp++;

			assert(*rbp == STACK_MAGIC_CANARY);

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
				pushq $0xFADD00D15CAFED06

			*/




			/*
				*** steps to undertake ***

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

				note: this might be unsafe if the user stack in question doesn't have
				0x18 extra bytes to spare.
			*/

			// store where we were interrupted.
			uint64_t oldrip					= *(rbp + 9);

			// modify.
			*(rbp + 9)						= (uint64_t) handler;

			*(rbp + 12)						-= 0x8;
			*((uint64_t*) (*(rbp + 12)))	= oldrip;

			*(rbp + 12)						-= 0x8;
			*((uint64_t*) (*(rbp + 12)))	= *(rbp + 6);

			*(rbp + 12)						-= 0x8;
			*((uint64_t*) (*(rbp + 12)))	= (uint64_t) _sighandler;

			// modify rdi (signum)
			*(rbp + 6) = (uint64_t) signum;
		}
	}

	extern "C" void IPC_SignalProcess(pid_t pid, int signum)
	{
		Multitasking::Process* proc = Multitasking::GetProcess(pid);
		assert(proc);
		IPC_SignalThread(proc->Threads.front()->ThreadID, signum);
	}















	extern "C" sighandler_t Syscall_InstallSigHandler(uint64_t signum, sighandler_t handler)
	{
		if(signum >= __SIGCOUNT)
		{
			Log(1, "Error: invalid signal number %d", signum);
			Multitasking::SetThreadErrno(EINVAL);
			return SIG_ERR;
		}

		else if((signum == SIGKILL || signum == SIGSTOP) && handler != SIG_DFL)
		{
			Log(1, "Error: cannot override handler for signal %d, which is either SIGKILL or SIGSTOP", signum);
			Multitasking::SetThreadErrno(EINVAL);
			return SIG_ERR;
		}

		sighandler_t ret = Multitasking::GetCurrentProcess()->SignalHandlers[signum];
		Multitasking::GetCurrentProcess()->SignalHandlers[signum] = handler;
		return ret;
	}
}
}















