// Mutexes.hpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#pragma once
#include <stdint.h>
#include <String.hpp>

namespace Kernel
{
	class Mutex
	{
		public:
			uint64_t owner = 0;
			uint64_t recursion = 0;
			uint64_t* contestants = 0;
			uint64_t numcontestants = 0;
			bool lock = false;
	};

	class AutoMutex
	{
		Mutex* lock;
		public:
			AutoMutex(Mutex* l);
			AutoMutex(const AutoMutex& m);
			~AutoMutex();
	};

	namespace Mutexes
	{
		void Initialise();
		extern Mutex* ConsoleOutput;
		extern Mutex* SystemTime;
		extern Mutex* KernelHeap;
		extern Mutex* SerialLog;
		extern Mutex* WindowDispatcher;
		extern Mutex* TestMutex;
	}
}
