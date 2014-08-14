// InterruptPlugs.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>

namespace Kernel {
namespace HardwareAbstraction {
namespace SystemCalls
{
	extern "C" uint8_t Syscall_ReadKeyboardKey()
	{
		return Kernel::KernelKeyboard->ReadBuffer();
	}
}
}
}
