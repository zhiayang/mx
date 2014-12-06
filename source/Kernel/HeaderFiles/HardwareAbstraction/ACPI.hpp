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
		class SystemDescriptionTable
		{
			public:
				SystemDescriptionTable(uintptr_t address);

				const char*	Signature()			{ return this->signature;		}
				uint32_t	Length()			{ return this->length;			}
				uint8_t		Revision()			{ return this->revision;		}
				const char*	OemID()				{ return this->oemid;			}
				const char*	OemTableID()		{ return this->oemtableid;		}
				uint32_t	OemRevision()		{ return this->oemrevision;		}
				uint32_t	CreatorID()			{ return this->creatorid;		}
				uint32_t	CreatorRevision()	{ return this->creatorrevision;	}

			private:
				char		signature[4];
				uint32_t	length;
				uint8_t		revision;
				char		oemid[6];
				char		oemtableid[8];
				uint32_t	oemrevision;
				uint32_t	creatorid;
				uint32_t	creatorrevision;
				uintptr_t	address;
		};

		class RootTable
		{
			public:
				RootTable(uintptr_t address);

				uint64_t GetNumberOfHeaders();
				rde::vector<SystemDescriptionTable*>* GetDescriptionTables();

				char* Signature()				{ return this->signature;				}
				char* OemID()					{ return this->oemid;					}
				uint8_t Revision()				{ return this->revision;				}
				uint32_t RSDTAddress()			{ return (uint32_t) this->rsdtaddress;	}

				uint32_t Length()				{ return this->length;					}
				uint64_t XSDTAddress()			{ return this->xsdtaddress;				}


			private:
				char signature[8];
				char oemid[6];
				uint8_t revision;
				uint64_t rsdtaddress;

				uint32_t length;
				uint64_t xsdtaddress;
				uint8_t extendedchecksum;
				uint8_t reserved[3];

				rde::vector<SystemDescriptionTable*>* tables;
		};
	}
}
}
