// GenericExecutable.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <string.h>
#include <HardwareAbstraction/LoadBinary.hpp>
#include <HardwareAbstraction/BinaryFormats/ELF.hpp>

using namespace Library;
using namespace Kernel::HardwareAbstraction::MemoryManager;

namespace Kernel {
namespace HardwareAbstraction {
namespace LoadBinary
{
	void GenericExecutable::SetApplicationType(Multitasking::ThreadType type)
	{
		for(uint64_t i = 0; i < this->proc->Threads->Size(); i++)
		{
			this->proc->Threads->Get(i)->Type = type;
		}
	}

	void GenericExecutable::SetPriority(uint8_t Priority)
	{
		for(uint64_t i = 0; i < this->proc->Threads->Size(); i++)
		{
			this->proc->Threads->Get(i)->Priority = Priority;
		}
	}

	void GenericExecutable::Execute()
	{
		Multitasking::AddToQueue(this->proc);
	}

	GenericExecutable::GenericExecutable(ExecutableType type)
	{
		this->Type = type;
	}

	GenericExecutable::GenericExecutable(const char* pn, uint8_t* data)
	{
		strcpy((char*) this->procname, pn);
		this->buf = data;
	}

	GenericExecutable::~GenericExecutable()
	{
	}

	void GenericExecutable::AutomaticLoadExecutable()
	{
		assert(this->buf);

		// check for ELF
		if(this->buf[0] == ELF_MAGIC0 && this->buf[1] == ELF_MAGIC1 && this->buf[2] == ELF_MAGIC2 && this->buf[3] == ELF_MAGIC3)
		{
			ELFExecutable* elf = new ELFExecutable(this->procname, this->buf);

			this->proc = elf->proc;
			delete elf;
		}
	}
}
}
}
