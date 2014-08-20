// WindowDispatch.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under Creative Commons Attribution ShareAlike 3.0 Unported.

#include <IPC.hpp>
#include <Kernel.hpp>
#include <StandardIO.hpp>
#include <List.hpp>
#include "CentralDispatch.hpp"

using Library::LinkedList;
using namespace Kernel::HardwareAbstraction;
using namespace Kernel::HardwareAbstraction::Multitasking;
using namespace Library::StandardIO;
using Kernel::IPC::SimpleMessage;

namespace Kernel {
namespace IPC {
namespace CentralDispatch
{
	// defined service numbers:
	// 0: reserved
	// 1: 1000 Hz timer
	// 2: keyboard
	// 3: mouse

	// note that events 2 and 3 (or any other I/O events) will be handled by the windowing dispatcher.




	static LinkedList<GenericFocusableApplication>* ApplicationList;
	static GenericFocusableApplication* ApplicationInFocus;

	void InitialiseWindowDispatcher()
	{
		ApplicationList = new LinkedList<GenericFocusableApplication>();
	}

	Thread* WindowDispatcher(SimpleMessage* msg)
	{
		// same packet structure as CentralDispatch
		(void) msg;

		if(ApplicationInFocus)
			return ApplicationInFocus->Thread;

		return 0;
	}

	LinkedList<GenericFocusableApplication>* GetApplicationList()
	{
		return ApplicationList;
	}

	GenericFocusableApplication* GetCurrentApplicationInFocus()
	{
		return ApplicationInFocus;
	}

	void AddApplicationToList(Multitasking::Thread* thread, Multitasking::Process* proc)
	{
		AutoMutex mtx(Mutexes::WindowDispatcher);
		ApplicationList->InsertBack(new GenericFocusableApplication(proc, thread));

		// logically, when an application is added to the focusable list, it spawns a window, hence make it focused.
		ApplicationInFocus = ApplicationList->Back();
	}
}
}
}







