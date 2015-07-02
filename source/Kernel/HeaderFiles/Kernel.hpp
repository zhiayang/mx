// Kernel.hpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#pragma once

#ifndef ORION_KERNEL
#define ORION_KERNEL
#endif

#include <stdint.h>
#include "Time.hpp"
#include "GlobalTypes.hpp"
#include "HardwareAbstraction/VideoOutput/VideoDevice.hpp"
#include "HardwareAbstraction/Multitasking.hpp"
#include "HardwareAbstraction/IO.hpp"
#include "HardwareAbstraction/Devices/PS2.hpp"
#include "HardwareAbstraction/Devices/Keyboard.hpp"
#include "HardwareAbstraction/ACPI.hpp"
#include "HardwareAbstraction/MemoryManager.hpp"
#include "HardwareAbstraction/CPUID.hpp"
#include "HardwareAbstraction/Random.hpp"
#include "HardwareAbstraction/Devices/NIC.hpp"

#include "JobDispatcher.hpp"
#include <assert.h>


// Configurable addresses.
// Contrary to the name, best not to mess with these.


// TSS at 0x500-0x800
#define FPLAddress					0xFFFFFF0000000000
#define KernelHeapMetadata			0xFFFFFF1000000000
#define KernelHeapAddress			0xFFFFFF2000000000
#define TemporaryVirtualMapping		0x000000FF00000000
#define DefaultUserStackAddr		0xFFFFFFF0

#define __ORIONX_KERNEL

// Configurable sizes
// As above, try not to mess with these
#define DefaultRing3StackSize	0x4000

// Global IRQ0 tickrate.
#define GlobalTickRate		20
#define GlobalMilliseconds	1000

// Configuration paramaters
#define DEBUG				1
#define SKIPREGDUMP			0
#define SERIALMIRROR		0
#define LOGSPAM				0
#define DMABUFFERSIZE		0x4000
#define ENABLELOGGING		1
#define EXTRADELAY			1

#define SyscallNumber		0xF8
#define IPCNumber			0xF9
#define VolumeMountPoint	"/Volumes/"



// wrapper types for big endian shits
// I don't think we will ever support big endian archs
// I cannot handle that shit

template<typename T, int bits>
struct beInt
{
	beInt(T bval) : val(bval) { }
	T le() { return (bits == 16 ? __builtin_bswap16(this->val) : (bits == 32 ? __builtin_bswap32(this->val) : __builtin_bswap64(this->val))); }
	T be() { return this->val; }

	T val;
};

template<> struct beInt<int16_t, 16>;
template<> struct beInt<uint16_t, 16>;
template<> struct beInt<int32_t, 32>;
template<> struct beInt<uint32_t, 32>;
template<> struct beInt<int64_t, 64>;
template<> struct beInt<uint64_t, 64>;




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
	extern HardwareAbstraction::Multitasking::Process* KernelProcess;
	extern HardwareAbstraction::ACPI::RootTable* RootACPITable;
	extern HardwareAbstraction::MemoryManager::MemoryMap::MemoryMap_type* K_MemoryMap;
	extern HardwareAbstraction::CPUID::CPUIDData* KernelCPUID;
	extern HardwareAbstraction::Random* KernelRandom;


	extern bool __debug_flag__;


	void Idle();
	void KeyboardService();
	void KernelCore(uint32_t MultibootMagic, uint32_t MBTAddr);
	void SetupKernelThreads();
	void KernelCoreThread();

	void AssertCondition(const char* file, int line, const char* func, const char* expr);
	void HaltSystem(const char* message, const char* filename, const char* line, const char* reason = 0);


	uint64_t GetCentralDispatchPID();
	uint64_t GetKernelCR3();
	uint64_t GetFramebufferAddress();
	uint64_t GetTrueLFBAddress();
	uint64_t GetLFBLengthInPages();
	void PrintVersion();

	namespace Utilities
	{
		void DumpBytes(uint64_t address, uint64_t length);
		void StackDump(uint64_t* ptr, int num, bool fromTop = false);
	}
}




// Easy, globally accessible macros for common things.
// #define YIELD()			Yield()
#define SLEEP(x)			Kernel::HardwareAbstraction::Multitasking::Sleep(x)
#define BLOCK()				Kernel::HardwareAbstraction::Multitasking::Block()

#define LOCK(x)				Kernel::LockMutex(x);
#define UNLOCK(x)			Kernel::UnlockMutex(x);

#define BOpt_Likely(x)		__builtin_expect((x), 1)
#define BOpt_Unlikely(x)	__builtin_expect((x), 0)

#define UNUSED(x)			((void) x)

#define STRINGIFY(x) STRINGIFY0(x)
#define STRINGIFY0(x) #x
#define __LINE_STRING__ STRINGIFY(__LINE__)
;

#define DEBUG_NEW 0

void operator delete(void* p) _GLIBCXX_USE_NOEXCEPT;
void operator delete[](void* p) _GLIBCXX_USE_NOEXCEPT;
void* operator new(size_t size);
void* operator new[](size_t size);

void* operator new(size_t, void* addr) noexcept;


#if DEBUG_NEW
// fucking hack
const char* __set_line(const char* line);
const char* __set_file(const char* file);

#define new ((__set_file(__FILE__), __set_line(__LINE_STRING__)) && 0) ? nullptr : new
#endif


#define UHALT()					asm volatile("cli; hlt")
#define HALT(x)					Kernel::HaltSystem(x, __FILE__, __LINE_STRING__, "")
#define BBPNT()					asm volatile("xchg %bx, %bx")





