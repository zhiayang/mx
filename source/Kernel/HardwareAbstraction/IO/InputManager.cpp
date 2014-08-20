// InputManager.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <CircularBuffer.hpp>
namespace Kernel {
namespace HardwareAbstraction {
namespace IO {
namespace Manager
{
	#define BUFFER_SIZE 1024
	static Library::CircularMemoryBuffer* buffer;

	void Write(void* data, size_t length)
	{
		if(!buffer)
			buffer = new Library::CircularMemoryBuffer(BUFFER_SIZE);

		buffer->Write((uint8_t*) data, length);
	}

	size_t Read(void* outbuf, size_t length)
	{
		size_t read = buffer->ByteCount();
		buffer->Read((uint8_t*) outbuf, length);
		return read;
	}
}
}
}
}
