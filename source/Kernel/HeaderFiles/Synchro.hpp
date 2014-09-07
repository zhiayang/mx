// Synchro.hpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#pragma once
#include <stdint.h>
#include <String.hpp>
#include <List.hpp>

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
			Library::LinkedList<HardwareAbstraction::Multitasking::Thread>* contestants = 0;
			HardwareAbstraction::Multitasking::Thread* owner = 0;
			uint64_t recursion = 0;
			uint64_t lock = false;
	};

	class AutoMutex
	{
		Mutex* lock;
		public:
			AutoMutex(Mutex* l);
			AutoMutex(const AutoMutex& m);
			~AutoMutex();
	};

	bool TryLockMutex(Mutex* lock);
	void LockMutex(Mutex* lock);
	void UnlockMutex(Mutex* lock);





	class Semaphore
	{
		public:
			Library::LinkedList<HardwareAbstraction::Multitasking::Thread>* contestants = 0;
			HardwareAbstraction::Multitasking::Thread* owner = 0;
			uint64_t recursion = 0;
			int64_t value = 0;
	};

	class AutoSemaphore
	{
		Semaphore* sem;
		public:
			AutoSemaphore(Semaphore* _sem);
			AutoSemaphore(const AutoSemaphore& as);
			~AutoSemaphore();
	};

	bool TrySemaphore(Semaphore* sem);
	void AquireSemaphore(Semaphore* sem);
	void ReleaseSemaphore(Semaphore* sem);
}














