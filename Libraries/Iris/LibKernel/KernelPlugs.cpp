// userspace/PrintCharK.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <stdint.h>


namespace Kernel
{
	namespace Console
	{
		void PrintChar(uint8_t c);
		void SetColour(uint32_t Colour);
	}
	namespace HardwareAbstraction
	{
		namespace MemoryManager
		{
			namespace KernelHeap
			{
				uint64_t QuerySize(void* Address);
			}
		}
	}
}

namespace Library
{
	namespace StandardIO
	{
		void PrintChar(uint8_t c, void (*pf)(uint8_t))
		{
			if(pf)
				pf(c);

			else
				Kernel::Console::PrintChar(c);
		}
	}

	namespace Heap
	{
		uint64_t QuerySize(void* Address)
		{
			return Kernel::HardwareAbstraction::MemoryManager::KernelHeap::QuerySize(Address);
		}
	}

	namespace SystemCall
	{
		void SetTextColour(uint32_t Colour)
		{
			Kernel::Console::SetColour(Colour);
		}
	}
}






