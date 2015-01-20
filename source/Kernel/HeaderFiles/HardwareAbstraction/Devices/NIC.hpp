// NIC.hpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#include <stdint.h>
#include "PCI.hpp"
#include <GlobalTypes.hpp>
#include <Synchro.hpp>
#include <HardwareAbstraction/DeviceManager.hpp>
#pragma once

namespace Kernel {
namespace HardwareAbstraction {
namespace Devices {
namespace NIC
{
	class GenericNIC : public DeviceManager::Device
	{
		public:
			GenericNIC();
			virtual ~GenericNIC();

			virtual void Reset();
			virtual void SendData(uint8_t* data, uint64_t bytes);
			virtual uint8_t* GetMAC();
			virtual uint64_t GetHardwareType();
			virtual void HandleInterrupt();

		protected:
			Devices::PCI::PCIDevice* pcidev;
			uint8_t MAC[6];
	};


	class RTL8139 : public GenericNIC
	{
		public:
			RTL8139(PCI::PCIDevice* pcidev);
			virtual ~RTL8139() override;
			virtual void Reset() override;
			virtual void SendData(uint8_t* data, uint64_t bytes) override;
			virtual uint8_t* GetMAC() override;
			virtual uint64_t GetHardwareType() override;
			virtual void HandleInterrupt() override;

			void HandlePacket();

			void HandleRxOk();
			void HandleRxErr();
			void HandleTxOk();
			void HandleTxErr();
			void HandleSysErr();


		private:
			uint16_t ioaddr;
			DMAAddr ReceiveBuffer;
			uint8_t CurrentTxBuffer;
			uint64_t SeenOfs;

			bool TxBufferInUse[4];
			DMAAddr TransmitBuffers[4];
			Mutex* transmitbuffermtx[4];
	};
}
}
}
}
