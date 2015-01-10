// PseudoRandom.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under Creative Commons Attribution ShareAlike 3.0 Unported.


#include <stdint.h>
#include <HardwareAbstraction/Random.hpp>

namespace Kernel {
namespace HardwareAbstraction
{
	Random_PseudoRandom::Random_PseudoRandom() : Random(this)
	{
		this->seed = 1;
	}

	uint8_t Random_PseudoRandom::GenerateByte()
	{
		return this->Generate32() & 0xFF;
	}

	uint16_t Random_PseudoRandom::Generate16()
	{
		return this->Generate32() & 0xFFFF;
	}

	uint32_t Random_PseudoRandom::Generate32()
	{
		this->seed = (this->seed * 1103515245u + 12345u) & 0x7FFFFFFFu;
		return this->seed;
	}

	uint64_t Random_PseudoRandom::Generate64()
	{
		uint64_t ret = this->Generate32();
		ret |= this->Generate32() << 31;

		return ret;
	}
}
}
