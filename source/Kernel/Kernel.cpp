// Kernel.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <String.hpp>
#include <Memory.hpp>
#include <StandardIO.hpp>
#include <Colours.hpp>
#include <math.h>
#include <Console.hpp>
#include <HardwareAbstraction/Interrupts.hpp>
#include <HardwareAbstraction/Network.hpp>
#include <HardwareAbstraction/Devices.hpp>
#include <HardwareAbstraction/VideoOutput.hpp>
#include <HardwareAbstraction/LoadBinary.hpp>
#include <ConfigFile.hpp>
#include <Bootscreen.hpp>
#include <Symbolicate.hpp>
#include "../../.build.h"

#include "IPC/Dispatchers/CentralDispatch.hpp"


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
	HashMap<std::string, std::string>* KernelConfigFile;
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
		Mutexes::Initialise();

		// detect and print SSE support.
		// Why?
		// Answer: no idea.
		Kernel::KernelCPUID = CPUID::Initialise(Kernel::KernelCPUID);
		if(!KernelCPUID->SSE3Instructions())
		{
			PrintFormatted("I don't know how you got this far.\n");
			PrintFormatted("Orion-X4 requires your CPU to support SSE3 instructions.\n");
			UHALT();
		}

		// check if we have enough memory.
		if(K_SystemMemoryInBytes < 0x02000000)
		{
			PrintFormatted("Orion-X4 requires at least 32 Megabytes (33554432 bytes) of memory to operate.\n");
			PrintFormatted("Only %d bytes of memory detected.\n", K_SystemMemoryInBytes);
			PrintFormatted("Install more RAM, or increase the amount of memory in your Virtual Machine.");
			UHALT();
		}

		// copy kernel CR3 to somewhere sane-r
		{
			// uint64_t newcr3 = Physical::AllocatePage_Csontiguous(0x18);
			uint64_t newcr3 = Physical::AllocateDMA(0x18, false);
			Memory::Copy64((void*) newcr3, (void*) 0x3000, 0x18000 / 8);

			// switch to that cr3.
			Virtual::SwitchPML4T((Virtual::PageMapStructure*) newcr3);
			asm volatile("mov %0, %%cr3" :: "r"(newcr3));
			CR3Value = newcr3;
			Log("Moved Kernel CR3 to %x", CR3Value);
		}


		// Copy the kernel memory map elsewhere.
		{
			uint64_t a = (uint64_t) K_MemoryMap;
			uint32_t s = K_MemoryMap->SizeOfThisStructure;
			K_MemoryMap = (MemoryMap::MemoryMap_type*) Allocate_G(K_MemoryMap->SizeOfThisStructure + sizeof(uint32_t));
			Library::Memory::CopyOverlap((void*) K_MemoryMap, (void*) a, s);
			Log("Memory map relocation complete");
		}


		PrintFormatted("Loading [mx]...\n");
		Log("Initialising Kernel subsystem");



		// Setup the TSS. this will mostly be void once the scheduler initialises.
		{
			Memory::Set32((void*) 0x0504, 0x00060000, 1);
			Memory::Set32((void*) 0x0508, 0x00000000, 1);
			Memory::Set32((void*) 0x0524, 0x00040000, 1);
			Memory::Set32((void*) 0x0528, 0x00000000, 1);
			asm volatile("mov $0x28, %ax; ltr %ax");

			Log("TSS installed");
		}


		// Setup the kernel core as a kernel-space thread.
		{
			HardwareAbstraction::Interrupts::SetGate(0x20, (uint64_t) ProcessTimerInterrupt, 0x08, 0xEE);
			HardwareAbstraction::Interrupts::SetGate(0xF7, (uint64_t) TaskSwitcherCoOp, 0x08, 0xEE);

			HardwareAbstraction::Interrupts::SetGate(SyscallNumber, (uint64_t) HandleSyscall, 0x08, 0xEE);
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
				PrintFormatted("Orion-X4 requires such a device to work.\n");
				PrintFormatted("Check your system and try again.\n\n");
				PrintFormatted("Currently, supported systems include: BGA (Bochs, QEMU, VirtualBox) and SVGA (VMWare)\n");
				{
					for(uint16_t num = 0; num < PCI::PCIDevice::PCIDevices->Size(); num++)
					{
						PCI::PCIDevice::PCIDevices->Get(num)->PrintPCIDeviceInfo();

						if(PCI::PCIDevice::PCIDevices->Get(num)->GetIsMultifunction())
							PrintFormatted(" ==>%w Multifunction Device", Colours::Yellow);

						if(PCI::MatchVendorDevice(PCI::PCIDevice::PCIDevices->Get(num), 0x1234, 0x1111) || PCI::MatchVendorDevice(PCI::PCIDevice::PCIDevices->Get(num), 0x80EE, 0xBEEF))
							PrintFormatted(" ==>%w BGA Compatible Video Card:%w %x", Colours::Cyan, Colours::Orange, GetTrueLFBAddress());

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
					PrintFormatted("Orion-X4 does not support VGA-only video cards.\n");
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

		// RootFS = f1->Partitions->Get(0)->GetFilesystem()->RootFS();


		PrintFormatted("Initialising RTC...\n");
		Devices::RTC::Initialise(0);
		Log("RTC Initialised");


		// manual jump start.
		{
			using namespace Filesystems;
			using namespace Filesystems::VFS;
			Devices::Storage::ATADrive* f1 = Devices::Storage::ATADrive::ATADrives->Get(0);
			FSDriverFat32* fs = new FSDriverFat32(f1->Partitions->Get(0));

			// mount root fs from partition 0 at /
			VFS::Mount(f1->Partitions->Get(0), fs, "/");
			// auto fd = OpenFile("/hello", 0);
			// PrintFormatted("%d\n", fd);

			vnode* n = new vnode;
			n->refcount = 1;
			n->type = VNodeType::Folder;
			n->info = new fsref;

			struct vnode_data
			{
				std::string* name;
				uint32_t size;
				uint32_t entrycluster;
				std::vector<uint32_t>* clusters;
			};

			n->info->driver = fs;
			n->info->data = new vnode_data;
			vnode_data* vnd = (vnode_data*) n->info->data;

			vnd->entrycluster = 2;

			auto ret = fs->GetClusterChain(n);
			for(auto v : *ret)
			{
				PrintFormatted("%d\n", v);
			}
		}



		// kernel stops here
		// for now.
		PrintFormatted("Kernel Halted\n");
		UHALT();


































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
		// Bootscreen::PrintMessage("Loading Orion-X4\n");

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

		// 	IPC::CentralDispatch::AddApplicationToList(Exec->proc->Threads->Front(), Exec->proc);
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



	bool AssertCondition(bool condition, const char* filename, uint64_t line, const char* reason)
	{
		(void) line;
		if(BOpt_Unlikely(condition))
			return 0;

		HaltSystem("assert() Failed!", filename, "line dammit", reason);
		return 1;
	}

	bool AssertCondition(bool condition, const char* filename, const char* line, const char* reason)
	{
		if(BOpt_Unlikely(condition))
			return 0;

		HaltSystem("assert() Failed!", filename, line, reason);
		return 1;
	}


	void HaltSystem(const char* message, const char* filename, uint64_t line, const char* reason)
	{
		Log("System Halted: %s, %s:%d -- %x", message, filename, line, __builtin_return_address(1));
		PrintFormatted("\n\n%wERROR: %w%s%r\n%wReason%r: %w%s%r\n%w%s%r -- %wLine %w%d%r%w\n\n%wOrion-X4 has met an unresolvable error, and will now halt.", Colours::Yellow, Colours::Red, message, Colours::DarkCyan, Colours::Orange, !reason ? "None" : reason, Colours::Cyan, filename, Colours::Silver, Colours::Blue, line, Colours::Silver, Colours::Silver);


		UHALT();
	}

	void HaltSystem(const char* message, const char* filename, const char* line, const char* reason)
	{
		Log("System Halted: %s, %s:%s", message, filename, line);
		PrintFormatted("\n\n%wERROR: %w%s%r\n%wReason%r: %w%s%r\n%w%s%r -- %wLine %w%s%r%w\n\n%wOrion-X4 has met an unresolvable error, and will now halt.", Colours::Yellow, Colours::Red, message, Colours::DarkCyan, Colours::Orange, !reason ? "None" : reason, Colours::Cyan, filename, Colours::Silver, Colours::Blue, line, Colours::Silver, Colours::Silver);

		UHALT();
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
		PrintFormatted("%wOrion-X4%r Version %w%d.%d.%d %wr%02d%r -- Build %w%d\n", Colours::Chartreuse, Colours::DarkCyan, VER_MAJOR, VER_MINOR, VER_REVSN, Colours::Orange, VER_MINRV, Colours::Cyan, X_BUILD_NUMBER);
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

void operator delete(void* p) noexcept
{
	Free_G(p);
}

void operator delete[](void* p) noexcept
{
	Free_G(p);
}

void* operator new(unsigned long size)
{
	return (void*) Allocate_G((uint32_t) size);
}

void* operator new[](unsigned long size)
{
	return (void*) Allocate_G((uint32_t) size);
}

// void* operator new(unsigned long, void* addr) noexcept
// {
// 	return addr;
// }








