// LoadRawBinary.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under Creative Commons Attribution ShareAlike 3.0 Unported.

#include <Kernel.hpp>

namespace Kernel {
namespace HardwareAbstraction {
namespace LoadBinary
{
	uint8_t* LoadFileToMemory(const char* filename)
	{
		using Kernel::HardwareAbstraction::Filesystems::VFS::File;

		File* file = new File(filename);
		if(!file->Exists())
		{
			Log(3, "File %s does not exist!", filename);
			return 0;
		}

		uint8_t* data = new uint8_t[file->FileSize()];
		file->Read(data);

		return data;
	}
}
}
}
