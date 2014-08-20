// MemoryMap.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#include <Kernel.hpp>
#include <Console.hpp>
#include <Memory.hpp>
#include <StandardIO.hpp>
#include <Colours.hpp>

using namespace Kernel;
using namespace Library::StandardIO;
using namespace HardwareAbstraction::MemoryManager::MemoryMap;

extern MemoryMap_type* K_MemoryMap;


namespace Kernel {
namespace HardwareAbstraction {
namespace MemoryManager {
namespace MemoryMap
{

	// An array containing a list of memory types.
	static const char* K_MemoryTypes[6] =
	{
		"[Invalid Type]",
		"[Available]",
		"[Reserved]",
		"[ACPI Reclaimable]",
		"[ACPI NVM]",
		"[Bad Memory]"
	};


	bool IsMemoryValid(uint64_t Address)
	{
		for(int i = 0; i < K_MemoryMap->NumberOfEntries; i++)
		{
			if(Address >= K_MemoryMap->Entries[i].BaseAddress && Address < (K_MemoryMap->Entries[i].BaseAddress + K_MemoryMap->Entries[i].Length))
			{
				if(K_MemoryMap->Entries[i].MemoryType == G_MemoryTypeAvailable || K_MemoryMap->Entries[i].MemoryType == G_MemoryACPI)
				{
					return true;
				}
				else
				{
					return false;
				}
			}
		}
		return false;
	}

	void GetGlobalMemoryMap(MemoryMap_type *MemoryMap)
	{
		Library::Memory::Copy((void*) MemoryMap, (void*) K_MemoryMap, K_MemoryMap->SizeOfThisStructure);
	}

	uint64_t GetTotalSystemMemory()
	{
		return K_SystemMemoryInBytes;
	}

	uint64_t CheckForMoreAvailableMemory(uint64_t CurrentAddr)
	{
		for(int i = 0; i < K_MemoryMap->NumberOfEntries; i++)
		{
			if(CurrentAddr >= K_MemoryMap->Entries[i].BaseAddress && CurrentAddr < K_MemoryMap->Entries[i].BaseAddress + K_MemoryMap->Entries[i].Length)
			{
				for(int f = i; f < K_MemoryMap->NumberOfEntries; f++)
				{
					if(K_MemoryMap->Entries[f].MemoryType == G_MemoryTypeAvailable)
					{
						return K_MemoryMap->Entries[f].BaseAddress;
					}
				}
			}
		}
		return 0;
	}

	void PrintKernelMemoryMap()
	{
		// We do this so we can verify that our kernel memory map is good after copying it.
		uint16_t i = 0;
		while(i < K_MemoryMap->NumberOfEntries)
		{
			PrintFormatted("%w\t=>%w %#16.4x%r - %w%#-16x %r:%w Type %d ", Library::Colours::White, Library::Colours::Yellow, K_MemoryMap->Entries[i].BaseAddress,
				Library::Colours::Green, K_MemoryMap->Entries[i].BaseAddress + K_MemoryMap->Entries[i].Length, Library::Colours::Violet, K_MemoryMap->Entries[i].MemoryType);


			switch(K_MemoryMap->Entries[i].MemoryType)
			{
				case 1:
					Console::SetColour(Library::Colours::Green);
					break;

				case 2:
					Console::SetColour(Library::Colours::Red);
					break;

				case 3:
					Console::SetColour(Library::Colours::Yellow);
					break;

				case 4:
					Console::SetColour(Library::Colours::Orange);
					break;

				case 5:
					Console::SetColour(Library::Colours::Red);
					break;
			}
			PrintFormatted(" %s\n", K_MemoryTypes[K_MemoryMap->Entries[i].MemoryType]);
			Console::SetColour(Library::Colours::White);

			i++;
		}
	}

	void Initialise(Multiboot::Info_type* MBTStruct)
	{
		uint64_t TotalMapAddressess = (uint64_t) MBTStruct->mmap_addr;
		uint64_t TotalMapLength = MBTStruct->mmap_length;

		K_MemoryMap->SizeOfThisStructure = sizeof(uint32_t) + sizeof(uint16_t);
		K_MemoryMap->NumberOfEntries = 0;


		// Check if the fields are valid:
		if(!(MBTStruct->flags & (1 << 6)))
		{
			PrintFormatted("FLAGS: %d", MBTStruct->flags);
			HALT("No Multiboot Memory Map!");
		}

		Multiboot::MemoryMap_type* mmap = (Multiboot::MemoryMap_type*) TotalMapAddressess;
		while((uint64_t) mmap < (uint64_t)(TotalMapAddressess + TotalMapLength))
		{

			(K_MemoryMap->Entries[K_MemoryMap->NumberOfEntries]).BaseAddress = (uint64_t) mmap->BaseAddr_Low | (uint64_t)(mmap->BaseAddr_High) << 32;
			(K_MemoryMap->Entries[K_MemoryMap->NumberOfEntries]).Length = (uint64_t)(mmap->Length_Low | (uint64_t)(mmap->Length_High) << 32);
			(K_MemoryMap->Entries[K_MemoryMap->NumberOfEntries]).MemoryType = (uint8_t) mmap->Type;
			K_MemoryMap->SizeOfThisStructure += sizeof(MemoryMapEntry_type);



			switch(K_MemoryMap->Entries[K_MemoryMap->NumberOfEntries].MemoryType)
			{
				case 1:
				case 3:
				K_SystemMemoryInBytes += (K_MemoryMap->Entries[K_MemoryMap->NumberOfEntries].Length);
				break;
			}


			K_MemoryMap->NumberOfEntries++;
			mmap = (Multiboot::MemoryMap_type*)((uint64_t) mmap + mmap->Size + sizeof(uint32_t));
		}
	}

}
}
}
}







