// Filesystems.hpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#pragma once
#include <stdint.h>
#include <CircularBuffer.hpp>
#include "Devices/StorageDevice.hpp"
#include <sys/stat.h>

#include "Filesystems/ConsoleVFS.hpp"
#include "Filesystems/HFSPlusVFS.hpp"
#include "Filesystems/Fat32VFS.hpp"

namespace Kernel
{
	namespace HardwareAbstraction
	{
		namespace Multitasking
		{
			struct Process;
		}

		namespace Filesystems
		{
			namespace MBR
			{
				void ReadPartitions(Devices::Storage::StorageDevice* atadev);
			}

			namespace GPT
			{
				void ReadPartitions(Devices::Storage::StorageDevice* atadev);
			}
		}
	}
}
