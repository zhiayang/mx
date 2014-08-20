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



	uint8_t* LoadFileToMemory(const char* filename);

	class GenericExecutable
	{
		public:
			GenericExecutable(ExecutableType type);
			GenericExecutable(const char* pn, uint8_t* data);
			virtual ~GenericExecutable();
			void AutomaticLoadExecutable();

			void SetPriority(uint8_t Priority);
			void SetApplicationType(Multitasking::ThreadType type);
			void Execute();

			ExecutableType Type;

			Multitasking::Process* proc;
			const char* procname;
			uint8_t* buf;
	};


	class ELFExecutable : public GenericExecutable
	{
		public:
			ELFExecutable(const char* name, uint8_t* databuffer);
			~ELFExecutable();
	};

	class MachOExecutable : public GenericExecutable
	{
		public:
			MachOExecutable(const char* pathname, uint8_t* databuffer);
			~MachOExecutable();

			void SetPriority(uint8_t Priority);
			void Execute();
	};
}
}
}
