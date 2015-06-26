// PS2Keyboard.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <Console.hpp>
#include <HardwareAbstraction/Devices/IOPort.hpp>

#include "Translation.hpp"

namespace Kernel {
namespace HardwareAbstraction {
namespace Devices
{
	#define BUFFER_SIZE	256

	#define IS_SHIFT_KEY(x)		((x) == 0x59 || (x) == 0x12)
	#define IS_CAPS_KEY(x)		((x) == 0x58)

	static bool isShifting = false;
	static bool isCapsLock = false;

	PS2Keyboard::PS2Keyboard() : ByteBuffer(BUFFER_SIZE)
	{
		this->type = KeyboardInterface::PS2;

		// IOPort::ReadByte(PS2::DataPort);
	}

	void PS2Keyboard::HandleKeypress()
	{
		// wait for ready
		while(!(IOPort::ReadByte(PS2::CommandPort) & (1 << 0)))
			;

		PS2::Device1Buffer = IOPort::ReadByte(PS2::DataPort);
		if(PS2::Device1Buffer == 0xFA || PS2::Device1Buffer == 0xFE)
			return;

		this->ByteBuffer.Write(&PS2::Device1Buffer, 1);

		if(PS2::Device1Buffer != 0xE1 && PS2::Device1Buffer != 0xF0 && PS2::Device1Buffer != 0xE0)
		{
			this->DecodeScancode();
		}
	}


	uint8_t PS2Keyboard::Translate(uint8_t scancode)
	{
		using namespace KeyboardTranslations;
		uint64_t USBHIDIndex = ScanCode2_US_PS2_HID[scancode] + 3;

		uint64_t findex = USBHIDIndex * 4 + 1;

		if(isShifting || (isCapsLock && USB_HID[findex] >= 'a' && USB_HID[findex] <= 'z'))
		{
			findex++;
		}


		return (uint8_t) USB_HID[findex];
	}


	uint8_t PS2Keyboard::TranslateE0(uint8_t scancode)
	{
		using namespace KeyboardTranslations;
		return (uint8_t) ScanCode2_US_E0_HID[scancode];
	}

	void PS2Keyboard::DecodeScancode()
	{
		uint8_t buf = 0;
		this->ByteBuffer.Read(&buf, 1);

		bool isE0 = false;
		bool isF0 = false;


		if(buf == 0xE0)
		{
			isE0 = true;
			this->ByteBuffer.Read(&buf, 1);

			if(buf == 0xF0)
			{
				isF0 = true;
				this->ByteBuffer.Read(&buf, 1);
			}
		}
		else if(buf == 0xF0)
		{
			isF0 = true;
			this->ByteBuffer.Read(&buf, 1);
		}



		// left and right shift
		if(!isF0 && !isE0 && IS_SHIFT_KEY(buf))
		{
			isShifting = true;
		}
		else if(isF0 && !isE0 && IS_SHIFT_KEY(buf))
		{
			isShifting = false;
		}
		else if(!isF0 && !isE0 && IS_CAPS_KEY(buf))
		{
			isCapsLock = !isCapsLock;

			// change LED.
			// wait for ready...
			while(IOPort::ReadByte(PS2::CommandPort) & (1 << 1))
				;


			IOPort::WriteByte(PS2::DataPort, isCapsLock & (1 << 2));
		}
		else if(!isF0 && !isE0)
		{
			uint8_t x = this->Translate(PS2::Device1Buffer);
			TTY::WriteTTY(0, &x, 1);
		}
		else if(!isF0 && isE0)
		{
			// E0s can't be printed.
			// forward the HID code, ORed with 0x80 to prevent conflict
			// with printable ASCII.

			uint8_t x = this->TranslateE0(PS2::Device1Buffer) | 0x80;
			TTY::WriteTTY(0, &x, 1);
		}
	}
}
}
}



























