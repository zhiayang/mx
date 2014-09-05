// CentralDispatch.cpp
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
	#define MaxServices			4096

	// defined service numbers:
	// 0: reserved
	// 1: 1000 Hz timer
	// 2: keyboard
	// 3: mouse

	// note that events 2 and 3 (or any other I/O events) will be handled by the windowing dispatcher.


	static Service_type* ServiceList[256] = { 0 };

	Service_type** GetServiceHandlers()
	{
		return ServiceList;
	}

	void DispatchLoop()
	{
		while(true)
		{
			// check for directed messages.
			// to simplify this, we only check for messages sent to our process.
			// also we will only respond to SimpleMessages as service requests.

			// incoming packet structure:
			// D1: service number
			// D2: data
			// D3: data

			// this will only wake us up when there's a message, so we don't need to check for msg being 0.
			BLOCK();
			SimpleMessage* msg = GetSimpleMessage();
			if(!msg)
				continue;




			uint64_t ServiceNumber = msg->Data1;
			if(msg->MessageType == MessageTypes::RequestServiceDispatch)
			{
				if(ServiceNumber > MaxServices)
				{
					Log("Max service number is %d, %d invalid.", MaxServices, ServiceNumber);
					continue;
				}

				if(!ServiceList[ServiceNumber])
				{
					ServiceList[ServiceNumber] = new Service_type(ServiceNumber);

					// this doesn't really make much sense, but we can use it as a limited form of pipe
					// a process can start listening in on a channel (ie. send a request to centraldispatch)
					// and make it look like a normal service request

					// CD will happily initialise the channel. But since no drivers send on that channel,
					// it is essentially free to use.
					// therefore other programs can send messages to that service through this channel
					// in the event of unexpected shutdown, all perosnnel are to evacuate the area immediately. Thank yo for your cooperation in this matter, we hope to work with you again in the future, thank you.
				}

				Log("Thread %d requested service dispatch of Service <%d>", msg->SenderTID, ServiceNumber);
				ServiceList[ServiceNumber]->Handlers->push_back(new ServiceHandler_type(msg->SenderTID, Multitasking::GetThread(msg->SenderTID)));
			}
			else if(msg->MessageType == MessageTypes::RequestServiceInitialise)
			{
				// service init request
				ServiceNumber = msg->Data2;
				ServiceList[ServiceNumber] = new Service_type(ServiceNumber);

				Log("Initialised service vector (%d) with Central Dispatch", ServiceNumber);
			}
			else if(msg->MessageType == MessageTypes::ServiceData)
			{
				ServiceNumber &= 0xFFFFFFFF;
				// Thread* thread = 0;

				// if(ServiceNumber == 2 || ServiceNumber == 3)
				// {
				// 	thread = WindowDispatcher(msg);
				// }

				if(ServiceList[ServiceNumber])
				{
					// forward the message to registered listeners
					for(uint64_t i = 0; i < ServiceList[ServiceNumber]->Handlers->size(); i++)
					{
						// if(!thread)
						// {
							SendSimpleMessage(ServiceList[ServiceNumber]->Handlers->get(i)->target, MessageTypes::ServiceData, msg->Data1, msg->Data2, msg->Data3, 0);
						// }
						// else
						// {
							// SendSimpleMessage(thread->ThreadID, MessageTypes::ServiceData, msg->Data1, msg->Data2, msg->Data3, 0);
						// }
					}
				}
			}
		}
	}

	void Initialise()
	{
		DispatchLoop();
	}
}
}
}







