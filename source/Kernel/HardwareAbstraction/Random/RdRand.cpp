// RdRand.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under Creative Commons Attribution ShareAlike 3.0 Unported.


#include <stdint.h>
#include <HardwareAbstraction/Random.hpp>

namespace Kernel {
namespace HardwareAbstraction
{
	Random_RdRand::~Random_RdRand()
	{
	}

	uint8_t Random_RdRand::GenerateByte()
	{
		return (uint8_t) Random_RdRand::Generate16();
	}

	uint16_t Random_RdRand::Generate16()
	{
		uint16_t ret = 0;
		asm volatile("rdrand %%ax; mov %%ax, %[r]" : [r]"=r"(ret) :: "%rax");
		return ret;
	}

	uint32_t Random_RdRand::Generate32()
	{
		uint32_t ret = 0;
		asm volatile("rdrand %%eax; mov %%eax, %[r]" : [r]"=r"(ret) :: "%rax");
		return ret;
	}

	uint64_t Random_RdRand::Generate64()
	{
		uint64_t ret = 0;
		asm volatile("rdrand %%rax; mov %%rax, %[r]" : [r]"=r"(ret) :: "%rax");
		return ret;
	}
}
}
