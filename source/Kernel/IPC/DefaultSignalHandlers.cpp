// DefaultSignalHandlers.cpp
// Copyright (c) 2014 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <StandardIO.hpp>

#define TermProc	TerminateProcess
#define SuspProc	SuspendProcess
#define DumpProc	CoreDump
#define Discard	((sighandler_t) 1)
// #define ResmProc	ResumeProcess		// silence an unused macro warning.

namespace Kernel {
namespace IPC
{
	using namespace Kernel::HardwareAbstraction::Multitasking;
	void TerminateProcess(int sig)
	{
		StdIO::PrintFmt("Terminated PID %d: Signal %d", GetCurrentProcess()->ProcessID, sig);
		Kill(GetCurrentProcess());

		while(true);
	}

	void SuspendProcess(int)
	{
		Suspend(GetCurrentProcess());
	}

	void ResumeProcess(int)
	{
		Resume(GetCurrentProcess());
	}

	void CoreDump(int sig)
	{
		// todo: create core dump here
		TerminateProcess(sig);
	}

	sighandler_t GetHandler(int sig)
	{
		switch(sig)
		{
			case SIGHUP:		return TermProc;
			case SIGINT:		return TermProc;
			case SIGQUIT:		return DumpProc;
			case SIGILL:		return DumpProc;
			case SIGTRAP:		return DumpProc;
			case SIGABRT:		return DumpProc;
			case SIGEMT:		return DumpProc;
			case SIGFPE:		return DumpProc;
			case SIGKILL:		return TermProc;
			case SIGBUS:		return DumpProc;
			case SIGSEGV:		return DumpProc;
			case SIGSYS:		return DumpProc;
			case SIGPIPE:		return TermProc;
			case SIGALRM:		return TermProc;
			case SIGTERM:		return TermProc;
			case SIGURG:		return Discard;
			case SIGSTOP:		return SuspProc;
			case SIGTSTP:		return SuspProc;
			case SIGCONT:		return Discard;
			case SIGCHLD:		return Discard;
			case SIGTTIN:		return SuspProc;
			case SIGTTOU:		return SuspProc;
			case SIGIO:			return Discard;
			case SIGXCPU:		return TermProc;
			case SIGXFSZ:		return TermProc;
			case SIGVTALRM:		return TermProc;
			case SIGPROF:		return TermProc;
			case SIGWINCH:		return Discard;
			case SIGINFO:		return Discard;
			default:			return TermProc;
		}

		// return (sighandler_t) 1;
	}
}
}










