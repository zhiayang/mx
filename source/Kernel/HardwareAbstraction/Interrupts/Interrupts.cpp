// Interrupts.c
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


// Does good things about interrupt handling.

#include <Kernel.hpp>
#include <HardwareAbstraction/Interrupts.hpp>
#include <HardwareAbstraction/Devices/IOPort.hpp>
#include <Memory.hpp>
#include <StandardIO.hpp>
#include <List.hpp>
#include <Colours.hpp>

using namespace Kernel;
using namespace Library;
using Library::LinkedList;
using namespace StandardIO;
using namespace Kernel::HardwareAbstraction::Devices;


namespace Kernel {
namespace HardwareAbstraction {
namespace Interrupts
{
	extern "C" void ThreadExceptionTerminate();

	LinkedList<IRQHandlerPlugList>* IRQHandlerList;






	static IRQHandlerPlugList* GetPlugList(uint64_t irq)
	{
		// see if we need to create a new plug list.
		IRQHandlerPlugList* pluglist = 0;
		for(uint64_t i = 0; i < IRQHandlerList->Size(); i++)
		{
			if(irq == IRQHandlerList->Get(i)->IRQNum)
			{
				pluglist = IRQHandlerList->Get(i);
				break;
			}
		}

		if(!pluglist)
		{
			pluglist = new IRQHandlerPlugList(irq);
			IRQHandlerList->InsertBack(pluglist);
			Log("Creating new PlugList for IRQ %d", irq);
		}

		return pluglist;
	}

	/* This installs a custom IRQ handler for the given IRQ */
	void InstallIRQHandler(uint64_t irq, void (*Handler)(RegisterStruct_type* r))
	{
		// add it to the list.
		GetPlugList(irq)->HandlerList->InsertBack(new IRQHandlerPlug(Handler, Multitasking::GetCurrentProcess()));
	}

	void InstallIRQHandler(uint64_t irq, void(*Handler)())
	{
		// add it to the list.
		GetPlugList(irq)->HandlerList->InsertBack(new IRQHandlerPlug(Handler, Multitasking::GetCurrentProcess()));
	}

	void UninstallIRQHandler(uint64_t irq)
	{
		UNUSED(irq);
	}













	extern "C" void InterruptHandler_C(uint64_t iid)
	{
		// get the proper IRQHandlerPlugList.
		for(uint64_t i = 0; i < IRQHandlerList->Size(); i++)
		{
			// operate with zero-based index, not absolute IDT offset.
			if(IRQHandlerList->Get(i)->IRQNum == iid - 32)
			{
				IRQHandlerPlugList* pl = IRQHandlerList->Get(i);

				for(uint64_t k = 0; k < pl->HandlerList->Size(); k++)
				{
					if(pl->HandlerList->Get(k)->handleregs)
					{
						HALT("not supported");
					}

					else
						pl->HandlerList->Get(k)->handle();
				}

				// send a message to central dispatch, informing of this IRQ.
				// don't send for timer interrupts, aka IRQ0 and IRQ8.
				if(iid != 32 + 0 && iid != 32 + 8)
				{
					// IPC::SendSimpleMessage(0, IPC::MessageTypes::RequestServiceDispatch, 0, 0, iid - 32, 0);
				}
			}
		}


		// This IRQ is >7, send an EOI to the slave controller too.
		if(iid >= 40)
		{
			IOPort::WriteByte(0xA0, 0x20);
		}

		// We need to send the ACK to the master regardless.
		IOPort::WriteByte(0x20, 0x20);
		return;
	}
}

}
}















