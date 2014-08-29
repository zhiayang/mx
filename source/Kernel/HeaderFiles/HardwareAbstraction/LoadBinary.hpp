// IPC.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under Creative Commons Attribution ShareAlike 3.0 Unported.

#include <stdint.h>
#include <Kernel.hpp>
#pragma once

namespace Kernel {
namespace HardwareAbstraction {
namespace LoadBinary
{
	enum class ExecutableType
	{
		ELF,
		MachO,
		RawBinary
	};

	Multitasking::Process* Load(const char* path, const char* procname, void* a1 = 0, void* a2 = 0, void* a3 = 0, void* a4 = 0, void* a5 = 0, void* a6 = 0);

	class ExecutableFormat
	{
		public:
			virtual ~ExecutableFormat() { }
			virtual uint64_t GetEntryPoint();
			virtual void Load(Multitasking::Process* proc);

		protected:
			uint8_t* buffer;
	};

	class ELFExecutable : public ExecutableFormat
	{
		public:
			ELFExecutable(uint8_t* buf);
			~ELFExecutable();
			virtual uint64_t GetEntryPoint() override;
			virtual void Load(Multitasking::Process* proc) override;
	};
}
}
}
