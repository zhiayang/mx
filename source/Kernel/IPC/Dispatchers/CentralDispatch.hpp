// CentralDispatch.hpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <IPC.hpp>
#include <List.hpp>
#include <HardwareAbstraction/Multitasking.hpp>

using Library::LinkedList;

namespace Kernel {
namespace IPC {
namespace CentralDispatch
{
	class ServiceHandler_type
	{
		public:
			ServiceHandler_type(uint64_t tar, HardwareAbstraction::Multitasking::Thread* thrd) : target(tar), thread(thrd) { }
			uint64_t target;
			HardwareAbstraction::Multitasking::Thread* thread;
	};

	class Service_type
	{
		public:
			Service_type(uint64_t sn) : servicenumber(sn)
			{
				this->Handlers = new LinkedList<ServiceHandler_type>();
			}

			uint64_t servicenumber;
			LinkedList<ServiceHandler_type>* Handlers;
	};


	class GenericFocusableApplication
	{
		public:
			GenericFocusableApplication(HardwareAbstraction::Multitasking::Process* p, HardwareAbstraction::Multitasking::Thread* t) : Process(p), Thread(t) { }


			HardwareAbstraction::Multitasking::Process* Process;
			HardwareAbstraction::Multitasking::Thread* Thread;
	};

	void InitialiseWindowDispatcher();
	HardwareAbstraction::Multitasking::Thread* WindowDispatcher(SimpleMessage* msg);
	Service_type** GetServiceHandlers();

	LinkedList<GenericFocusableApplication>* GetApplicationList();
	GenericFocusableApplication* GetCurrentApplicationInFocus();

	void AddApplicationToList(HardwareAbstraction::Multitasking::Thread* thread, HardwareAbstraction::Multitasking::Process* proc);
}
}
}
