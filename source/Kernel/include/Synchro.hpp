// Synchro.hpp
// Copyright (c) 2013 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#pragma once
#include <stdint.h>
#include <rdestl/vector.h>

namespace Kernel
{
	namespace HardwareAbstraction {
	namespace Multitasking
	{
		struct Thread;
	}
	}

	class Mutex
	{
		public:
			rde::vector<HardwareAbstraction::Multitasking::Thread*> contestants;
			HardwareAbstraction::Multitasking::Thread* owner = 0;
			uint64_t recursion = 0;
			uint64_t lock = false;
			uint64_t type = 0;
	};

	class AutoMutex
	{
		Mutex* lock;
		public:
			explicit AutoMutex(Mutex* l);
			AutoMutex(const AutoMutex& m);
			~AutoMutex();
	};

	bool TryLockMutex(Mutex* lock);
	void LockMutex(Mutex* lock);
	void UnlockMutex(Mutex* lock);





	class Semaphore
	{
		public:
			explicit Semaphore(int64_t maxval) : value(maxval) { }
			rde::vector<HardwareAbstraction::Multitasking::Thread*> contestants;
			int64_t value = 0;
	};

	class AutoSemaphore
	{
		Semaphore* sem;
		public:
			explicit AutoSemaphore(Semaphore* _sem);
			AutoSemaphore(const AutoSemaphore& as);
			~AutoSemaphore();
	};

	bool TrySemaphore(Semaphore* sem);
	void AquireSemaphore(Semaphore* sem);
	void ReleaseSemaphore(Semaphore* sem);
}














