// IPC.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under Creative Commons Attribution ShareAlike 3.0 Unported.

#include <Kernel.hpp>
#include <HardwareAbstraction.hpp>
#include <HardwareAbstraction/BinaryFormats/ELF.hpp>

#include <Memory.hpp>
#include <rdestl/rdestl.h>

using namespace Library;
using namespace Kernel::HardwareAbstraction::MemoryManager;

namespace Kernel {
namespace HardwareAbstraction {
namespace LoadBinary
{
	ELFExecutable::ELFExecutable(uint8_t* buf)
	{
		this->buffer = buf;
	}


	uint64_t ELFExecutable::GetEntryPoint()
	{
		ELF64FileHeader_type* FileHeader = (ELF64FileHeader_type*) this->buffer;
		assert(FileHeader->ElfIdentification[EI_MAGIC0] == ELF_MAGIC0);
		assert(FileHeader->ElfIdentification[EI_MAGIC1] == ELF_MAGIC1);
		assert(FileHeader->ElfIdentification[EI_MAGIC2] == ELF_MAGIC2);
		assert(FileHeader->ElfIdentification[EI_MAGIC3] == ELF_MAGIC3);
		assert(FileHeader->ElfIdentification[EI_CLASS] == ElfClass64Bit);
		assert(FileHeader->ElfIdentification[EI_DATA] == ElfDataLittleEndian);
		assert(FileHeader->ElfType == ElfTypeExecutable);

		return FileHeader->ElfEntry;
	}




	void ELFExecutable::Load(Multitasking::Process* proc)
	{
		ELF64FileHeader_type* FileHeader = (ELF64FileHeader_type*) this->buffer;
		assert(FileHeader->ElfIdentification[EI_MAGIC0] == ELF_MAGIC0);
		assert(FileHeader->ElfIdentification[EI_MAGIC1] == ELF_MAGIC1);
		assert(FileHeader->ElfIdentification[EI_MAGIC2] == ELF_MAGIC2);
		assert(FileHeader->ElfIdentification[EI_MAGIC3] == ELF_MAGIC3);
		assert(FileHeader->ElfIdentification[EI_CLASS] == ElfClass64Bit);
		assert(FileHeader->ElfIdentification[EI_DATA] == ElfDataLittleEndian);
		assert(FileHeader->ElfType == ElfTypeExecutable);

		rde::hash_map<uint64_t, bool>* allocatedpgs = new rde::hash_map<uint64_t, bool>();

		// temporarily map the process's pml4.
		for(uint64_t k = 0; k < FileHeader->ElfProgramHeaderEntries; k++)
		{
			ELF64ProgramHeader_type* ProgramHeader = (ELF64ProgramHeader_type*)(this->buffer + FileHeader->ElfProgramHeaderOffset + (k * FileHeader->ElfProgramHeaderEntrySize));

			if(ProgramHeader->ProgramType == ProgramTypeNull || ProgramHeader->ProgramMemorySize == 0 || ProgramHeader->ProgramVirtualAddress == 0)
				continue;

			for(uint64_t m = 0; m < (ProgramHeader->ProgramMemorySize + 0x1000) / 0x1000; m++)
			{
				if(allocatedpgs->size() > 0 && allocatedpgs->find(ProgramHeader->ProgramVirtualAddress + (m * 0x1000)) != allocatedpgs->end())
					continue;	// we've already mapped this address to a page, continue.

				(*allocatedpgs)[ProgramHeader->ProgramVirtualAddress + (m * 0x1000)] = true;

				uint64_t t = Physical::AllocatePage();
				Virtual::MapAddress(ProgramHeader->ProgramVirtualAddress + (m * 0x1000), t, 0x07, (Virtual::PageMapStructure*) proc->CR3);

				// map it to this address space for a bit.
				Virtual::MapAddress(TemporaryVirtualMapping + ProgramHeader->ProgramVirtualAddress + (m * 0x1000), t, 0x07, true);
			}

			Memory::Copy((void*)(TemporaryVirtualMapping + ProgramHeader->ProgramVirtualAddress), (void*)(this->buffer + ProgramHeader->ProgramOffset), ProgramHeader->ProgramFileSize);

			if(ProgramHeader->ProgramMemorySize > ProgramHeader->ProgramFileSize)
				Memory::Set((void*)(this->buffer + ProgramHeader->ProgramFileSize), 0x00, ProgramHeader->ProgramMemorySize - ProgramHeader->ProgramFileSize);

			for(uint64_t m = 0; m < (ProgramHeader->ProgramMemorySize + 0x1000) / 0x1000; m++)
			{
				// unmap what we did just now.
				Virtual::UnMapAddress(TemporaryVirtualMapping + ProgramHeader->ProgramVirtualAddress + (m * 0x1000), true);
			}
		}

		delete allocatedpgs;
	}

	ELFExecutable::~ELFExecutable()
	{
	}
}
}
}
