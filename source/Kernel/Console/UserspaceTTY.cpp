// UserspaceTTY.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <Console.hpp>
#include <StandardIO.hpp>
#include <stdio.h>

namespace Kernel {
namespace TTY
{
	// we need to manage stdin and stdout in terms of buffering.
	// stdout is usually line buffered.
	// we keep an internal buffer of 4096 (default) bytes
	// depending on buffer mode:
	// if we're line buffered, once we read a newline we flush the buffer (send to userspace)
	// if we're block buffered, if the data we get cannot fit, we flush, read, repeat until all data is gotten.

	#define BLOCKING_READ	0x80

	static rde::hash_map<long, TTYObject*>* ttys;

	TTYObject::TTYObject(uint8_t bufmode, uint64_t (*read)(TTYObject*, uint8_t*, uint64_t), uint64_t (*write)(TTYObject*, uint8_t*, uint64_t), void (*flsh)(TTYObject*))
	{
		// this->buffer = new rde::vector<uint8_t>();
		// this->internalbuffer = new rde::vector<uint8_t>();
		this->BufferMode = bufmode;
		this->in = read;
		this->out = write;
		this->flush = flsh;
		this->buffersize = BUFSIZ;
		this->echomode = false;
	}

	// sometimes i like underscores.
	// mostly, no.
	void stdout_flush(TTYObject*);
	void stdin_flush(TTYObject*);
	void noop_flush(TTYObject*);
	uint64_t noopfunc(TTYObject*, uint8_t*, uint64_t);
	uint64_t stdin_read(TTYObject*, uint8_t*, uint64_t);
	uint64_t stdin_write(TTYObject*, uint8_t*, uint64_t);
	uint64_t stdout_write(TTYObject*, uint8_t*, uint64_t);

	uint64_t noopfunc(TTYObject*, uint8_t*, uint64_t)
	{
		return 0;
	}

	uint64_t stdin_read(TTYObject* tty, uint8_t* buf, uint64_t length)
	{
		assert(tty);
		assert(ttys->find(1)->second);
		assert(buf);

		// flush stdout first.
		stdout_flush(ttys->find(1)->second);

		do
		{
			asm volatile("pause");
			if(tty->buffer.size() == 0)
				continue;

			Memory::Copy(buf, tty->buffer.data(), __min(tty->buffer.size(), length));

			tty->buffer.erase(tty->buffer.begin(), tty->buffer.begin() + __min(tty->buffer.size(), length));
			return __min(tty->buffer.size(), length);

		} while(tty->buffer.size() == 0 && tty->BufferMode & BLOCKING_READ);

		return 0;
	}

	uint64_t stdin_write(TTYObject* tty, uint8_t* buf, uint64_t length)
	{
		assert(buf);
		assert(tty);

		if(tty->BufferMode & _IOLBF)
		{
			uint64_t oldsize = tty->internalbuffer.size();
			for(uint64_t i = 0; i < length; i++)
				tty->internalbuffer.push_back(buf[i]);

			uint64_t i = 0;
			for(i = 0; i < length; i++)
			{
				if(buf[i] == '\n')
					break;
			}

			if(i < length)
			{
				assert(buf[i] == '\n');

				// this '\n' should be at buffer->oldsize + i.
				assert(tty->internalbuffer[oldsize + i] == '\n');

				uint64_t j = 0;
				for(j = 0; j < oldsize + i; j++)
					tty->buffer.push_back(tty->internalbuffer[j]);

				tty->internalbuffer.erase(tty->internalbuffer.begin(), tty->internalbuffer.begin() + j);
			}
			else if(tty->internalbuffer.size() >= tty->buffersize)
			{
				for(auto d : tty->internalbuffer)
					tty->buffer.push_back(d);

				tty->internalbuffer.clear();
			}
		}
		else if(tty->BufferMode & _IOFBF)
		{
			// fully buffered. write everything to the internal buffer, unless it's full.
			for(uint64_t i = 0; i < length; i++)
				tty->internalbuffer.push_back(buf[i]);

			if(tty->buffer.size() >= tty->buffersize)
			{
				for(auto d : tty->internalbuffer)
					tty->buffer.push_back(d);

				tty->internalbuffer.clear();
			}
		}
		else
		{
			for(uint64_t i = 0; i < length; i++)
				tty->buffer.push_back(buf[i]);
		}

		if(tty->echomode)
		{
			stdout_write(ttys->find(1)->second, buf, length);
			stdout_flush(ttys->find(1)->second);
		}

		return length;
	}


	uint64_t stdout_write(TTYObject* tty, uint8_t* buf, uint64_t length)
	{
		assert(tty);
		if(tty->BufferMode & _IOLBF)
		{
			// todo: might want an 'in-house' buffer that handles newline flushing etc. better.
			uint8_t* tmp = buf;
			uint64_t total = 0;
			for(uint64_t i = 0; i < length; i++)
			{
				if(*tmp == '\n')
				{
					// flush
					size_t bc = tty->buffer.size();
					uint8_t* tempout = new uint8_t[bc];
					Memory::Copy(tempout, tty->buffer.data(), bc);
					tty->buffer.clear();

					total += Library::StandardIO::PrintString((const char*) tempout, bc);
					total += Library::StandardIO::PrintString("\n", 1);

					delete[] tempout;
				}
				else
				{
					tty->buffer.push_back(*tmp);
					total++;
				}
				tmp++;
			}
		}
		else if(tty->BufferMode & _IOFBF)
		{
			for(uint64_t i = 0; i < length; i++)
				tty->buffer.push_back(buf[i]);

			if(tty->buffer.size() >= tty->buffersize)
			{
				Library::StandardIO::PrintString((const char*) tty->buffer.data(), tty->buffer.size());
				tty->buffer.clear();
			}
		}
		else
		{
			return Library::StandardIO::PrintString((const char*) buf, length);
		}

		return length;
	}

	void stdout_flush(TTYObject* tty)
	{
		assert(tty);

		uint64_t bc = tty->buffer.size();
		uint8_t* tempout = new uint8_t[bc];
		Memory::Copy(tempout, tty->buffer.data(), bc);
		tty->buffer.clear();

		Library::StandardIO::PrintString((const char*) tempout, bc);
		delete[] tempout;
	}

	void stdin_flush(TTYObject* tty)
	{
		assert(tty);

		// flush the internal buffer to the read buffer.
		for(auto d : tty->internalbuffer)
			tty->buffer.push_back(d);

		tty->internalbuffer.clear();
	}

	void noop_flush(TTYObject*)
	{
	}





	void Initialise()
	{
		ttys = new rde::hash_map<long, TTYObject*>();
		TTYObject* stdin	= new TTYObject(_IOLBF | BLOCKING_READ, stdin_read, stdin_write, stdin_flush);
		TTYObject* stdout	= new TTYObject(_IOLBF, noopfunc, stdout_write, stdout_flush);
		TTYObject* stderr	= new TTYObject(_IONBF, noopfunc, stdout_write, noop_flush);
		TTYObject* stdlog	= new TTYObject(_IONBF, noopfunc, noopfunc, noop_flush);

		stdin->echomode = true;

		ttys->insert(rde::pair<long, TTYObject*>(0, stdin));
		ttys->insert(rde::pair<long, TTYObject*>(1, stdout));
		ttys->insert(rde::pair<long, TTYObject*>(2, stderr));
		ttys->insert(rde::pair<long, TTYObject*>(-64, stdlog));
	}

	uint64_t WriteTTY(long ttyid, uint8_t* data, uint64_t length)
	{
		assert(ttys);
		if(ttys->find(ttyid) != ttys->end())
		{
			TTYObject* tty = ttys->find(ttyid)->second;
			return tty->out(tty, data, length);
		}
		else
		{
			Log(1, "Attempted to write to unknown tty with id %d, ignoring", ttyid);
		}

		return 0;
	}

	uint64_t ReadTTY(long ttyid, uint8_t* data, uint64_t length)
	{
		assert(ttys);
		if(ttys->find(ttyid) != ttys->end())
		{
			TTYObject* tty = ttys->find(ttyid)->second;
			return tty->in(tty, data, length);
		}
		else
		{
			Log(1, "Attempted to read from unknown tty with id %d, ignoring", ttyid);
		}

		return 0;
	}

	void FlushTTY(long ttyid)
	{
		assert(ttys);
		if(ttys->find(ttyid) != ttys->end())
		{
			TTYObject* tty = ttys->find(ttyid)->second;
			tty->flush(tty);
		}
		else
		{
			Log(1, "Attempted to read from unknown tty with id %d, ignoring", ttyid);
		}
	}
}
}
















