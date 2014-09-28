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

extern "C" void TaskSwitcherCoOp();
extern "C" void ProcessTimerInterrupt();
extern "C" void HandleSyscall();
extern "C" void KernelInit(uint32_t MultibootMagic, uint32_t MBTAddr)
{
	KernelCore(MultibootMagic, MBTAddr);
}

void sig(int x)
{
	PrintFormatted("received signal %d\n", x);
}

namespace Kernel
{
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
	// Filesystems::VFS::Filesystem* RootFS;
	CPUID::CPUIDData* KernelCPUID;


	// devices
	VideoOutput::GenericVideoDevice* VideoDevice;
	Devices::NIC::GenericNIC* KernelNIC;
	Devices::PS2Controller* KernelPS2Controller;
	Devices::Keyboard* KernelKeyboard;
	Random* KernelRandom;


	bool EnableTimeService = true;
	bool IsKernelInCentralDispatch = false;

	static uint64_t VER_MAJOR;
	static uint64_t VER_MINOR;
	static uint64_t VER_REVSN;
	static uint64_t VER_MINRV;



	HardwareAbstraction::MemoryManager::MemoryMap::MemoryMap_type* K_MemoryMap;

	void KernelCore(uint32_t MultibootMagic, uint32_t MBTAddr)
	{
		using namespace Kernel::HardwareAbstraction::Devices;
		using namespace Kernel::HardwareAbstraction::VideoOutput;
		EndOfKernel = (uint64_t)&KernelEnd;

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
			uint64_t newcr3 = Physical::AllocateDMA(0x18, false);
			Memory::Copy((void*) newcr3, (void*) 0x3000, 0x18000);

			// switch to that cr3.
			Virtual::SwitchPML4T((Virtual::PageMapStructure*) newcr3);
			asm volatile("mov %0, %%cr3" :: "r"(newcr3));
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

		// check if we have enough memory.
		if(K_SystemMemoryInBytes < 0x02000000)
		{
			PrintFormatted("[mx] requires at least 32 Megabytes (33554432 bytes) of memory to operate.\n");
			PrintFormatted("Only %d bytes of memory detected.\n", K_SystemMemoryInBytes);
			PrintFormatted("Install more RAM, or increase the amount of memory in your Virtual Machine.");
			UHALT();
		}


		// Copy the kernel memory map elsewhere.
		{
			uint64_t a = (uint64_t) K_MemoryMap;
			uint32_t s = K_MemoryMap->SizeOfThisStructure;
			K_MemoryMap = (MemoryMap::MemoryMap_type*) Allocate_G(K_MemoryMap->SizeOfThisStructure + sizeof(uint64_t));
			Memory::CopyOverlap((void*) K_MemoryMap, (void*) a, s);
			Log("Memory map relocation complete");
		}


		PrintFormatted("Loading [mx]...\n");
		Log("Initialising Kernel subsystem");

		// tss on page 337 of manual.
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
				{
					// for(uint16_t num = 0; num < PCI::PCIDevice::PCIDevices->size(); num++)
					for(auto dev : *PCI::PCIDevice::PCIDevices)
					{
						dev->PrintPCIDeviceInfo();

						if(dev->GetIsMultifunction())
							PrintFormatted(" ==>Multifunction Device");

						if(PCI::MatchVendorDevice(dev, 0x1234, 0x1111) || PCI::MatchVendorDevice(dev, 0x80EE, 0xBEEF))
							PrintFormatted(" ==>BGA Compatible Video Card: %x", GetTrueLFBAddress());

						PrintFormatted("\n");
					}
				}
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
					Kernel::VideoDevice = new GenericVideoDevice(new BochsGraphicsAdapter(VideoDev));
				}
				else
				{
					PrintFormatted("Error: No supported video card found.\n");
					PrintFormatted("[mx] does not support VGA-only video cards.\n");
					PrintFormatted("Currently, supported systems include: BGA (Bochs, QEMU, VirtualBox) and SVGA (VMWare)\n");
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
				// KernelNIC = new NIC::GenericNIC(new NIC::RTL8139(nic));
			}
		}


		KernelRandom = new Random(new Random_PseudoRandom());

		LFBAddr = VideoDevice->GetFramebufferAddress();
		LFBBufferAddr = LFBAddr;

		Log("Compatible video card located");

		PrintFormatted("Initialising RTC...\n");
		Devices::RTC::Initialise(0);
		Log("RTC Initialised");

		KernelKeyboard = new PS2Keyboard();


		// setup framebuffer
		{
			uint16_t PrefResX = 1024;
			uint16_t PrefResY = 600;

			// Set video mode
			PrintFormatted("\nInitialising Linear Framebuffer at %x...", LFBAddr);
			VideoDevice->SetMode(PrefResX, PrefResY, 32);
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

				// map a bunch of pages for the buffer.
				for(uint64_t k = 0; k < bytes; k++)
					Virtual::MapAddress(LFBBufferAddress_INT + (k * 0x1000), Physical::AllocatePage(), 0x07);

				Virtual::MapRegion(LFBAddr, LFBAddr, bytes, 0x07);
				// LFBBufferAddr = LFBBufferAddress_INT;
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

			Devices::Storage::ATADrive* f1 = Devices::Storage::ATADrive::ATADrives->get(0);
			FSDriverFat32* fs = new FSDriverFat32(f1->Partitions->get(0));

			// mount root fs from partition 0 at /
			VFS::Mount(f1->Partitions->get(0), fs, "/");
			Log("Root FS Mounted at /");
		}

		TTY::Initialise();
		Console::ClearScreen();

		Log("Initialising LaunchDaemons from /System/Library/LaunchDaemons...");
		{
			{
				// setup args:
				// 0: prog name (duh)
				// 1: FB address
				// 2: width
				// 3: height
				// 4: bpp (32)

				const char* path = "/System/Library/LaunchDaemons/displayd.mxa";
				// const char* path = "/a.out";
				auto proc = LoadBinary::Load(path, "displayd",
					(void*) 5, (void*) new uint64_t[5] { (uint64_t) path,
					GetFramebufferAddress(), LinearFramebuffer::GetResX(), LinearFramebuffer::GetResY(), 32 });

				proc->Threads->get(0)->Priority = 2;
				Multitasking::AddToQueue(proc);
			}
		}


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






		Log("Starting console...");
		{

		}




		// kernel stops here
		// for now.
		BLOCK();


































		// // Stuff this in a small scope.
		// // Read the configuration file.
		// // This is mainly used for setting the resolution as well as UTC offset.
		// uint16_t PrefResX = 0, PrefResY = 0;
		// {
		// 	PrintFormatted("Reading Configuration File...\n");
		// 	Log("Reading Configuration file from /System/Library/Preferences/CorePreferences.plist");
		// 	ConfigFile::Initialise();

		// 	PrefResX = (uint16_t) ConfigFile::ReadInteger("ResolutionHorizontal");
		// 	PrefResY = (uint16_t) ConfigFile::ReadInteger("ResolutionVertical");
		// 	Kernel::SystemTime->UTCOffset = (int8_t) ConfigFile::ReadInteger("UTCTimezone");
		// 	Time::PrintSeconds = ConfigFile::ReadBoolean("ClockShowSeconds");

		// 	if(PrefResX == 0 || PrefResY == 0)
		// 	{
		// 		Log(3, "Invalid resolution -- config file may be corrupted, assuming 800x600");
		// 		PrefResX = 800;
		// 		PrefResY = 600;
		// 	}
		// }


		// Multitasking::AddToQueue(Multitasking::CreateKernelThread(Network::DHCP::MonitorThread));
		// Multitasking::AddToQueue(Multitasking::CreateKernelThread(Network::DNS::MonitorThread));
		// Network::Initialise();
		// // Symbolicate::Initialise();



		// // Set video mode
		// PrintFormatted("\nInitialising Linear Framebuffer at %x...", LFBAddr);
		// VideoDevice->SetMode(PrefResX, PrefResY, 32);
		// VideoOutput::LinearFramebuffer::Initialise();

		// Log("Requested resolution of %dx%d, LFB at %x", PrefResX, PrefResY, LFBAddr);


		// // scope this.
		// {
		// 	// Get the set resolution (may be different than our preferred)
		// 	uint16_t ResX = VideoOutput::LinearFramebuffer::GetResX();
		// 	uint16_t ResY = VideoOutput::LinearFramebuffer::GetResX();

		// 	// Calculate how many bytes we need. (remember, 4 bytes per pixel)
		// 	uint32_t bytes = (ResX * ResY) * 4;

		// 	// Get that rounded up to the nearest page
		// 	bytes = (bytes + (4096 - 1)) / 4096;
		// 	LFBInPages = bytes;

		// 	// map a bunch of pages for the buffer.
		// 	for(uint64_t k = 0; k < bytes; k++)
		// 		Virtual::MapAddress(LFBBufferAddress_INT + (k * 0x1000), Physical::AllocatePage(), 0x07);

		// 	Virtual::MapRegion(LFBAddr, LFBAddr, bytes, 0x07);

		// 	// LFBBufferAddr = LFBBufferAddress_INT;
		// }

		// // setup our sockets.
		// Log("Socket subsystem online");
		// {
		// 	Multitasking::Process* kernel = KernelProcess;
		// 	kernel->FileDescriptors[0].Pointer = new IPC::IPCSocketEndpoint(0);
		// 	kernel->FileDescriptors[1].Pointer = new IPC::IPCSocketEndpoint(1);
		// 	kernel->FileDescriptors[2].Pointer = new IPC::IPCSocketEndpoint(2);
		// 	kernel->FileDescriptors[3].Pointer = new IPC::IPCSocketEndpoint(3);
		// }


		// Log("Video mode set");

		// Console::Initialise();
		// // Log("Console initialised");
		// // {
		// // 	uint8_t* CProg = LoadBinary::LoadFileToMemory("/System/Library/CoreServices/VTConsole.oex");
		// // 	LoadBinary::GenericExecutable* Exec = new LoadBinary::GenericExecutable("VTConsole", CProg);
		// // 	Exec->AutomaticLoadExecutable();

		// // 	Exec->SetApplicationType(Multitasking::ThreadType::NormalApplication);
		// // 	Exec->Execute();

		// // 	delete[] CProg;
		// // }

		// IPC::CentralDispatch::InitialiseWindowDispatcher();
		// Log("Window Dispatcher online");




		// KernelKeyboard = new Keyboard(new PS2Keyboard());
		// Console::ClearScreen();
		// Bootscreen::PrintMessage("Loading [mx]\n");

		// #if 0
		// {
		// 	// experimentation area.
		// 	Multitasking::GetThread(1)->SignalHandlers[41] = sig;
		// 	IPC::SendSimpleMessage(1, IPC::MessageTypes::PosixSignal, 41, 0, 0, 0);
		// 	while(true);
		// }
		// #endif



		// // Bootscreen::Initialise();

		// string bms;
		// PrintToString(&bms, "Version %d.%d.%d r%02d -- Build %d", VER_MAJOR, VER_MINOR, VER_REVSN, VER_MINRV, X_BUILD_NUMBER);
		// Bootscreen::PrintMessage(bms.CString());

		// if(false)
		// {
		// 	SLEEP(100);
		// 	Bootscreen::StartProgressBar();
		// 	SLEEP(500);
		// }
		// else
		// {
		// 	// SLEEP(300);
		// }


		// // Load the CarbonShell.oex program.
		// {
		// 	uint8_t* CProg = LoadBinary::LoadFileToMemory("/System/Library/CoreServices/CarbonShell.oex");
		// 	LoadBinary::GenericExecutable* Exec = new LoadBinary::GenericExecutable("CarbonShell", CProg);
		// 	Exec->AutomaticLoadExecutable();
		// 	Exec->SetApplicationType(Multitasking::ThreadType::NormalApplication);

		// 	IPC::CentralDispatch::AddApplicationToList(Exec->proc->Threads->front(), Exec->proc);
		// 	Exec->Execute();

		// 	delete[] CProg;
		// }

		// Log("Starting TimeService");
		// // Multitasking::AddToQueue(Multitasking::CreateKernelThread(Kernel::Time::PrintTime));
		// Console::ClearScreen();

		// Log("Kernel entering Central Dispatch mode...");
		// IsKernelInCentralDispatch = true;
		// IPC::CentralDispatch::Initialise();
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
		Log("System Halted: %s, %s:%d -- %x", message, filename, line, __builtin_return_address(1));
		PrintFormatted("\n\nERROR: %s\nReason: %s\n%s -- Line %d, Return Addr %x\n\n[mx] has met an unresolvable error, and will now halt.", message, !reason ? "None" : reason, filename, line, __builtin_return_address(0));


		UHALT();
	}

	void HaltSystem(const char* message, const char* filename, const char* line, const char* reason)
	{
		Log("System Halted: %s, %s:%s", message, filename, line);
		PrintFormatted("\n\nERROR: %s\nReason: %s\n%s -- Line %s\n\n[mx] has met an unresolvable error, and will now halt.", message, !reason ? "None" : reason, filename, line);

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

	Kernel::HardwareAbstraction::VideoOutput::GenericVideoDevice* GetVideoDevice()
	{
		return VideoDevice;
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




