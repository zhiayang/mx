// Interrupts.c
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


// Does good things about interrupt handling.

#include <Kernel.hpp>
#include <HardwareAbstraction/Interrupts.hpp>
#include <HardwareAbstraction/Devices/IOPort.hpp>
#include <Memory.hpp>
#include <StandardIO.hpp>

using namespace Kernel;
using namespace Library;
using namespace Kernel::HardwareAbstraction::Devices;


namespace Kernel {
namespace HardwareAbstraction {
namespace Interrupts
{
	extern "C" void ThreadExceptionTerminate();

	rde::vector<IRQHandlerPlugList*>* IRQHandlerList;


	static IRQHandlerPlugList* GetPlugList(uint64_t irq)
	{
		// see if we need to create a new plug list.
		IRQHandlerPlugList* pluglist = 0;
		for(uint64_t i = 0; i < IRQHandlerList->size(); i++)
		{
			if(irq == (*IRQHandlerList)[i]->IRQNum)
			{
				pluglist = (*IRQHandlerList)[i];
				break;
			}
		}

		if(!pluglist)
		{
			pluglist = new IRQHandlerPlugList(irq);
			IRQHandlerList->push_back(pluglist);
			Log("Creating new PlugList for IRQ %d", irq);
		}

		return pluglist;
	}

	void InstallIRQHandler(uint64_t irq, void(*Handler)(void*), void* arg)
	{
		// add it to the list.
		GetPlugList(irq)->HandlerList.push_back(new IRQHandlerPlug(Handler, arg));
	}

	void UninstallIRQHandler(uint64_t irq)
	{
		HALT("Unimplemented");
		UNUSED(irq);
	}













	extern "C" void InterruptHandler_C(uint64_t iid)
	{
		// get the proper IRQHandlerPlugList.
		for(uint64_t i = 0; i < IRQHandlerList->size(); i++)
		{
			// operate with zero-based index, not absolute IDT offset.
			if((*IRQHandlerList)[i]->IRQNum == iid - 32)
			{
				IRQHandlerPlugList* pl = (*IRQHandlerList)[i];

				for(uint64_t k = 0; k < pl->HandlerList.size(); k++)
				{
					IRQHandlerPlug* handler = pl->HandlerList[k];
					handler->handle(handler->arg);
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
















