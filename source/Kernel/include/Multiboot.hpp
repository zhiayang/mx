// Multiboot.hpp
// Copyright (c) 2013 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#pragma once
#include <stdint.h>

namespace Multiboot
{

	// The Multiboot information.
	struct __attribute__((packed)) Info_type
	{
		uint32_t flags, mem_lower, mem_upper, boot_device, cmdline, mods_count, mods_addr;
		uint32_t num, size, addr, shndx;
		uint32_t mmap_length, mmap_addr;
	};

	struct __attribute__((packed)) Module_type { uint32_t mod_start, mod_end, string, reserved; };
	struct __attribute__((packed)) MemoryMap_type
	{
		uint32_t Size;
		uint32_t BaseAddr_Low;
		uint32_t BaseAddr_High;
		uint32_t Length_Low;
		uint32_t Length_High;
		uint32_t Type;
	};
}
