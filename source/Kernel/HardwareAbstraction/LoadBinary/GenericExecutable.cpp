// GenericExecutable.cpp
// Copyright (c) 2014 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <string.h>
#include <errno.h>
#include <HardwareAbstraction/LoadBinary.hpp>
#include <HardwareAbstraction/BinaryFormats/ELF.hpp>

using namespace Library;
using namespace Kernel::HardwareAbstraction::MemoryManager;

namespace Kernel {
namespace HardwareAbstraction {
namespace LoadBinary
{
	Multitasking::Process* Load(const char* path, const char* procname, void* a1, void* a2, void* a3, void* a4, void* a5, void* a6)
	{
		assert(path);
		assert(procname);

		using namespace Filesystems;
		auto fd = OpenFile(path, 0);
		if(fd < 0)
		{
			Multitasking::SetThreadErrno(EINVAL);
			return 0;
		}

		struct stat st;
		Stat(fd, &st);

		auto buf = new uint8_t[st.st_size];
		auto read = Read(fd, (void*) buf, st.st_size);
		assert(read == st.st_size);

		// Utilities::DumpBytes((uint64_t) buf, st.st_size);

		assert(buf);

		Multitasking::Process* proc = nullptr;

		// check for ELF
		if(buf[0] == ELF_MAGIC0 && buf[1] == ELF_MAGIC1 && buf[2] == ELF_MAGIC2 && buf[3] == ELF_MAGIC3)
		{
			ELFExecutable* elf = new ELFExecutable(buf);
			proc = Multitasking::CreateProcess(procname, FLAG_USERSPACE, elf->GetTLSSize(),(void(*)()) elf->GetEntryPoint(),
				1, a1, a2, a3, a4, a5, a6);

			elf->Load(proc);

			delete elf;
		}
		else
		{
			HALT("enotsup");
		}

		delete[] buf;
		return proc;
	}
}
}
}
