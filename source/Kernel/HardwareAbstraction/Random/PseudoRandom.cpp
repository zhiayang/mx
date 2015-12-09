// PseudoRandom.cpp
// Copyright (c) 2014 - 2016, zhiayang@gmail.com
// Licensed under Creative Commons Attribution ShareAlike 3.0 Unported.


#include <stdint.h>
#include <HardwareAbstraction/Random.hpp>

namespace Kernel {
namespace HardwareAbstraction
{
	Random_PseudoRandom::~Random_PseudoRandom()
	{
	}

	Random_PseudoRandom::Random_PseudoRandom(uint32_t _seed)
	{
		this->seed = _seed;
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
