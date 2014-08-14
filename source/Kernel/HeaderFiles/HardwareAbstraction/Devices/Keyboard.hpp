// Keyboard.hpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under Creative Commons Attribution ShareAlike 3.0 Unported.


#include <stdint.h>
#include <CircularBuffer.hpp>
#pragma once

namespace Kernel {
namespace HardwareAbstraction {
namespace Devices
{
	class Keyboard
	{
		public:
			Keyboard(Keyboard* dev);
			virtual ~Keyboard();
			void Enable();
			void Disable();

			virtual void HandleKeypress();
			virtual uint8_t ReadBuffer();
			virtual bool ItemsInBuffer();

		protected:
			Keyboard* ActualDevice;
			bool Enabled;
	};

	class PS2Keyboard : public Keyboard
	{
		public:
			PS2Keyboard();

			void HandleKeypress();
			uint8_t ReadBuffer();
			void DecodeScancode();
			uint8_t Translate(uint8_t sc);
			bool ItemsInBuffer();

		private:
			// Library::CircularBuffer<uint8_t>* Buffer;
			// Library::CircularBuffer<uint8_t>* ByteBuffer;

			Library::CircularMemoryBuffer* Buffer;
			Library::CircularMemoryBuffer* ByteBuffer;
			uint8_t TranslateE0(uint8_t sc);
	};
}
}
}
