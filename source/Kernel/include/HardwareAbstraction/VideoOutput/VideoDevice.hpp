// VideoDevice.hpp
// Copyright (c) 2014 - 2016, zhiayang@gmail.com
// Licensed under Creative Commons Attribution ShareAlike 3.0 Unported.


#include <stdint.h>
#include <HardwareAbstraction/Devices/PCI.hpp>
#include <HardwareAbstraction/DeviceManager.hpp>
#pragma once

namespace Kernel {
namespace HardwareAbstraction {
namespace VideoOutput
{
	enum class VideoDeviceType
	{
		BochsGraphicsAdapter,
		VMWareSVGA,
		GenericVESA,
	};

	class GenericVideoDevice : public Devices::DeviceManager::Device
	{
		public:
			virtual ~GenericVideoDevice();
			virtual void SetMode(uint16_t XRes, uint16_t YRes, uint16_t BitDepth);
			virtual uint16_t GetResX();
			virtual uint16_t GetResY();
			virtual uint16_t GetBitDepth();
			virtual uint64_t GetFramebufferAddress();

		protected:
			Devices::PCI::PCIDevice* pcidev;

	};

	class BochsGraphicsAdapter : public GenericVideoDevice
	{
		public:
			BochsGraphicsAdapter(Devices::PCI::PCIDevice* thisdev);
			void WriteRegister(uint64_t RegIndex, uint64_t Value);
			uint64_t ReadRegister(uint64_t RegIndex);

			virtual void SetMode(uint16_t XRes, uint16_t YRes, uint16_t BitDepth) override;
			virtual uint16_t GetResX() override;
			virtual uint16_t GetResY() override;
			virtual uint16_t GetBitDepth() override;
			virtual uint64_t GetFramebufferAddress() override;
	};
}
}
}










