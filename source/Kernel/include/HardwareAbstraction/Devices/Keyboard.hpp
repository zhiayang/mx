// Keyboard.hpp
// Copyright (c) 2014 - 2016, zhiayang@gmail.com
// Licensed under Creative Commons Attribution ShareAlike 3.0 Unported.


#include <stdint.h>
#include <CircularBuffer.hpp>
#include <HardwareAbstraction/DeviceManager.hpp>

#pragma once

namespace Kernel {
namespace HardwareAbstraction {
namespace Devices
{
	enum class KeyboardInterface
	{
		PS2,
		USB
	};

	class Keyboard : public DeviceManager::Device
	{
		public:
			Keyboard() { }
			virtual ~Keyboard();

			virtual void HandleKeypress();

			KeyboardInterface type;

		protected:
			bool Enabled;
	};

	class PS2Keyboard : public Keyboard
	{
		public:
			PS2Keyboard();

			virtual void HandleKeypress() override;

			void DecodeKey();
			uint8_t Translate(uint8_t sc);

		private:
			Library::CircularMemoryBuffer ByteBuffer;
			uint8_t TranslateE0(uint8_t sc);
	};
}
}
}
