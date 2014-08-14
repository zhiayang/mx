// VideoDevice.hpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under Creative Commons Attribution ShareAlike 3.0 Unported.


#include <stdint.h>
#include <HardwareAbstraction/Devices/PCI.hpp>
#pragma once

namespace Kernel {
namespace HardwareAbstraction {
namespace VideoOutput
{
	enum VideoDeviceType
	{
		_BochsGraphicsAdapter	= 0x01,
		_VMWareSVGA		= 0x02,
		_GenericVESA			= 0x03
	};

	class GenericVideoDevice
	{
		public:
			GenericVideoDevice(GenericVideoDevice* dev);
			virtual ~GenericVideoDevice();

			virtual void SetMode(uint16_t XRes, uint16_t YRes, uint16_t BitDepth);

			virtual uint16_t GetResX();
			virtual uint16_t GetResY();
			virtual uint16_t GetBitDepth();
			virtual uint64_t GetFramebufferAddress();

		protected:
			GenericVideoDevice* Device;
	};

	class BochsGraphicsAdapter : public GenericVideoDevice
	{
		public:
			BochsGraphicsAdapter(Devices::PCI::PCIDevice* thisdev);
			void WriteRegister(uint64_t RegIndex, uint64_t Value);
			uint64_t ReadRegister(uint64_t RegIndex);
			void SetMode(uint16_t XRes, uint16_t YRes, uint16_t BitDepth);

			uint16_t GetResX();
			uint16_t GetResY();
			uint16_t GetBitDepth();
			uint64_t GetFramebufferAddress();

		private:
			Devices::PCI::PCIDevice* PCIDev;
	};

	class GenericVESA : public GenericVideoDevice
	{
		public:
			GenericVESA(Devices::PCI::PCIDevice* thisdev);
			void WriteRegister(uint64_t RegIndex, uint64_t Value);
			uint64_t ReadRegister(uint64_t RegIndex);
			void SetMode(uint16_t XRes, uint16_t YRes, uint16_t BitDepth);

			uint16_t GetResX();
			uint16_t GetResY();
			uint16_t GetBitDepth();
			uint64_t GetFramebufferAddress();

		private:
			Devices::PCI::PCIDevice* PCIDev;
	};



	class VMWareSVGA : public GenericVideoDevice
	{
		public:
			VMWareSVGA(Devices::PCI::PCIDevice* thisdev);
			void WriteRegister(uint32_t RegIndex, uint32_t Value);
			uint32_t ReadRegister(uint32_t RegIndex);

			void SetMode(uint16_t XRes, uint16_t YRes, uint16_t BitDepth);

			uint16_t GetResX();
			uint16_t GetResY();
			uint16_t GetBitDepth();
			uint64_t GetFramebufferAddress();
	};
}
}
}
