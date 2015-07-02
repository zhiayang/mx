// CircularBuffer.hpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#include <Kernel.hpp>
#include <HardwareAbstraction/Devices/IOPort.hpp>
#include <HardwareAbstraction/Interrupts.hpp>

extern "C" void ASM_HandlePS2IRQ1();
extern "C" void ASM_HandlePS2IRQ12();

namespace Kernel {
namespace HardwareAbstraction {
namespace Devices
{
	namespace PS2
	{
		using namespace DeviceManager;

		const uint8_t DataPort = 0x60;
		const uint8_t CommandPort = 0x64;

		using namespace Kernel::HardwareAbstraction::Devices;

		extern "C" void HandleIRQ1()
		{
			// not sure if this is a good idea, but...
			// if we get an IRQ1, it means that we have a PS/2 keyboard.
			// before this, we don't exactly know for sure. So it's kinda like a
			// "keyboard on demand" thing.

			rde::vector<Device*> kbs = GetDevices(DeviceType::Keyboard);
			Keyboard* thekb = 0;

			for(auto kb : kbs)
			{
				if(kb && ((Keyboard*) kb)->type == KeyboardInterface::PS2)
				{
					thekb = (Keyboard*) kb;
					break;
				}
			}

			if(thekb == 0)
			{
				thekb = new PS2Keyboard();
				AddDevice(thekb, DeviceType::Keyboard);
			}

			thekb->HandleKeypress();
		}

		extern "C" void HandleIRQ12()
		{
			uint8_t x = IOPort::ReadByte(DataPort);
			(void) x;
		}



		void Initialise()
		{
			// flush the buffer.
			IOPort::ReadByte(PS2::DataPort);

			// read the config byte.
			uint8_t ConfigByte = 0;

			IOPort::WriteByte(PS2::CommandPort, 0x20);

			while(!(IOPort::ReadByte(PS2::CommandPort) & 0x1));

			ConfigByte = IOPort::ReadByte(PS2::DataPort);
			ConfigByte &= ~(1 << 6);

			// write the value back
			IOPort::WriteByte(PS2::CommandPort, 0x60);

			while(IOPort::ReadByte(PS2::CommandPort) & 0x2);

			IOPort::WriteByte(PS2::DataPort, ConfigByte);


			Kernel::HardwareAbstraction::Interrupts::SetGate(32 + 1, (uint64_t) ASM_HandlePS2IRQ1, 0x8, 0xEE);
			Kernel::HardwareAbstraction::Interrupts::SetGate(32 + 12, (uint64_t) ASM_HandlePS2IRQ12, 0x8, 0xEE);

			// should be ready to use.
		}
	}
}

}
}
