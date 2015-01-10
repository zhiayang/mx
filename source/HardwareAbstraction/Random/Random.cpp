// Random.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under Creative Commons Attribution ShareAlike 3.0 Unported.


#include <stdint.h>
#include <HardwareAbstraction/Random.hpp>


namespace Kernel {
namespace HardwareAbstraction
{
	Random::Random(Random* dev)		{	this->ActualGen = dev;					}
	Random::~Random()				{											}
	uint8_t Random::GenerateByte()	{	return this->ActualGen->GenerateByte();	}
	uint16_t Random::Generate16()	{	return this->ActualGen->Generate16();	}
	uint32_t Random::Generate32()	{	return this->ActualGen->Generate32();	}
	uint64_t Random::Generate64()	{	return this->ActualGen->Generate64();	}
}
}
