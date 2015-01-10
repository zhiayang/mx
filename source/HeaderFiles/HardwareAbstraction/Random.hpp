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
			Random(Random* dev);
			virtual ~Random();
			virtual uint8_t GenerateByte();
			virtual uint16_t Generate16();
			virtual uint32_t Generate32();
			virtual uint64_t Generate64();

		protected:
			Random* ActualGen;
	};

	class Random_RdRand : public Random
	{
		public:
			Random_RdRand();

			uint8_t GenerateByte();
			uint16_t Generate16();
			uint32_t Generate32();
			uint64_t Generate64();
	};

	class Random_PseudoRandom : public Random
	{
		public:
			Random_PseudoRandom();

			uint8_t GenerateByte();
			uint16_t Generate16();
			uint32_t Generate32();
			uint64_t Generate64();

		private:
			uint32_t seed;
	};
}
}
