// PCI.hpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under Creative Commons Attribution ShareAlike 3.0 Unported.


#include <stdint.h>
#include <List.hpp>
#pragma once

namespace Kernel {
namespace HardwareAbstraction {
namespace Devices
{
	namespace PCI
	{
		class PCIDevice
		{

		public:
			PCIDevice(uint16_t Bus, uint16_t Slot, uint8_t Function);

			uint16_t GetBus();
			uint16_t GetSlot();
			uint8_t GetFunction();

			uint8_t GetClass();
			uint8_t GetSubclass();
			uint16_t GetVendorID();
			uint16_t GetDeviceID();
			uint32_t GetAddress();
			uint64_t GetBAR(uint8_t num);
			uint64_t GetBarSize(uint8_t num);
			uint8_t GetInterruptPin();
			uint8_t GetInterruptLine();

			void SetInterruptPin(uint8_t pin);
			void SetInterruptLine(uint8_t line);
			bool IsBARIOPort(uint8_t num);
			bool GetIsMultifunction();

			uint8_t GetHeaderType();
			uint32_t GetRegisterData(uint16_t Offset, uint8_t FirstBit, uint8_t Length);
			void WriteRegisterData(uint16_t Offset, uint8_t FirstBit, uint8_t Length, uint32_t Value);

			void PrintPCIDeviceInfo();


			static Library::LinkedList<PCIDevice>* PCIDevices;


		private:
			uint16_t Bus;
			uint16_t Slot;
			uint8_t Function;

			uint8_t Class;
			uint8_t Subclass;
			uint16_t VendorID;
			uint16_t DeviceID;

			uint8_t ProgIF;

			uint32_t Address;

			uint8_t InterruptPin;
			uint8_t InterruptLine;
		};


		#define MAXBUS		255
		#define MAXSLOT		32
		#define MAXFUNC		8



		void Initialise();
		uint32_t MakeAddr(uint16_t Bus, uint16_t Slot, uint16_t Function);
		uint32_t ReadConfig32(uint32_t Address, uint16_t Offset);
		uint16_t ReadConfig16(uint32_t Address, uint16_t Offset);
		uint8_t ReadConfig8(uint32_t Address, uint16_t Offset);
		void WriteConfig32(uint32_t Address, uint16_t Offset, uint32_t Data);
		void WriteConfig16(uint32_t Address, uint16_t Offset, uint16_t Data);
		void WriteConfig8(uint32_t Address, uint16_t Offset, uint8_t Data);
		uint16_t CheckDeviceExistence(uint16_t Bus, uint16_t Slot);

		PCIDevice* GetDeviceByVendorDevice(uint16_t VendorID, uint16_t DeviceID);
		PCIDevice* GetDeviceByClassSubclass(uint8_t c, uint8_t sc);
		Library::LinkedList<PCIDevice>* SearchByVendorDevice(uint16_t VendorID, uint16_t DeviceID);
		Library::LinkedList<PCIDevice>* SearchByClassSubclass(uint8_t c, uint8_t sc);
		bool MatchVendorDevice(PCIDevice* dev, uint16_t VendorID, uint16_t DeviceID);
		bool MatchClassSubclass(PCIDevice* dev, uint8_t Class, uint8_t Subclass);
	}
}
}
}
