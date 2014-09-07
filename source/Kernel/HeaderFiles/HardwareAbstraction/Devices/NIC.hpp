// NIC.hpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#include <stdint.h>
#include "PCI.hpp"
#include <Synchro.hpp>
#pragma once

namespace Kernel {
namespace HardwareAbstraction {
namespace Devices {
namespace NIC
{
	class GenericNIC
	{
		public:
			GenericNIC(GenericNIC* dev);
			virtual ~GenericNIC();

			virtual void Reset();
			virtual void SendData(uint8_t* data, uint64_t bytes);
			virtual uint8_t* ReceiveData(uint8_t* data, uint64_t bytes);
			virtual void HandleInterrupt();
			virtual uint8_t* GetMAC();
			virtual uint64_t GetHardwareType();

		protected:
			GenericNIC* device;
			Devices::PCI::PCIDevice* pcidev;
			uint8_t MAC[6];
	};


	class RTL8139 : public GenericNIC
	{
		public:
			RTL8139(PCI::PCIDevice* pcidev);
			~RTL8139();
			void Reset();
			void SendData(uint8_t* data, uint64_t bytes);
			uint8_t* ReceiveData(uint8_t* data, uint64_t bytes);
			uint64_t GetHardwareType();

			void HandleInterrupt();
			void HandlePacket();

			void HandleRxOk();
			void HandleRxErr();
			void HandleTxOk();
			void HandleTxErr();
			void HandleSysErr();

		private:
			uint16_t ioaddr;
			uint8_t* ReceiveBuffer;
			uint8_t CurrentTxBuffer;
			uint64_t SeenOfs;

			bool TxBufferInUse[4];
			uint64_t TransmitBuffers[4];
			Mutex* transmitbuffermtx[4];
	};
}
}
}
}
