// ACPI.hpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under Creative Commons Attribution ShareAlike 3.0 Unported.

#include <stdint.h>
#include <rdestl/vector.h>
#pragma once

namespace Kernel {
namespace HardwareAbstraction
{
	namespace ACPI
	{
		void Initialise();

		struct ACPIAddress
		{
			uint8_t isMemMapped;
			uint8_t regBitWidth;
			uint8_t regBitOffset;
			uint8_t reserved;

			uint64_t baseAddress;

		} __attribute__ ((packed));


		struct SystemDescriptionTable;


		struct RootTable
		{
			RootTable(uint64_t address);

			char signature[8];
			char oemid[6];
			uint8_t revision;
			uint64_t rsdtaddress;

			uint32_t length;
			uint64_t xsdtaddress;
			uint8_t extendedchecksum;
			uint8_t reserved[3];

			rde::vector<SystemDescriptionTable*> tables;

		} __attribute__ ((packed));

		struct SDTable;
		struct SystemDescriptionTable
		{
			SystemDescriptionTable(uint64_t address);

			char		signature[4];
			uint32_t	length;
			uint8_t		revision;
			char		oemid[6];
			char		oemtableid[8];
			uint32_t	oemrevision;
			uint32_t	creatorid;
			uint32_t	creatorrevision;
			uint64_t	address;

			SDTable*	table;
		} __attribute__ ((packed));

		// specific tables
		enum class SDTableType
		{
			HPET,
			SSDT,
			APIC,
			FADT
		};

		struct SDTable
		{
		};

		struct HPETTable : SDTable
		{
			HPETTable(uint64_t address);

			uint8_t		hardwareRevisionID;
			uint8_t		compCount		: 5;
			uint8_t		counterSize		: 1;
			uint8_t		reserved		: 1;
			uint8_t		isLegacy		: 1;
			uint16_t	pciVendorID;
			ACPIAddress	baseAddress;
			uint8_t		hpetNumber;
			uint16_t	minTick;
			uint8_t		pageProt;
		} __attribute__ ((packed));

		struct APICTable : SDTable
		{
			APICTable(uint64_t address);

			uint32_t	physLocalControllerAddr;
			uint32_t	flags;

			// fuck all
		};

		namespace APICStructs
		{
			enum class APICStructType
			{
				LocalAPIC				= 0,
				IOAPIC					= 1,
				IntrSrcOverride			= 2,
				NMISource				= 3,
				LocalAPICNMI			= 4,
				LocalAPICAddrOverride	= 5,
				IOSAPIC					= 6,
				LocalSAPIC				= 7,
				PlatformIntrSrcs		= 8,
				Localx2APIC				= 9,
				Localx2APICNMI			= 10,
				GIC						= 11,
				GICD					= 12,
			};

			struct APICStruct
			{
				uint8_t		type;
				uint8_t		length;
			};

			struct LocalAPIC : APICStruct
			{
				uint8_t		ACPIProcId;
				uint8_t		ACPIId;

				uint32_t	flags;
			};

			struct IOAPIC : APICStruct
			{
				uint8_t		IOAPICId;
				uint8_t		reserved;
				uint32_t	physAddress;
				uint32_t	IOAPICInterruptStart;
			};
		}
	}
}
}



















