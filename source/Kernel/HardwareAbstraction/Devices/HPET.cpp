// HPET.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <HardwareAbstraction/Devices.hpp>

// fuck
// #define HPET_COUNTER_CLK_PERIOD_MASK    (0xffffffff00000000ULL)
// #define HPET_COUNTER_CLK_PERIOD_SHIFT   (32UL)
// #define HPET_VENDOR_ID_MASK             (0x00000000ffff0000ULL)
// #define HPET_VENDOR_ID_SHIFT            (16ULL)
// #define HPET_LEG_RT_CAP_MASK            (0x8000)
// #define HPET_COUNTER_SIZE_MASK          (0x2000)
// #define HPET_NUM_TIM_CAP_MASK           (0x1f00)
// #define HPET_NUM_TIM_CAP_SHIFT          (8ULL)

// #define HPET_LEG_RT_CNF_MASK            (2UL)
// #define HPET_ENABLE_CNF_MASK            (1UL)

// #define Tn_INT_ROUTE_CAP_MASK           (0xffffffff00000000ULL)
// #define Tn_INI_ROUTE_CAP_SHIFT          (32UL)
// #define Tn_FSB_INT_DELCAP_MASK          (0x8000UL)
// #define Tn_FSB_INT_DELCAP_SHIFT         (15)
// #define Tn_FSB_EN_CNF_MASK              (0x4000UL)
// #define Tn_FSB_EN_CNF_SHIFT             (14)
// #define Tn_INT_ROUTE_CNF_MASK           (0x3e00UL)
// #define Tn_INT_ROUTE_CNF_SHIFT          (9)
// #define Tn_32MODE_CNF_MASK              (0x0100UL)
// #define Tn_VAL_SET_CNF_MASK             (0x0040UL)
// #define Tn_SIZE_CAP_MASK                (0x0020UL)
// #define Tn_PER_INT_CAP_MASK             (0x0010UL)
// #define Tn_TYPE_CNF_MASK                (0x0008UL)
// #define Tn_INT_ENB_CNF_MASK             (0x0004UL)
// #define Tn_INT_TYPE_CNF_MASK            (0x0002UL)

// #define Tn_FSB_INT_ADDR_MASK            (0xffffffff00000000ULL)
// #define Tn_FSB_INT_ADDR_SHIFT           (32UL)
// #define Tn_FSB_INT_VAL_MASK             (0x00000000ffffffffULL)


enum HPETReg
{
	CapsID	= 0x0,
	Config	= 0x1,
	ISR		= 0x2,
	MainCtr	= 0xF,
	Timer0 	= 0x10,
};


namespace Kernel {
namespace HardwareAbstraction {
namespace Devices
{
	HPET::HPET(ACPI::HPETTable* table)
	{
		Log("Initialising HPET Device, base address %x", table->baseAddress.baseAddress);
		Log("ACPI address type: %s, register width: %d bits, register offset: %d bits", table->baseAddress.isMemMapped ? "memory mapped" : "io memory", table->baseAddress.regBitWidth, table->baseAddress.regBitOffset);

		Log("%d HPET timer%s present", table->compCount, table->compCount == 1 ? "" : "s");

		if(table->isLegacy)
		{
			Log("HPET has Legacy Replacement bit set");
		}
		else
		{
			Log(1, "HPET does not have the Legacy Replacement bit set, setting.");
		}
	}
}









namespace ACPI
{
	HPETTable::HPETTable(uint64_t address, SystemDescriptionTable* sdtable)
	{
		HPETTable* hpet = (HPETTable*) address;
		(void) sdtable;

		this->hardwareRevisionID	= hpet->hardwareRevisionID;
		this->compCount				= hpet->compCount;
		this->counterSize			= hpet->counterSize;
		this->reserved				= hpet->reserved;
		this->isLegacy				= hpet->isLegacy;
		this->pciVendorID			= hpet->pciVendorID;
		this->baseAddress			= hpet->baseAddress;
		this->hpetNumber			= hpet->hpetNumber;
		this->minTick				= hpet->minTick;
		this->pageProt				= hpet->pageProt;
	}
}


}
}
