// ACPITables.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <StandardIO.hpp>
#include <Memory.hpp>
#include <List.hpp>

using namespace Library;
using namespace Library::StandardIO;

namespace Kernel {
namespace HardwareAbstraction {
namespace ACPI
{
	static bool fail = false;
	static bool sdtfail = false;
	void Initialise()
	{
		// search for root system description pointer
		// either in the first 1K of EBDA
		// or between 0xE0000 to 0xFFFFF

		// first get the ebda address.
		uint8_t* ebda = (uint8_t*)(uint64_t)((uint32_t)(*((uint16_t*)0x40E)) << 4);

		Log("Searching for RootSystemDescriptionPointer structure in EBDA at %x", (uintptr_t) ebda);

		// search 1k.
		bool found = false;
		uintptr_t addr = 0;
		for(uint64_t i = 0; i < 1024; i++)
		{
			if(ebda[i] == 'R' && ebda[i + 1] == 'S' && ebda[i + 2] == 'D' && ebda[i + 3] == ' ' && ebda[i + 4] == 'P' && ebda[i + 5] == 'T' && ebda[i + 6] == 'R' && ebda[i + 7] == ' ')
			{
				found = true;
				addr = (uintptr_t) ebda + i;
				break;
			}
		}

		if(!found)
		{
			// search from 0xE0000 to 0xFFFFF
			uint8_t* x = (uint8_t*)0xE0000;
			uint64_t i = 0;

			while((uintptr_t) x < 0xFFFFF)
			{
				if(x[i] == 'R' && x[i + 1] == 'S' && x[i + 2] == 'D' && x[i + 3] == ' ' && x[i + 4] == 'P' && x[i + 5] == 'T' && x[i + 6] == 'R' && x[i + 7] == ' ')
				{
					found = true;
					addr = (uintptr_t) x + i;
					break;
				}

				i++;
			}
		}


		if(!found)
		{
			Log(2, "Error: RSDP not found, cannot continue with ACPI initialisation. Orion-X4 will operate with reduced capabilities");
			fail = true;
			return;
		}

		PrintFormatted("RSDP found at %x\n", addr);
		Log("RSDP found at %x", addr);


		// init the kernel var.
		Kernel::RootACPITable = new RootTable(addr);

		// our work here is done, the rest is handled by class constructors.
	}















	RootTable::RootTable(uintptr_t address)
	{
		// initialise
		Memory::Copy((void*) this->signature, (void*) address, 8);
		Memory::Copy((void*) this->oemid, (void*)(address + 9), 6);
		this->revision = *((uint8_t*)(address + 15));
		this->rsdtaddress = (uint64_t)(*((uint32_t*)(address + 16)));

		// check revision.
		if(this->revision > 0)
		{
			Log("XSDP Structure present");

			this->length = *((uint32_t*)(address + 20));
			this->xsdtaddress = *((uint64_t*)(address + 24));
			this->extendedchecksum = *((uint8_t*)(address + 32));
		}
		else
		{
			this->length = 0;
			this->xsdtaddress = 0;
			this->extendedchecksum = 0;
		}

		// calc checksum.
		uint8_t cs = 0;
		for(uint64_t i = 0; i < 36; i++)
		{
			cs += (uint8_t)(*((uint8_t*)(address + i)));
		}

		Log("RSDP structure checksum: %d, expected %d, %s", cs, 0, cs == 0 ? "match" : "fail");
		if(cs != 0)
		{
			Log("RSDP checksum invalid, ACPI initialisation cannot continue");
			fail = true;
			return;
		}













		// enumerate all System Description Tables
		Log("Found %sRoot System Descriptor Table at %x", this->revision > 0 ? "64-bit " : "", this->rsdtaddress);
		// map the table to virtual memory.

		uint64_t a = (this->revision > 0 ? this->xsdtaddress : this->rsdtaddress) & 0xFFFFFFFFFFFFF000;

		// map the first page first.
		MemoryManager::Virtual::MapAddress(a, a, 0x03);

		uint32_t Length = *((uint32_t*)((uint64_t)(this->revision > 0 ? this->xsdtaddress : this->rsdtaddress) + 4));
		for(uint64_t i = 1; i < (Length + 0xFFF) / 0x1000; i++)
		{
			MemoryManager::Virtual::MapAddress((uint64_t)(a + (i * 0x1000)), (uint64_t)(a + (i * 0x1000)), 0x03);
		}



		// assert checksum.
		cs = 0;
		for(uint64_t cc = 0; cc < Length; cc++)
		{
			cs += *((uint8_t*)(uint64_t)(this->revision > 0 ? this->xsdtaddress : this->rsdtaddress) + cc);
		}

		if(cs != 0)
		{
			Log(2, "RSDT Checksum mismatch; expected 0, got %d, aborting ACPI initialisation.", cs);
			fail = true;
			return;
		}


		uint32_t numentries = (Length - 36) / (this->revision > 0 ? 8 : 4);
		Log("%d SD Tables found, enumerating:", numentries);

		uint32_t* entries = (uint32_t*)((uint64_t) this->rsdtaddress + 36);
		uint64_t* entries64 = (uint64_t*)((uint64_t) this->xsdtaddress + 36);

		// add them all to list.
		this->tables = new Library::LinkedList<SystemDescriptionTable>();

		for(uint64_t k = 0; k < numentries; k++)
		{
			uint64_t ad = this->revision > 0 ? (*((uint64_t*)((uint64_t) entries64 + (k * (this->revision > 0 ? 8 : 4))))) : ((uint64_t)*((uint32_t*)((uint64_t) entries + (k * 4))));

			SystemDescriptionTable* sdt = new SystemDescriptionTable(ad);

			if(sdtfail)
			{
				sdtfail = false;
				continue;
			}

			this->tables->InsertBack(sdt);
		}
	}




















	SystemDescriptionTable::SystemDescriptionTable(uintptr_t addr)
	{
		this->address = addr;

		// map these pages.
		uint64_t a = this->address & 0xFFFFFFFFFFFFF000;

		// map the first page first.
		MemoryManager::Virtual::MapAddress(a, a, 0x03);

		uint32_t Length = *((uint32_t*)(this->address + 4));
		for(uint64_t i = 1; i < (Length + 0xFFF) / 0x1000; i++)
		{
			MemoryManager::Virtual::MapAddress(a + (i * 0x1000), a + (i * 0x1000), 0x03);
		}



		// assert checksum.
		uint8_t cs = 0;
		for(uint64_t cc = 0; cc < Length; cc++)
		{
			cs += *((uint8_t*)(uint64_t) this->address + cc);
		}

		if(cs != 0)
		{
			Log(2, "SDT Checksum mismatch; expected 0, got %d, ignoring SD Table...", cs);
			sdtfail = true;
			return;
		}



		// init the fields
		Memory::Copy((void*) this->signature, (void*) this->address, 4);
		this->length = *((uint32_t*)(this->address + 4));
		this->revision = *((uint8_t*)(this->address + 8));
		Memory::Copy((void*) this->oemid, (void*)(this->address + 10), 6);
		Memory::Copy((void*) this->oemtableid, (void*)(this->address + 16), 8);

		this->oemrevision = *((uint32_t*)(this->address + 24));
		this->creatorid = *((uint32_t*)(this->address + 28));
		this->creatorrevision = *((uint32_t*)(this->address + 32));


		// print names
		Log("SDT: %c%c%c%c", this->signature[0], this->signature[1], this->signature[2], this->signature[3]);
	}
}
}
}


































