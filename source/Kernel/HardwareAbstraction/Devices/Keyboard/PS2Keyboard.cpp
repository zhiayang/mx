// PS2Keyboard.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <HardwareAbstraction/Devices/IOPort.hpp>
#include "Translation.hpp"

namespace Kernel {
namespace HardwareAbstraction {
namespace Devices
{
	using namespace Kernel::HardwareAbstraction::Devices;
	using Library::CircularMemoryBuffer;

	#define BufferSize 256

	static bool Shifting = false;
	static bool CapsLock = false;

	PS2Keyboard::PS2Keyboard()
	{
		IOPort::ReadByte(PS2::DataPort);
		// this->Buffer = new CircularBuffer<uint8_t>(BufferSize);
		// this->ByteBuffer = new CircularBuffer<uint8_t>(BufferSize * 2);

		this->Buffer = new CircularMemoryBuffer(BufferSize);
		this->ByteBuffer = new CircularMemoryBuffer(BufferSize * 2);

		Log("Initialising IPC based driver...");
		// IPC::SendSimpleMessage(0, IPC::MessageTypes::RequestServiceInitialise, 0, 2, 0, 0);
	}

	void PS2Keyboard::HandleKeypress()
	{
		// wait for ready.
		while(!(IOPort::ReadByte(PS2::CommandPort) & (1 << 0)));

		PS2::Device1Buffer = IOPort::ReadByte(PS2::DataPort);

		if(PS2::Device1Buffer == 0xFA || PS2::Device1Buffer == 0xFE)
			return;

		// this->ByteBuffer->Write(PS2::Device1Buffer);
		this->ByteBuffer->Write(&PS2::Device1Buffer, 1);

		if(PS2::Device1Buffer != 0xE1 && PS2::Device1Buffer != 0xF0 && PS2::Device1Buffer != 0xE0)
			this->DecodeScancode();
	}

	uint8_t PS2Keyboard::Translate(uint8_t sc)
	{
		using namespace Kernel::HardwareAbstraction::Devices::KeyboardTranslations;
		uint64_t USBHIDIndex = ScanCode2_US_PS2_HID[sc] + 3;

		uint64_t findex = USBHIDIndex * 4 + 1;

		if(Shifting || (CapsLock && USB_HID[findex] >= 'a' && USB_HID[findex] <= 'z'))
			findex++;

		return (uint8_t) USB_HID[findex];
	}

	uint8_t PS2Keyboard::TranslateE0(uint8_t sc)
	{
		using namespace Kernel::HardwareAbstraction::Devices::KeyboardTranslations;
		return (uint8_t) ScanCode2_US_E0_HID[sc];
	}

	void PS2Keyboard::DecodeScancode()
	{
		uint8_t buf = 0;
		this->ByteBuffer->Read(&buf, 1);

		bool IsE0 = false, IsF0 = false;

		if(buf == 0xE0)
		{
			IsE0 = true;
			this->ByteBuffer->Read(&buf, 1);

			if(buf == 0xF0)
			{
				this->ByteBuffer->Read(&buf, 1);
				IsF0 = true;
			}
		}
		else if(buf == 0xF0)
		{
			IsF0 = true;
			this->ByteBuffer->Read(&buf, 1);
		}

		if(!IsF0 && !IsE0 && (buf == 0x59 || buf == 0x12))
		{
			Shifting = true;
			return;
		}

		if(IsF0 && !IsE0 && (buf == 0x59 || buf == 0x12))
		{
			Shifting = false;
			return;
		}


		if(!IsF0 && !IsE0 && buf == 0x58)
		{
			CapsLock = !CapsLock;

			// send LED state
			while(IOPort::ReadByte(PS2::CommandPort) & (1 << 1));

			IOPort::WriteByte(PS2::DataPort, CapsLock & (1 << 2));
			return;
		}

		if(!IsF0 && !IsE0)
		{
			uint8_t x = this->Translate(PS2::Device1Buffer);
			IO::Manager::Write(&x, 1);
			// this->Buffer->Write(&x, 1);
			// IPC::SendSimpleMessage(0, IPC::MessageTypes::ServiceData, 2, x, 0, 0);
		}
		else if(!IsF0 && IsE0)
		{
			// because E0 codes have no printable thingy, we just forward the actual HID code.
			// ORed with 0x80, to avoid the ASCII printable range.
			// IPC::SendSimpleMessage(0, IPC::MessageTypes::ServiceData, 2, this->TranslateE0(PS2::Device1Buffer) | 0x80, 0, 0);
			uint8_t x = this->TranslateE0(PS2::Device1Buffer) | 0x80;
			IO::Manager::Write(&x, 1);
			// this->Buffer->Write(&x, 1);
		}
	}

	uint8_t PS2Keyboard::ReadBuffer()
	{
		uint8_t ret = 0;
		this->Buffer->Read(&ret, 1);
		return ret;
	}

	bool PS2Keyboard::ItemsInBuffer()
	{
		return this->Buffer->ByteCount() > 0;
	}
}
}
}
