// PCI.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


// Functions to read PCI devices, check for their existence and read their values.
// Defines a PCI device on the bus as an object. Useful.

#include <Kernel.hpp>
#include <HardwareAbstraction/Devices/IOPort.hpp>
#include <Utility.hpp>
#include <StandardIO.hpp>

using namespace Kernel;
using namespace Kernel::HardwareAbstraction::Devices;


namespace Kernel {
namespace HardwareAbstraction {
namespace Devices {
namespace PCI
{
	PCIDevice* GetDeviceByVendorDevice(uint16_t VendorID, uint16_t DeviceID)
	{
		rde::list<PCIDevice*>* r = SearchByVendorDevice(VendorID, DeviceID);
		return r->size() == 0 ? 0 : r->front();
	}

	PCIDevice* GetDeviceByClassSubclass(uint8_t Class, uint8_t Subclass)
	{
		rde::list<PCIDevice*>* r = SearchByClassSubclass(Class, Subclass);
		return r->size() == 0 ? 0 : r->front();
	}


	rde::list<PCIDevice*>* SearchByVendorDevice(uint16_t VendorID, uint16_t DeviceID)
	{
		uint16_t bus = 0, slot = 0;
		uint8_t func = 0;
		uint16_t vendor = 0, device = 0;
		rde::list<PCIDevice*>* ret = new rde::list<PCIDevice*>();

		for(auto dev : *PCIDevice::PCIDevices)
		{
			bus = dev->GetBus();
			slot = dev->GetSlot();
			func = dev->GetFunction();

			vendor = PCI::ReadConfig16(PCI::MakeAddr(bus, slot, func), 0);
			device = (uint16_t)(PCI::ReadConfig32(PCI::MakeAddr(bus, slot, func), 0) >> 16);

			if(vendor == (VendorID == 0xFFFF ? vendor : VendorID) && device == (DeviceID == 0xFFFF ? device : DeviceID))
				ret->push_back(dev);
		}

		return ret;
	}

	rde::list<PCIDevice*>* SearchByClassSubclass(uint8_t c, uint8_t sc)
	{
		uint16_t bus = 0, slot = 0;
		uint8_t func = 0;
		uint16_t tClass = 0, tSubclass = 0;
		rde::list<PCIDevice*>* ret = new rde::list<PCIDevice*>();


		for(auto dev : *PCIDevice::PCIDevices)
		{
			bus = dev->GetBus();
			slot = dev->GetSlot();
			func = dev->GetFunction();


			tClass = (uint8_t)(ReadConfig32(MakeAddr(bus, slot, func), 0x08) >> 24);
			tSubclass = (uint8_t)(ReadConfig32(MakeAddr(bus, slot, func), 0x08) >> 16);

			if(tClass == (c == 0xFF ? tClass : c) && tSubclass == (sc == 0xFF ? tSubclass : sc))
				ret->push_back(dev);
		}

		return ret;
	}



	bool MatchVendorDevice(PCIDevice* dev, uint16_t VendorID, uint16_t DeviceID)
	{
		return (dev->GetVendorID() == VendorID && dev->GetDeviceID() == DeviceID);
	}

	bool MatchClassSubclass(PCIDevice* dev, uint8_t Class, uint8_t Subclass)
	{
		return (dev->GetClass() == Class && dev->GetSubclass() == Subclass);
	}




	uint32_t MakeAddr(uint16_t Bus, uint16_t Slot, uint16_t Function)
	{
		return (uint32_t)(((uint32_t) Bus << 16) | ((uint32_t) Slot << 11) | ((uint32_t) Function << 8) | ((uint32_t)1 << 31));
	}

	// READ REGISTERS

	uint32_t ReadConfig32(uint32_t Address, uint16_t Offset)
	{
		IOPort::Write32(0xCF8, Address + Offset);
		return IOPort::Read32(0xCFC);
	}

	uint16_t ReadConfig16(uint32_t Address, uint16_t Offset)
	{
		IOPort::Write32(0xCF8, Address + Offset);
		return (uint16_t) IOPort::Read32(0xCFC);
	}

	uint8_t ReadConfig8(uint32_t Address, uint16_t Offset)
	{
		IOPort::Write32(0xCF8, Address + Offset);
		return (uint8_t) IOPort::Read32(0xCFC);
	}






	// WRITE REGISTERS

	void WriteConfig32(uint32_t Address, uint16_t Offset, uint32_t Data)
	{
		using namespace Kernel::HardwareAbstraction::Devices::IOPort;
		// Write the address to read.
		Write32(0xCF8, Address + Offset);
		Write32(0xCFC, Data);
	}

	void WriteConfig16(uint32_t Address, uint16_t Offset, uint16_t Data)
	{
		using namespace Kernel::HardwareAbstraction::Devices::IOPort;
		Write32(0xCF8, Address + Offset);
		Write32(0xCFC, (uint32_t) Data);
	}

	void WriteConfig8(uint32_t Address, uint16_t Offset, uint8_t Data)
	{
		using namespace Kernel::HardwareAbstraction::Devices::IOPort;
		Write32(0xCF8, Address + Offset);
		Write32(0xCFC, (uint32_t) Data);
	}

	uint16_t CheckDeviceExistence(uint16_t Bus, uint16_t Slot)
	{
		if(ReadConfig16(MakeAddr(Bus, Slot, 0), 0) == 0xFFFF)
		{
			// Non-existent devices return 0xFFFF on read.
			return 0;
		}
		else
			return 1;
	}

	void Initialise()
	{
		uint16_t vendor = 0;
		uint8_t headertype = 0;
		// PCIDevice::PCIDevices = new Library::LinkedList<PCIDevice>();
		PCIDevice::PCIDevices = new rde::list<PCIDevice*>();

		Log("Scanning PCI bus:");
		for(uint16_t bus = 0; bus < 256; bus++)
		{
			for(uint16_t slot = 0; slot < 32; slot++)
			{
				if(PCI::CheckDeviceExistence(bus, slot))
				{
					vendor = PCI::ReadConfig16(PCI::MakeAddr(bus, slot, 0), 0);
					headertype = (uint8_t)(PCI::ReadConfig32(PCI::MakeAddr(bus, slot, 0), 0x0C) >> 16);

					using PCI::PCIDevice;
					PCIDevice* curdev = new PCI::PCIDevice(bus, slot, 0);
					// PCIDevice::PCIDevices->push_back(curdev);
					PCIDevice::PCIDevices->push_back(curdev);


					uint8_t cl = curdev->GetClass();
					uint8_t sb = curdev->GetSubclass();
					uint16_t devid = curdev->GetDeviceID();

					Log("=> /dev/pci%d > %d:%d, v:%#04x, d:%#04x, c:%#02x:%#02x h:%#02x",
						bus * 32 + slot, bus, slot, vendor, devid, cl, sb, headertype);




					if((headertype) & (1 << 7))
					{
						for(uint8_t func = 1; func < 8; func++)
						{
							vendor = PCI::ReadConfig16(PCI::MakeAddr(bus, slot, func), 0);
							if(vendor != 0xFFFF)
							{
								PCIDevice* devfunc = new PCI::PCIDevice(bus, slot, func);
								// PCIDevice::PCIDevices->push_back(devfunc);
								PCIDevice::PCIDevices->push_back(devfunc);

								cl = devfunc->GetClass();
								sb = devfunc->GetSubclass();
								devid = devfunc->GetDeviceID();

								Log("\t=> /dev/pci%df%d > %d:%d, v:%#04x, d:%#04x, c:%#02x:%#02x h:%#02x",
									bus * 32 + slot, func, bus, slot, vendor, devid, cl, sb, headertype);
							}
						}
					}
				}
			}
		}
	}





	// PCI Device class
	rde::list<PCIDevice*>* PCIDevice::PCIDevices;

	PCIDevice::PCIDevice(uint16_t b, uint16_t s, uint8_t f)
	{
		this->Bus			= b;
		this->Slot			= s;
		this->Function		= f;

		this->Address		= MakeAddr(this->Bus, this->Slot, this->Function);
		this->VendorID		= ReadConfig16(this->Address, 0x00);
		this->DeviceID		= (uint16_t) (ReadConfig32(this->Address, 0x00) >> 16);
		this->Class			= (uint8_t) (ReadConfig32(this->Address, 0x08) >> 24);
		this->Subclass		= (uint8_t) (ReadConfig32(this->Address, 0x08) >> 16);
		this->ProgIF		= (uint8_t) (ReadConfig32(this->Address, 0x08) >> 8);
		this->InterruptLine	= (uint8_t) (ReadConfig8(this->Address, 0x3C));
		this->InterruptPin	= (uint8_t) (ReadConfig32(this->Address, 0x3C + 0x1) >> 8);
	}






	uint16_t PCIDevice::GetBus(){ return this->Bus; }
	uint16_t PCIDevice::GetSlot(){ return this->Slot; }
	uint8_t PCIDevice::GetFunction(){ return this->Function; }

	uint8_t PCIDevice::GetClass(){ return this->Class; }
	uint8_t PCIDevice::GetSubclass(){ return this->Subclass; }
	uint16_t PCIDevice::GetVendorID(){ return this->VendorID; }
	uint16_t PCIDevice::GetDeviceID(){ return this->DeviceID; }
	uint32_t PCIDevice::GetAddress(){ return this->Address; }
	bool PCIDevice::IsBARIOPort(uint8_t num){ return ReadConfig32(this->Address, 0x10 + (num * 0x4)) & 0x1; }

	uint64_t PCIDevice::GetBAR(uint8_t num)
	{
		uint32_t bar = ReadConfig32(this->Address, 0x10 + (num * 0x4));
		if(bar & 0x1)
		{
			// io space, mask to 0xfffffffc
			return bar & 0xFFFFFFFC;
		}
		else
		{
			// memory mapped BAR.
			// get type
			if((bar & 0x6) == 0)
			{
				// 32 bit bar
				return bar & 0xFFFFFFF0;
			}
			else if(num < 5)
			{
				// 64 bit bar.
				return (uint64_t)((bar & 0xFFFFFFF0) + ((uint64_t) ReadConfig32(this->Address, 0x10 + ((num + 1) * 0x4)) << 32));
			}
		}

		return 0;
	}

	uint64_t PCIDevice::GetBarSize(uint8_t num)
	{
		// save original
		uint64_t orig = this->GetBAR(num);

		// write all ones.
		WriteConfig32(this->Address, 0x10 + (num * 4), 0xFFFFFFFF);

		// read it back
		uint32_t mem = ReadConfig32(this->Address, 0x10 + (num * 4));

		// mask info bits
		if(orig & 0x1)
		{
			// io space, mask to 0xfffffffc
			mem &= 0xFFFFFFFC;
		}
		else
		{
			mem &= 0xFFFFFFF0;
		}

		mem = ~mem;
		mem += 1;

		WriteConfig32(this->Address, 0x10 + (num * 4), (uint32_t) orig);
		return mem;
	}

	bool PCIDevice::GetIsMultifunction(){ return (uint8_t)(ReadConfig32(this->Address, 0x0C) >> 16) & (1 << 7); }

	uint8_t PCIDevice::GetHeaderType()
	{
		return (uint8_t)(ReadConfig32(this->Address, 0x0C) >> 16) & ~(1 << 7);
	}

	uint32_t PCIDevice::GetRegisterData(uint16_t Offset, uint8_t FirstBit, uint8_t Length)
	{
		// Write the address to read.
		IOPort::Write32(0xCF8, this->Address + Offset);

		// Read the shit inside.

		uint32_t Data = IOPort::Read32(0xCFC);

		// Truncate accordingly
		switch(Length)
		{
			case sizeof(uint8_t):
				Data = (uint8_t) Data;
				break;

			case sizeof(uint16_t):
				Data = (uint16_t) Data;
				break;
		}

		Data >>= FirstBit;
		return Data;
	}

	void PCIDevice::WriteRegisterData(uint16_t Offset, uint8_t FirstBit, uint8_t Length, uint32_t Value)
	{
		// Write the address to read.
		IOPort::Write32(0xCF8, this->Address + Offset);

		// Truncate accordingly
		switch(Length)
		{
			case sizeof(uint8_t):
				Value = (uint8_t) Value;
				break;

			case sizeof(uint16_t):
				Value = (uint16_t) Value;
				break;
		}

		Value >>= FirstBit;

		IOPort::Write32(0xCFC, Value);
	}


	void PCIDevice::PrintPCIDeviceInfo()
	{
		StdIO::PrintFmt("\t%s=> /dev/pci%d%s%s > %d:%d, v:%#04x, d:%#04x, c:%#02x:%#02x h:%#02x int:%02d", (this->GetFunction() > 0) ? "\t" : "",	(this->GetBus() * 32 + this->GetSlot()), (this->GetFunction() > 0 ? "f" : ""), (this->GetFunction() > 0 ? (Library::Utility::ConvertToString(this->GetFunction())) : ((char*)"")), this->GetBus(), this->GetSlot(), this->GetVendorID(), this->GetDeviceID(), this->GetClass(), this->GetSubclass(), this->GetHeaderType(), this->GetRegisterData(0x3C, 0, 1));
	}


	uint8_t PCIDevice::GetInterruptPin()
	{
		return this->InterruptPin;
	}

	uint8_t PCIDevice::GetInterruptLine()
	{
		return this->InterruptLine;
	}

	void PCIDevice::SetInterruptPin(uint8_t pin)
	{
		this->InterruptPin = pin;
	}

	void PCIDevice::SetInterruptLine(uint8_t line)
	{
		this->InterruptLine = line;
	}
};
}
}
}




