// Kernel.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <String.hpp>
#include <Memory.hpp>
#include <StandardIO.hpp>
#include <math.h>
#include <Console.hpp>
#include <HardwareAbstraction/Interrupts.hpp>
#include <HardwareAbstraction/Network.hpp>
#include <HardwareAbstraction/Devices.hpp>
#include <HardwareAbstraction/VideoOutput.hpp>
#include <HardwareAbstraction/LoadBinary.hpp>
#include <ConfigFile.hpp>
#include <Bootscreen.hpp>
#include "../../.build.h"

using namespace Kernel;
using namespace Kernel::HardwareAbstraction;
using namespace Kernel::HardwareAbstraction::MemoryManager;
using namespace Library;
using namespace Library::StandardIO;


extern "C" uint64_t KernelEnd;
extern "C" uint64_t StartBSS;
extern "C" uint64_t EndBSS;

extern "C" void TaskSwitcherCoOp();
extern "C" void ProcessTimerInterrupt();
extern "C" void HandleSyscall();
extern "C" void KernelInit(uint32_t MultibootMagic, uint32_t MBTAddr)
{
	KernelCore(MultibootMagic, MBTAddr);
}

namespace Kernel
{
	// If you're against global variables, fuck away.
	// Thank you!
	uint64_t K_SystemMemoryInBytes;
	uint64_t EndOfKernel;


	static uint64_t LFBBufferAddr;
	static uint64_t LFBAddr;
	static uint64_t LFBInPages;
	static uint64_t CR3Value = 0x3000;

	// things
	Multitasking::Process* KernelProcess;
	Time::TimeStruct* SystemTime;
	ACPI::RootTable* RootACPITable;
	CPUID::CPUIDData* KernelCPUID;


	// devices
	Devices::PS2Controller* KernelPS2Controller;
	Devices::Keyboard* KernelKeyboard;
	Random* KernelRandom;

	static uint64_t VER_MAJOR;
	static uint64_t VER_MINOR;
	static uint64_t VER_REVSN;
	static uint64_t VER_MINRV;

	// memory map
	HardwareAbstraction::MemoryManager::MemoryMap::MemoryMap_type* K_MemoryMap;

	void KernelCore(uint32_t MultibootMagic, uint32_t MBTAddr)
	{
		using namespace Kernel::HardwareAbstraction::Devices;
		using namespace Kernel::HardwareAbstraction::VideoOutput;
		EndOfKernel = (uint64_t) &KernelEnd;

		// clear the BSS
		{
			uint64_t start = (uint64_t) &StartBSS;
			uint64_t end = (uint64_t) &EndBSS;
			uint64_t length = end - start;

			Memory::Set((void*) start, 0, length);
		}


		// check.
		assert(MultibootMagic == 0x2BADB002);


		// 'compute' the version number from the build number.
		{
			uint64_t v = X_BUILD_NUMBER;

			VER_MAJOR = 4;
			VER_MINOR = v / 10000;
			VER_REVSN = (v - (VER_MINOR * 10000)) / 100;
			VER_MINRV = (v - (VER_MINOR * 10000) - (VER_REVSN * 100)) / 1;
		}

		// Read the memory map.
		Multiboot::Info_type* MBTStruct = (Multiboot::Info_type*) ((uint64_t) MBTAddr);

		// Initalise various subsystems.
		SerialPort::Initialise();			// most important.

		// Read GRUB memory map and init memory managers
		MemoryMap::Initialise(MBTStruct);
		Virtual::Initialise();
		Physical::Bootstrap();
		KernelHeap::Initialise();

		// copy kernel CR3 to somewhere sane-r
		{
			uint64_t oldcr3 = Virtual::GetRawCR3();
			uint64_t newcr3 = Physical::AllocateDMA(0x20, false);
			Memory::Copy((void*) newcr3, (void*) 0x3000, 0xF000);

			// switch to that cr3.
			Virtual::SwitchPML4T((Virtual::PageMapStructure*) newcr3);
			Virtual::ChangeRawCR3(newcr3);

			// asm volatile("mov %0, %%cr3" :: "r"(newcr3));
			asm volatile("invlpg (%0)" : : "a" (newcr3));
			asm volatile("invlpg (%0)" : : "a" (oldcr3));

			CR3Value = newcr3;
		}

		Physical::Initialise();
		Multitasking::Initialise();
		Log("[mx] is initialising...");



		// we use this to store page mappings.
		// TODO: move to temp mapping scheme, where physical pages can come from anywhere.
		Log("PMM Reserved Region from %x to %x", Physical::ReservedRegionForVMM, Physical::ReservedRegionForVMM + Physical::LengthOfReservedRegion);



		// Start the less crucial but still important services.
		Interrupts::Initialise();
		Console80x25::Initialise();
		PIT::Initialise();

		// detect and print SSE support.
		// Why?
		// Answer: no idea.
		Kernel::KernelCPUID = CPUID::Initialise(Kernel::KernelCPUID);
		if(!KernelCPUID->SSE3Instructions())
		{
			PrintFormatted("I don't know how you got this far.\n");
			PrintFormatted("[mx] requires your CPU to support SSE3 instructions.\n");
			UHALT();
		}
		if(!KernelCPUID->OnboardAPIC())
		{
			PrintFormatted("[mx] requires your CPU to have an APIC chip.");
			UHALT();
		}

		// check if we have enough memory.
		if(K_SystemMemoryInBytes < 0x02000000)
		{
			PrintFormatted("[mx] requires at least 64 Megabytes (67 108 864 bytes) of memory to operate.\n");
			PrintFormatted("Only %d bytes of memory detected.\n", K_SystemMemoryInBytes);
			PrintFormatted("Install more RAM, or increase the amount of memory in your Virtual Machine.");
			UHALT();
		}


		// Copy the kernel memory map elsewhere.
		{
			uint64_t a = (uint64_t) K_MemoryMap;
			uint32_t s = K_MemoryMap->SizeOfThisStructure;
			K_MemoryMap = (MemoryMap::MemoryMap_type*) new uint8_t[K_MemoryMap->SizeOfThisStructure + sizeof(uint64_t)];
			Memory::CopyOverlap((void*) K_MemoryMap, (void*) a, s);
			Log("Memory map relocation complete");
		}


		PrintFormatted("Loading [mx]...\n");
		Log("Initialising Kernel subsystem");

		// tss on page 337 of manual. (AMD Vol. 3)
		// Setup the TSS. this will mostly be void once the scheduler initialises.
		{
			*((uint64_t*) (0x2504)) = 0x0000000000060000;
			*((uint64_t*) (0x2524)) = 0x0000000000040000;

			Log("TSS installed");
		}

		// Setup the kernel core as a thread.
		{
			HardwareAbstraction::Interrupts::SetGate(0x20, (uint64_t) ProcessTimerInterrupt, 0x08, 0xEE);
			HardwareAbstraction::Interrupts::SetGate(0xF7, (uint64_t) TaskSwitcherCoOp, 0x08, 0xEE);

			HardwareAbstraction::Interrupts::SetGate(SyscallNumber, (uint64_t) HandleSyscall, 0x08, 0xEF);
			Log("Syscall Handler installed at IDT entry %2x", SyscallNumber);


			using Kernel::HardwareAbstraction::Multitasking::Thread;
			using Kernel::HardwareAbstraction::Multitasking::Process;

			KernelProcess = Multitasking::CreateProcess("Kernel", 0x0, KernelCoreThread, 2);
			Multitasking::AddToQueue(Multitasking::CreateKernelThread(Idle, 0));
			Multitasking::AddToQueue(KernelProcess);

			asm volatile("sti");
		}

		YieldCPU();
	}

	void KernelCoreThread()
	{
		using namespace Kernel::HardwareAbstraction::Devices;
		using namespace Kernel::HardwareAbstraction::VideoOutput;

		Log("Kernel online");



		// Initialise the other, less-essential things.
		PCI::Initialise();

		// needs to be up before we start doing any file I/O
		IO::Initialise();

		Storage::ATA::Initialise();
		PS2::Initialise();
		ACPI::Initialise();


		// Detect and initialise the appropriate driver for the current machine.
		Log("ATA & PCI subsystems online");
		{
			PCI::PCIDevice* VideoDev;
			VideoDev = PCI::GetDeviceByClassSubclass(0x3, 0xFF);

			if(!VideoDev)
			{
				PrintFormatted("Error: No VGA compatible video card found.\n");
				PrintFormatted("[mx] requires such a device to work.\n");
				PrintFormatted("Check your system and try again.\n\n");
				PrintFormatted("Currently, supported systems include: BGA (Bochs, QEMU, VirtualBox) and SVGA (VMWare)\n");
				UHALT();
			}
			else
			{
				if(PCI::MatchVendorDevice(VideoDev, 0x15AD, 0x0405))
				{
					// VMWare's SVGA II Card. (Also available on QEMU)
					// Prefer this, since it has hardware acceleration.
					HALT("VMWare SVGA II Graphics card not implemented");
				}
				else if(PCI::MatchVendorDevice(VideoDev, 0x1234, 0x1111) || PCI::MatchVendorDevice(VideoDev, 0x80EE, 0xBEEF))
				{
					// QEMU, Bochs and VBox's BGA card.
					DeviceManager::AddDevice(new BochsGraphicsAdapter(VideoDev), DeviceType::FramebufferVideoCard);
				}
				else
				{
					PrintFormatted("Error: No supported video card found.\n");
					PrintFormatted("[mx] does not support VGA-only video cards.\n");
					PrintFormatted("Currently, supported systems include: BGA (Bochs, QEMU, VirtualBox) and SVGA II (VMWare)\n");
					UHALT();
				}
			}
		}

		Log("Searching for compatible NIC...");
		{
			using PCI::PCIDevice;
			PCIDevice* nic = PCI::GetDeviceByClassSubclass(0x02, 0xFF);

			if(PCI::MatchVendorDevice(nic, 0x10EC, 0x8139))
			{
				Log("Realtek RTL8139 NIC found, initialising driver...");
				DeviceManager::AddDevice(new NIC::RTL8139(nic), DeviceType::EthernetNIC);
			}
		}

		KernelRandom = new Random(new Random_PseudoRandom());

		Log("Compatible video card located");

		PrintFormatted("Initialising RTC...\n");
		// Devices::RTC::Initialise(0);
		Log("RTC Initialised");

		KernelKeyboard = new PS2Keyboard();

		// setup framebuffer
		{
			uint16_t PrefResX = 1024;
			uint16_t PrefResY = 600;

			// it better exist
			GenericVideoDevice* vd = (GenericVideoDevice*) DeviceManager::GetDevice(DeviceType::FramebufferVideoCard);
			assert(vd);

			LFBAddr = vd->GetFramebufferAddress();
			LFBBufferAddr = LFBAddr;

			// Set video mode
			PrintFormatted("\nInitialising Linear Framebuffer at %x...", LFBAddr);
			vd->SetMode(PrefResX, PrefResY, 32);
			VideoOutput::LinearFramebuffer::Initialise();

			Log("Requested resolution of %dx%d, LFB at %x", PrefResX, PrefResY, LFBAddr);


			// scope this.
			{
				// Get the set resolution (may be different than our preferred)
				uint16_t ResX = VideoOutput::LinearFramebuffer::GetResX();
				uint16_t ResY = VideoOutput::LinearFramebuffer::GetResX();

				// Calculate how many bytes we need. (remember, 4 bytes per pixel)
				uint32_t bytes = (ResX * ResY) * 4;

				// Get that rounded up to the nearest page
				bytes = (bytes + (4096 - 1)) / 4096;
				LFBInPages = bytes;

				Virtual::MapRegion(LFBAddr, LFBAddr, bytes, 0x07);
			}

			Log("Video mode set");
			Console::Initialise();
		}



		// manual jump start.
		{
			using namespace Filesystems;

			VFS::Initialise();

			// open fds for stdin, stdout and stderr.
			VFS::InitIO();

			// todo: detect fs type.
			{
				Devices::Storage::ATADrive* f1 = Devices::Storage::ATADrive::ATADrives->front();
				FSDriverFat32* fs = new FSDriverFat32(f1->Partitions.front());

				// mount root fs from partition 0 at /
				VFS::Mount(f1->Partitions.front(), fs, "/");
				Log("Root FS Mounted at /");
			}
		}

		TTY::Initialise();
		Console::ClearScreen();

		Log("Initialising LaunchDaemons from /System/Library/LaunchDaemons...");
		{
			// setup args:
			// 0: prog name (duh)
			// 1: FB address
			// 2: width
			// 3: height
			// 4: bpp (32)

			const char* path = "/System/Library/LaunchDaemons/displayd.mxa";
			auto proc = LoadBinary::Load(path, "displayd",
				(void*) 5, (void*) new uint64_t[5] { (uint64_t) path,
				GetFramebufferAddress(), LinearFramebuffer::GetResX(), LinearFramebuffer::GetResY(), 32 });

			Multitasking::AddToQueue(proc);
		}

		PrintFormatted("[mx] has completed initialisation.\n");
		Log("Kernel init complete\n--------------------------------------------------------\n");


		// PrintFormatted("mutex tests\n");
		// {
		// 	test = new Mutex;
		// 	auto func1 = []()
		// 	{
		// 		PrintFormatted("locking mutex\n");
		// 		LOCK(test);

		// 		PrintFormatted("sleeping for 2 seconds\n");
		// 		SLEEP(2000);
		// 		PrintFormatted("lock released\n");
		// 		UNLOCK(test);
		// 	};

		// 	auto func2 = []()
		// 	{
		// 		PrintFormatted("waiting for lock...");
		// 		SLEEP(1000);
		// 		PrintFormatted("trying mutex\n");

		// 		while(!TryLockMutex(test))
		// 			PrintFormatted("x");

		// 		PrintFormatted("locked!\n");
		// 		UNLOCK(test);
		// 	};

		// 	Multitasking::AddToQueue(Multitasking::CreateKernelThread(func2));
		// 	Multitasking::AddToQueue(Multitasking::CreateKernelThread(func1));
		// }


		// kernel stops here
		// for now.
		BLOCK();
	}

	void Idle()
	{
		while(true)
		{
			// Physical::CoalesceFPLs();
			YieldCPU();
		}
	}

	void HaltSystem(const char* message, const char* filename, uint64_t line, const char* reason)
	{
		Log("System Halted: %s, %s:%d -- (0: %x, 1: %x, 2: %x, 3: %x, 4: %x)", message, filename, line, __builtin_return_address(0), __builtin_return_address(1), __builtin_return_address(2), __builtin_return_address(3), __builtin_return_address(4));


		PrintFormatted("\n\nFATAL ERROR: %s\nReason: %s\n%s -- Line %d, Return Addr (0: %x, 1: %x, 2: %x, 3: %x, 4: %x)\n\n[mx] has met an unresolvable error, and will now halt.", message, !reason ? "None" : reason, filename, line, __builtin_return_address(0), __builtin_return_address(1), __builtin_return_address(2), __builtin_return_address(3), __builtin_return_address(4));


		UHALT();
	}

	void HaltSystem(const char* message, const char* filename, const char* line, const char* reason)
	{
		Log("System Halted: %s, %s:%s", message, filename, line);
		PrintFormatted("\n\nFATAL ERROR: %s\nReason: %s\n%s -- Line %s\n\n[mx] has met an unresolvable error, and will now halt.", message, !reason ? "None" : reason, filename, line);

		UHALT();
	}


	void AssertCondition(const char* file, int line, const char* func, const char* expr)
	{
		(void) line;
		(void) func;
		HaltSystem("assert() Failed!", file, line, expr);
	}


	bool AssertCondition(bool condition, const char* filename, const char* line, const char* reason)
	{
		if(BOpt_Unlikely(condition))
			return 0;

		HaltSystem("assert() Failed!", filename, line, reason);
		return 1;
	}

	extern "C" void __assert(const char* f, unsigned long l, const char* fn, const char* e)
	{
		AssertCondition(f, (int) l, fn, e);
	}

	uint64_t GetFramebufferAddress()
	{
		return LFBBufferAddr;
	}

	uint64_t GetTrueLFBAddress()
	{
		return LFBAddr;
	}

	uint64_t GetLFBLengthInPages()
	{
		return LFBInPages;
	}

	void PrintVersion()
	{
		PrintFormatted("[mx] Version %d.%d.%d r%02d -- Build %d\n", VER_MAJOR, VER_MINOR, VER_REVSN, VER_MINRV, X_BUILD_NUMBER);
	}

	uint64_t GetKernelCR3()
	{
		return CR3Value;
	}

	const char* K_BinaryUnits[]
	{
		"Bytes",
		"KB",
		"MB",
		"GB",
		"TB",
		"PB",
		"EB",
		"ZB",
		"YB"
	};

}












namespace rapidxml
{
	void parse_error_handler(const char *what, void *where)
	{
		HALT(what);
		UNUSED(where);
	}
}

void operator delete(void* p) _GLIBCXX_USE_NOEXCEPT
{
	KernelHeap::FreeChunk(p);
}

void operator delete[](void* p) _GLIBCXX_USE_NOEXCEPT
{
	KernelHeap::FreeChunk(p);
}

void* operator new(unsigned long size)
{
	return (void*) KernelHeap::AllocateChunk(size);
}

void* operator new[](unsigned long size)
{
	return (void*) KernelHeap::AllocateChunk(size);
}

extern "C" void* malloc(size_t s)
{
	return KernelHeap::AllocateChunk(s);
}
extern "C" void free(void* ptr)
{
	KernelHeap::FreeChunk(ptr);
}
extern "C" void* realloc(void* ptr, size_t size)
{
	void* np = KernelHeap::AllocateChunk(size);
	size_t os = KernelHeap::QuerySize(ptr);

	Memory::Copy(np, ptr, os);
	KernelHeap::FreeChunk(ptr);

	return np;
}




