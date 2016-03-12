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
		Mutex& operator=(Mutex&)				= delete;
		const Mutex& operator=(const Mutex&)	= delete;

		public:
			HardwareAbstraction::Multitasking::Thread* nextOwner = 0;
			HardwareAbstraction::Multitasking::Thread* owner = 0;
			uint64_t recursion = 0;
			uint64_t lock = false;
			uint64_t type = 0;
	};

	class AutoMutex
	{
		Mutex& lock;
		public:
			explicit AutoMutex(Mutex& l);
			AutoMutex& operator = (AutoMutex&&);

			~AutoMutex();


		AutoMutex& operator = (const AutoMutex&) = delete;
		AutoMutex(const AutoMutex& m) = delete;
	};

	bool TryLockMutex(Mutex& lock);
	void LockMutex(Mutex& lock);
	void UnlockMutex(Mutex& lock);










	class Semaphore
	{
		Semaphore& operator=(Semaphore&)				= delete;
		const Semaphore& operator=(const Semaphore&)	= delete;

		public:
			explicit Semaphore(int64_t maxval) : value(maxval) { }
			rde::vector<HardwareAbstraction::Multitasking::Thread*> contestants;
			int64_t value = 0;
	};

	class AutoSemaphore
	{
		Semaphore& sem;
		public:
			explicit AutoSemaphore(Semaphore& _sem);
			AutoSemaphore& operator = (AutoSemaphore&&);

			~AutoSemaphore();

		AutoSemaphore& operator = (const AutoSemaphore&) = delete;
		AutoSemaphore(const AutoSemaphore& as) = delete;
	};

	bool TrySemaphore(Semaphore& sem);
	void AquireSemaphore(Semaphore& sem);
	void ReleaseSemaphore(Semaphore& sem);
}














