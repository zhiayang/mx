// Console.hpp
// Copyright (c) 2013 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#pragma once
#include <stdint.h>
#include <rdestl/vector.h>

namespace Kernel
{
	namespace Console
	{
		void Initialise();
		bool IsInitialised();
		void PrintChar(uint8_t c);
		void ClearScreen();
		void Scroll();
		void ScrollDown(uint64_t lines);
		void Backspace(uint64_t characters);
		void SetColour(uint32_t Colour);
		uint32_t GetColour();
		void MoveCursor(uint16_t x, uint16_t y);
		uint16_t GetCharsPerLine();
		uint16_t GetCharsPerColumn();
		uint16_t GetCharsPerPage();
		uint16_t GetCursorX();
		uint16_t GetCursorY();
	}

	namespace TTY
	{
		class TTYObject
		{
			public:
				TTYObject(uint8_t bufmode, uint64_t (*)(TTYObject*, uint8_t*, uint64_t), uint64_t (*)(TTYObject*, uint8_t*, uint64_t), void (*)(TTYObject*));
				uint64_t (*in)(TTYObject*, uint8_t*, uint64_t);
				uint64_t (*out)(TTYObject*, uint8_t*, uint64_t);
				void (*flush)(TTYObject*);

				bool echomode;
				uint8_t BufferMode;
				size_t buffersize;
				rde::vector<uint8_t> buffer;
				rde::vector<uint8_t> internalbuffer;
		};

		void Initialise();
		void FlushTTY(long ttyid);
		uint64_t WriteTTY(long ttyid, uint8_t* data, uint64_t length);
		uint64_t ReadTTY(long ttyid, uint8_t* data, uint64_t length);
		uint64_t ConfigureTTY(uint64_t configkey, void* data);
	}
}






