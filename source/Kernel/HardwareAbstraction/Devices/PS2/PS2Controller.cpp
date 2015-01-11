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
		const uint8_t DataPort = 0x60;
		const uint8_t CommandPort = 0x64;

		uint8_t Device1Buffer = 0;
		uint8_t Device2Buffer = 0;

		using namespace Kernel::HardwareAbstraction::Devices;

		extern "C" void HandleIRQ1()
		{
			Kernel::KernelKeyboard->HandleKeypress();
		}

		extern "C" void HandleIRQ12()
		{
			Device2Buffer = IOPort::ReadByte(DataPort);
		}

		void Initialise()
		{
			Kernel::KernelPS2Controller = new PS2Controller();
		}
	}

	PS2Controller::PS2Controller()
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



		// while(IOPort::ReadByte(PS2::CommandPort) & (1 << 1));
		// IOPort::WriteByte(PS2::DataPort, 0xF3);
		// IOPort::WriteByte(PS2::DataPort, 0x00);


		Kernel::HardwareAbstraction::Interrupts::SetGate(32 + 1, (uint64_t) ASM_HandlePS2IRQ1, 0x8, 0xEE);
		Kernel::HardwareAbstraction::Interrupts::SetGate(32 + 12, (uint64_t) ASM_HandlePS2IRQ12, 0x8, 0xEE);

		// should be ready to use.
	}

}

}
}
