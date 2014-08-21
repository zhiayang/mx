// InterruptPlugs.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <HardwareAbstraction/Interrupts.hpp>

namespace Kernel {
namespace HardwareAbstraction {
namespace SystemCalls
{
	extern "C" void Syscall_InstallIRQHandler(uint64_t handler, uint64_t pointer)
	{
		Kernel::HardwareAbstraction::Interrupts::InstallIRQHandler(handler, (void(*)(Interrupts::RegisterStruct_type*)) pointer);
	}

	extern "C" void Syscall_InstallIRQHandlerNoRegs(uint64_t handler, uint64_t pointer)
	{
		Kernel::HardwareAbstraction::Interrupts::InstallIRQHandler(handler, (void(*)()) pointer);
	}
}
}
}
