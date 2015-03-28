// Random.hpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under Creative Commons Attribution ShareAlike 3.0 Unported.


#include <stdint.h>
#pragma once

namespace Kernel {
namespace HardwareAbstraction
{
	class Random
	{
		public:
			virtual ~Random();
			virtual uint8_t GenerateByte() = 0;
			virtual uint16_t Generate16() = 0;
			virtual uint32_t Generate32() = 0;
			virtual uint64_t Generate64() = 0;
	};

	class Random_RdRand : public Random
	{
		public:
			virtual ~Random_RdRand();
			virtual uint8_t GenerateByte() override;
			virtual uint16_t Generate16() override;
			virtual uint32_t Generate32() override;
			virtual uint64_t Generate64() override;
	};

	class Random_PseudoRandom : public Random
	{
		public:
			Random_PseudoRandom(uint32_t seed = 1);

			virtual ~Random_PseudoRandom();
			virtual uint8_t GenerateByte() override;
			virtual uint16_t Generate16() override;
			virtual uint32_t Generate32() override;
			virtual uint64_t Generate64() override;

		private:
			uint32_t seed;
	};
}
}
