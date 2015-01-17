// CircularMemoryBuffer.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "HeaderFiles/CircularBuffer.hpp"
#include <math.h>
#include <stdlib.h>

namespace Library
{
	CircularMemoryBuffer::CircularMemoryBuffer(uint64_t s)
	{
		this->size = s;
		this->readp = 0;
		this->writep = 0;
		this->elements = 0;

		this->backingstore = new uint8_t[this->size];
	}

	CircularMemoryBuffer::CircularMemoryBuffer(CircularMemoryBuffer& other)
	{
		this->size = other.size;
		this->readp = other.readp;
		this->writep = other.writep;
		this->elements = other.elements;

		this->backingstore = new uint8_t[this->size];
		Memory::Copy(this->backingstore, other.backingstore, this->size);
	}

	CircularMemoryBuffer::~CircularMemoryBuffer()
	{
		delete[] this->backingstore;
	}

	void CircularMemoryBuffer::MoveWritePointer(uint64_t p)
	{
		p %= this->size;

		this->writep = p;
	}

	void CircularMemoryBuffer::MoveReadPointer(uint64_t p)
	{
		p %= this->size;

		this->readp = p;
	}

	uint64_t CircularMemoryBuffer::GetWritePointer()
	{
		return this->writep;
	}

	uint64_t CircularMemoryBuffer::TotalSize()
	{
		return this->size;
	}

	uint64_t CircularMemoryBuffer::ByteCount()
	{
		return this->elements;
	}

	uint64_t CircularMemoryBuffer::Read(uint8_t* buf, uint64_t bytes)
	{
		if(bytes > this->elements)
			bytes = this->elements;


		uint64_t firstpass = (this->size - this->readp);
		uint64_t remaining = bytes > firstpass ? bytes - firstpass : 0;

		uint64_t ret = this->PeekRead(buf, bytes);

		this->elements -= bytes;
		this->readp = (this->readp + __min(firstpass, bytes)) % this->size;
		if(bytes > firstpass)
			this->readp = (this->readp + remaining) % this->size;

		return ret;
	}

	uint64_t CircularMemoryBuffer::PeekWrite(uint8_t* buf, uint64_t bytes)
	{
		// TODO
		return this->Write(buf, bytes);
	}





	uint64_t CircularMemoryBuffer::PeekRead(uint8_t* buf, uint64_t bytes)
	{
		if(bytes > this->elements)
			bytes = this->elements;

		uintptr_t rdptr = this->readp;

		// allocate and copy.
		uint64_t firstpass = (this->size - rdptr);
		uint64_t remaining = bytes > firstpass ? bytes - firstpass : 0;

		Memory::Copy(buf, this->backingstore + rdptr, __min(firstpass, bytes));
		rdptr = (rdptr + __min(firstpass, bytes)) % this->size;
		if(bytes > firstpass)
		{
			Memory::Copy(buf + firstpass, this->backingstore + rdptr, remaining);
			rdptr = (rdptr + remaining) % this->size;
		}

		return bytes;
	}

	uint64_t CircularMemoryBuffer::Write(uint8_t* buf, uint64_t bytes)
	{
		if(bytes > this->size)
			bytes = this->size;

		uint64_t firstpass = __min(this->size - this->writep, bytes);
		uint64_t remaining = bytes > firstpass ? bytes - firstpass : 0;

		Memory::Copy(this->backingstore + this->writep, buf, firstpass);
		this->writep = (this->writep + firstpass) % this->size;
		if(bytes > firstpass)
		{
			Memory::Copy(this->backingstore + this->writep, buf + firstpass, remaining);
			this->writep = (this->writep + remaining) % this->size;
		}

		this->elements += bytes;
		return bytes;
	}
}



















