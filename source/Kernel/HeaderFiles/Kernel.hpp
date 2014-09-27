// Kernel.hpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#pragma once

#ifndef ORION_KERNEL
#define ORION_KERNEL
#endif

#include <stdint.h>
#include <Time.hpp>
#include <HardwareAbstraction/VideoOutput/VideoDevice.hpp>
#include <HardwareAbstraction/Multitasking.hpp>
#include <HardwareAbstraction/IO.hpp>
#include <HardwareAbstraction/Devices/PS2.hpp>
#include <HardwareAbstraction/Devices/Keyboard.hpp>
#include <HardwareAbstraction/ACPI.hpp>
#include <HardwareAbstraction/MemoryManager.hpp>
#include <HardwareAbstraction/CPUID.hpp>
#include <HardwareAbstraction/Random.hpp>
#include <HardwareAbstraction/Devices/NIC.hpp>
#include <HashMap.hpp>
#include <assert.h>


// Configurable addresses.
// Contrary to the name, best not to mess with these.


// TSS at 0x500-0x800
#define LFBBufferAddress_INT			0xFFFFFFFD00000000
#define FPLAddress				0xFFFFFFFF00000000
#define KernelHeapMetadata			0xFFFFFFF000000000
#define KernelHeapAddress			0xFFFFFFFF00000000
#define TemporaryVirtualMapping		0x00000FF00000000
#define DefaultUserStackAddr			0xFFFFFFF0

#define __ORIONX_KERNEL

// Configurable sizes
// As above, try not to mess with these
#define DefaultRing3StackSize	0x4000

// Global IRQ0 tickrate.
#define GlobalTickRate		250
#define GlobalMilliseconds	1000

// Configuration paramaters
#define DEBUG			1
#define SKIPREGDUMP		0
#define SERIALMIRROR		0
#define LOGSPAM		0
#define DMABUFFERSIZE	0x4000
#define ENABLELOGGING	1
#define EXTRADELAY		1

#define SyscallNumber		0xF8
#define IPCNumber		0xF9
#define VolumeMountPoint	"/Volumes/"





namespace Kernel
{
	void Log(uint8_t level, const char* str, ...);
	void Log(const char* str, ...);

	extern const char* K_BinaryUnits[];

	// global variables
	extern uint64_t K_SystemMemoryInBytes;
	extern uint64_t EndOfKernel;
	extern bool EnableTimeService;
	extern bool IsKernelInCentralDispatch;

	// global objects, in Kernel.cpp.
	extern Time::TimeStruct* SystemTime;
	extern HardwareAbstraction::VideoOutput::GenericVideoDevice* VideoDevice;
	extern HardwareAbstraction::Multitasking::Process* KernelProcess;
	extern HardwareAbstraction::Devices::PS2Controller* KernelPS2Controller;
	extern HardwareAbstraction::Devices::Keyboard* KernelKeyboard;
	extern HardwareAbstraction::Devices::NIC::GenericNIC* KernelNIC;
	extern HardwareAbstraction::ACPI::RootTable* RootACPITable;
	extern HardwareAbstraction::MemoryManager::MemoryMap::MemoryMap_type* K_MemoryMap;
	extern HardwareAbstraction::CPUID::CPUIDData* KernelCPUID;
	extern HardwareAbstraction::Random* KernelRandom;




	void Idle();
	void KeyboardService();
	void KernelCore(uint32_t MultibootMagic, uint32_t MBTAddr);
	void KernelCoreThread();
	// bool AssertCondition(bool condition, const char* filename, uint64_t line, const char* reason = 0);
	// void HaltSystem(const char* message, const char* filename, uint64_t line, const char* reason = 0);

	void AssertCondition(const char* file, int line, const char* func, const char* expr);
	void HaltSystem(const char* message, const char* filename, const char* line, const char* reason = 0);


	uint64_t GetCentralDispatchPID();
	uint64_t GetKernelCR3();
	uint64_t GetFramebufferAddress();
	uint64_t GetTrueLFBAddress();
	uint64_t GetLFBLengthInPages();
	Kernel::HardwareAbstraction::VideoOutput::GenericVideoDevice* GetVideoDevice();
	void PrintVersion();

	namespace Utilities
	{
		void DumpBytes(uint64_t address, uint64_t length);
	}
}




// Easy, globally accessible macros for common things.
// #define YIELD()			Yield()
#define SLEEP(x)			Kernel::HardwareAbstraction::Multitasking::Sleep(x)
#define BLOCK()			Kernel::HardwareAbstraction::Multitasking::Block()

#define LOCK(x)			Kernel::LockMutex(x);
#define UNLOCK(x)			Kernel::UnlockMutex(x);

#define Allocate_G(x)			Kernel::HardwareAbstraction::MemoryManager::KernelHeap::AllocateChunk(x)
#define Free_G(x)			Kernel::HardwareAbstraction::MemoryManager::KernelHeap::FreeChunk(x)

#define BOpt_Likely(x)			__builtin_expect((x), 1)
#define BOpt_Unlikely(x)		__builtin_expect((x), 0)

#define UNUSED(x)			((void) x)

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)
;


void operator delete(void* p) _GLIBCXX_USE_NOEXCEPT;
void operator delete[](void* p) _GLIBCXX_USE_NOEXCEPT;
void* operator new(unsigned long size);
void* operator new[](unsigned long size);
void* operator new(unsigned long, void* addr) noexcept;



#define UHALT()					asm volatile("cli; hlt")
#define HALT(x)					Kernel::HaltSystem(x, __FILE__, LINE_STRING, "")
#define BBPNT()					asm volatile("xchg %bx, %bx")





