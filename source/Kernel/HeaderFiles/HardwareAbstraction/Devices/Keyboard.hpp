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
			Keyboard() { }
			virtual ~Keyboard();

			virtual void HandleKeypress();
			virtual uint8_t ReadBuffer();
			virtual bool ItemsInBuffer();

		protected:
			bool Enabled;
	};

	class PS2Keyboard : public Keyboard
	{
		public:
			PS2Keyboard();

			void HandleKeypress() override;
			uint8_t ReadBuffer() override;
			bool ItemsInBuffer() override;

			void DecodeScancode();
			uint8_t Translate(uint8_t sc);

		private:
			Library::CircularMemoryBuffer* Buffer;
			Library::CircularMemoryBuffer* ByteBuffer;
			uint8_t TranslateE0(uint8_t sc);
	};
}
}
}
