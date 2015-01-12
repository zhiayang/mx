// APIC.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <HardwareAbstraction/ACPI.hpp>
#include <HardwareAbstraction/Devices/APIC.hpp>

namespace Kernel {
namespace HardwareAbstraction {
namespace Devices
{
	APIC::APIC(ACPI::APICTable* table)
	{
		(void) table;
	}
}


namespace ACPI
{
	static const char* readableAPICType[]
	{
		"LocalAPIC",
		"IOAPIC",
		"IntrSrcOverride",
		"NMISource",
		"LocalAPICNMI",
		"LocalAPICAddrOverride",
		"IOSAPIC",
		"LocalSAPIC",
		"PlatformIntrSrcs",
		"Localx2APIC",
		"Localx2APICNMI",
		"GIC",
		"GICD",
	};


	APICTable::APICTable(uint64_t address, SystemDescriptionTable* sdtable)
	{
		APICTable* apic = (APICTable*) address;

		this->physLocalControllerAddr	= apic->physLocalControllerAddr;
		this->flags						= apic->flags;

		uint64_t saddr = address + (sizeof(uint32_t) * 2);
		Log("Parsing APIC table -- Controller Address: %x (phys), flags: %x", this->physLocalControllerAddr, this->flags);

		uint64_t length = sdtable->length - sizeof(SystemDescriptionTable) - sizeof(uint64_t) - sizeof(void*);
		Log("Length: %ld bytes", length);
		for(uint64_t i = 0; i < length;)
		{
			using namespace APICStructs;
			APICStruct* generic = (APICStruct*) saddr;

			Log("Found APIC struct: type %s (%u), %d bytes long", readableAPICType[generic->type], generic->type, generic->length);

			saddr += generic->length;
			i += generic->length;
		}
	}
}


}
}



















