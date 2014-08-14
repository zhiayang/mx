// ConfigFile.hpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#pragma once
#include <stdint.h>
#include <String.hpp>

namespace Kernel
{
	namespace ConfigFile
	{
		void Initialise();

		int64_t ReadInteger(const char* key);
		Library::string* ReadString(const char* key);
		bool ReadBoolean(const char* key);
	}
}
