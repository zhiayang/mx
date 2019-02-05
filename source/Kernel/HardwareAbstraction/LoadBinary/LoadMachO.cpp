// LoadMachO.cpp
// Copyright (c) 2014 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <StandardIO.hpp>

using namespace Library;
using namespace Kernel::HardwareAbstraction::MemoryManager;

namespace Kernel {
namespace HardwareAbstraction {
namespace LoadBinary
{
	// MachOExecutable::MachOExecutable(const char* name, uint8_t* buffer, uint64_t bs) : GenericExecutable(ExecutableType::MachO)
	// {
	// 	this->buf = buffer;
	// 	this->size = bs;

	// 	// display 28 bytes of the header.
	// 	// for(int i = 0; i < 28; i++)
	// 	{
	// 		StandardIO::PrintFmt("%#08x ", ((uint32_t*) this->buf)[0]);
	// 	}
	// }

	// void MachOExecutable::Execute()
	// {

	// }
}
}
}
