// main.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <stdint.h>
#include "FxLoader.hpp"
#include "Elf.hpp"

extern "C" uint64_t StartBSS;
extern "C" uint64_t EndBSS;

uint64_t KernelEnd = 0x00400000;

// set in start.s
static uint64_t CurrentCR3 = 0x3000;

extern "C" uint64_t LoaderBootstrap(uint32_t MultibootMagic, uint32_t MBTAddr)
{
	// clear the bss
	{
		uint64_t bs = (uint64_t) &StartBSS;
		uint64_t be = (uint64_t) &EndBSS;
		uint64_t sz = be - bs;

		memset((void*) bs, 0, sz);
	}

	Console::Initialise();

	Console::Print("[fx] loader is initialising...\n");
	assert(MultibootMagic == 0x2BADB002);

	Memory::MemoryMap_type MemoryMap;

	Console::Print("Multiboot struct at %x\n", MBTAddr);

	Memory::Initialise((Multiboot::Info_type*) (uint64_t) MBTAddr, &MemoryMap);
	Console::Print("Memory map initialised\n");

	uint64_t kstart = 0;
	uint64_t klength = 0;
	LoadKernelModule((Multiboot::Info_type*) (uint64_t) MBTAddr, &kstart, &klength);

	Memory::InitialiseVirt();
	Console::Print("Virtual memory online\n");

	Memory::InitialisePhys(&MemoryMap);
	Console::Print("Physical memory online\n");

	Console::Print("Loading kernel...\n");
	uint64_t entry = LoadKernelELF(kstart, klength);

	Console::Print("Entry point: %x\n", entry);

	// Console::ClearScreen(0);
	// let ASM do it.
	return entry;
}


void LoadKernelModule(Multiboot::Info_type* mbt, uint64_t* start, uint64_t* length)
{
	assert(mbt);
	assert(mbt->mods_count == 1 && "Expected only one module, the [mx] kernel!");

	Multiboot::Module_type* mod = (Multiboot::Module_type*) (uint64_t) mbt->mods_addr;
	Console::Print("mods_addr: %x\n", mbt->mods_addr);
	Console::Print("[mx] kernel loaded at %x, ends at %x, %d bytes long\n", mod->ModuleStart, mod->ModuleEnd, mod->ModuleEnd - mod->ModuleStart);

	KernelEnd = Memory::PageAlignUp(mod->ModuleEnd);

	*start = mod->ModuleStart;
	*length = mod->ModuleEnd - mod->ModuleStart;
}




uint64_t LoadKernelELF(uint64_t start, uint64_t length)
{
	(void) length;

	ELF64FileHeader_type* FileHeader = (ELF64FileHeader_type*) start;
	assert(FileHeader->ElfIdentification[EI_MAGIC0] == ELF_MAGIC0);
	assert(FileHeader->ElfIdentification[EI_MAGIC1] == ELF_MAGIC1);
	assert(FileHeader->ElfIdentification[EI_MAGIC2] == ELF_MAGIC2);
	assert(FileHeader->ElfIdentification[EI_MAGIC3] == ELF_MAGIC3);
	assert(FileHeader->ElfIdentification[EI_CLASS] == ElfClass64Bit);
	assert(FileHeader->ElfIdentification[EI_DATA] == ElfDataLittleEndian);
	assert(FileHeader->ElfType == ElfTypeExecutable);

	for(uint64_t k = 0; k < FileHeader->ElfProgramHeaderEntries; k++)
	{
		Console::Print("Loading segment %d\n", k);
		ELF64ProgramHeader_type* ProgramHeader = (ELF64ProgramHeader_type*) (start + FileHeader->ElfProgramHeaderOffset + (k * FileHeader->ElfProgramHeaderEntrySize));

		if(ProgramHeader->ProgramType == ProgramTypeNull || ProgramHeader->ProgramMemorySize == 0 || ProgramHeader->ProgramVirtualAddress == 0)
			continue;

		for(uint64_t m = 0; m < (ProgramHeader->ProgramMemorySize + 0x1000) / 0x1000; m++)
		{
			uint64_t actualvirt = (ProgramHeader->ProgramVirtualAddress + (m * 0x1000)) & ~0xFFF;

			uint64_t t = Memory::AllocateFromReserved();
			Memory::MapAddress(actualvirt, t, 0x03);
		}


		memcpy((void*) (ProgramHeader->ProgramVirtualAddress), (void*) (start + ProgramHeader->ProgramOffset), ProgramHeader->ProgramFileSize);

		if(ProgramHeader->ProgramMemorySize > ProgramHeader->ProgramFileSize)
		{
			memset((void*) (ProgramHeader->ProgramVirtualAddress + ProgramHeader->ProgramFileSize), 0,
				ProgramHeader->ProgramMemorySize - ProgramHeader->ProgramFileSize);
		}
	}

	return FileHeader->ElfEntry;
}


namespace Memory
{
	uint64_t GetCurrentCR3()
	{
		return CurrentCR3;
	}
}







