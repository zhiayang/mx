// IPC.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under Creative Commons Attribution ShareAlike 3.0 Unported.

#include <Kernel.hpp>
#include <HardwareAbstraction.hpp>
#include <HardwareAbstraction/BinaryFormats/ELF.hpp>

#include <Memory.hpp>

using namespace Library;
using namespace Kernel::HardwareAbstraction::MemoryManager;

namespace Kernel {
namespace HardwareAbstraction {
namespace LoadBinary
{
	ELFExecutable::ELFExecutable(const char* name, uint8_t* b) : GenericExecutable(ExecutableType::ELF)
	{
		this->buf = b;

		ELF64FileHeader_type* FileHeader = (ELF64FileHeader_type*) this->buf;
		assert(FileHeader->ElfIdentification[EI_MAGIC0] == ELF_MAGIC0);
		assert(FileHeader->ElfIdentification[EI_MAGIC1] == ELF_MAGIC1);
		assert(FileHeader->ElfIdentification[EI_MAGIC2] == ELF_MAGIC2);
		assert(FileHeader->ElfIdentification[EI_MAGIC3] == ELF_MAGIC3);
		assert(FileHeader->ElfIdentification[EI_CLASS] == ElfClass64Bit);
		assert(FileHeader->ElfIdentification[EI_DATA] == ElfDataLittleEndian);
		assert(FileHeader->ElfType == ElfTypeExecutable);

		void(*EntryPoint)() = (void(*)())(FileHeader->ElfEntry);
		this->proc = Multitasking::CreateProcess(name, 0x1, EntryPoint);


		// temporarily map the process's pml4.
		Virtual::MapAddress(this->proc->CR3, this->proc->CR3, 0x03, true);

		for(uint64_t k = 0; k < FileHeader->ElfProgramHeaderEntries; k++)
		{
			ELF64ProgramHeader_type* ProgramHeader = (ELF64ProgramHeader_type*)(this->buf + FileHeader->ElfProgramHeaderOffset + (k * FileHeader->ElfProgramHeaderEntrySize));

			if(ProgramHeader->ProgramType == ProgramTypeNull || ProgramHeader->ProgramMemorySize == 0 || ProgramHeader->ProgramVirtualAddress == 0)
				continue;

			for(uint64_t m = 0; m < (ProgramHeader->ProgramMemorySize + 0x1000) / 0x1000; m++)
			{
				uint64_t t = Physical::AllocatePage();
				Virtual::MapAddress(ProgramHeader->ProgramVirtualAddress + (m * 0x1000), t, 0x07, (Virtual::PageMapStructure*) proc->CR3);

				// map it to this address space for a bit.
				Virtual::MapAddress(TemporaryVirtualMapping + ProgramHeader->ProgramVirtualAddress + (m * 0x1000), t, 0x07, true);
			}

			Memory::Copy((void*)(TemporaryVirtualMapping + ProgramHeader->ProgramVirtualAddress), (void*)(this->buf + ProgramHeader->ProgramOffset), ProgramHeader->ProgramFileSize);

			if(ProgramHeader->ProgramMemorySize > ProgramHeader->ProgramFileSize)
				Memory::Set((void*)(this->buf + ProgramHeader->ProgramFileSize), 0x00, ProgramHeader->ProgramMemorySize - ProgramHeader->ProgramFileSize);

			for(uint64_t m = 0; m < (ProgramHeader->ProgramMemorySize + 0x1000) / 0x1000; m++)
			{
				// unmap what we did just now.
				Virtual::UnMapAddress(TemporaryVirtualMapping + ProgramHeader->ProgramVirtualAddress + (m * 0x1000), true);
			}
		}

		Virtual::UnMapAddress(this->proc->CR3, true);
	}

	ELFExecutable::~ELFExecutable()
	{
	}
}
}
}
