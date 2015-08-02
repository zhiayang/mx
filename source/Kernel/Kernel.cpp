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
using namespace StdIO;


extern "C" uint64_t KernelEnd;
extern "C" uint64_t StartBSS;
extern "C" uint64_t EndBSS;


static uint64_t LoadedCursorX = 1;
static uint64_t LoadedCursorY = 1;

extern "C" void TaskSwitcherCoOp();
extern "C" void ProcessTimerInterrupt();
extern "C" void HandleSyscall();
extern "C" void KernelInit(uint64_t MultibootMagic, uint64_t MBTAddr, uint64_t cx, uint64_t cy)
{
	LoadedCursorX = cx;
	LoadedCursorY = cy;

	KernelCore((uint32_t) MultibootMagic, (uint32_t) MBTAddr);
}

extern "C" void KernelThreadInit()
{
	Kernel::SetupKernelThreads();
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
	// static fd_t aSock = 0;

	// things
	Multitasking::Process* KernelProcess;
	Time::TimeStruct* SystemTime;
	ACPI::RootTable* RootACPITable;
	CPUID::CPUIDData* KernelCPUID;


	// devices
	Random* KernelRandom;

	bool __debug_flag__ = false;

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


		Log("[mx] kernel has control");
		// copy kernel CR3 to somewhere sane-r
		{
			uint64_t oldcr3 = Virtual::GetRawCR3();
			uint64_t newcr3 = Physical::AllocateFromReserved(0x20);
			Memory::Copy((void*) newcr3, (void*) 0x3000, 0xF000);

			// switch to that cr3.
			Virtual::SwitchPML4T((Virtual::PageMapStructure*) newcr3);
			Virtual::ChangeRawCR3(newcr3);

			asm volatile("invlpg (%0)" : : "a" (newcr3));
			asm volatile("invlpg (%0)" : : "a" (oldcr3));

			CR3Value = newcr3;
		}

		Physical::Initialise();
		Multitasking::Initialise();

		PrintVersion();

		// we use this to store page mappings.
		// TODO: move to temp mapping scheme, where physical pages can come from anywhere.
		Log("PMM Reserved Region from %x to %x", Physical::ReservedRegionForVMM, Physical::ReservedRegionForVMM + Physical::LengthOfReservedRegion);

		// Start the less crucial but still important services.
		Interrupts::Initialise();
		Console80x25::Initialise();
		Console80x25::MoveCursor((uint16_t) LoadedCursorX, (uint16_t) LoadedCursorY);
		PIT::Initialise();

		// detect and print SSE support.
		// Why?
		// Answer: no idea.
		Kernel::KernelCPUID = CPUID::Initialise(Kernel::KernelCPUID);
		if(!KernelCPUID->SSE3Instructions())
		{
			StdIO::PrintFmt("I don't know how you got this far.\n");
			StdIO::PrintFmt("[mx] requires your CPU to support SSE3 instructions.\n");
			UHALT();
		}
		if(!KernelCPUID->OnboardAPIC())
		{
			StdIO::PrintFmt("[mx] requires your CPU to have an APIC chip.");
			UHALT();
		}

		// check if we have enough memory.
		if(K_SystemMemoryInBytes < 0x02000000)
		{
			StdIO::PrintFmt("[mx] requires at least 64 Megabytes (67 108 864 bytes) of memory to operate.\n");
			StdIO::PrintFmt("Only %d bytes of memory detected.\n", K_SystemMemoryInBytes);
			StdIO::PrintFmt("Install more RAM, or increase the amount of memory in your Virtual Machine.");
			UHALT();
		}


		// Copy the kernel memory map elsewhere.
		{
			uint64_t a = (uint64_t) K_MemoryMap;
			uint32_t s = K_MemoryMap->SizeOfThisStructure;
			K_MemoryMap = (MemoryMap::MemoryMap_type*) (new uint8_t[K_MemoryMap->SizeOfThisStructure + sizeof(uint64_t)]);
			Memory::CopyOverlap((void*) K_MemoryMap, (void*) a, s);
			Log("Memory map relocation complete");
		}


		PrintFmt("Loading [mx]...\n");
		Log("Initialising Kernel subsystem");
	}



	void SetupKernelThreads()
	{
		// tss on page 337 of manual. (AMD Vol. 3)
		// Setup the TSS. this will mostly be void once the scheduler initialises.
		{
			*((uint64_t*) (0x2504)) = 0x0000000000060000;
			*((uint64_t*) (0x2524)) = 0x0000000000040000;

			Log("TSS installed");
		}

		// Setup the kernel core as a thread.
		{
			// handles timer interrupt from PIT
			HardwareAbstraction::Interrupts::SetGate(0x20, (uint64_t) ProcessTimerInterrupt, 0x08, 0xEE);

			// handles YieldCPU()
			HardwareAbstraction::Interrupts::SetGate(0xF7, (uint64_t) TaskSwitcherCoOp, 0x08, 0xEE);

			// handles... syscalls, duh.
			HardwareAbstraction::Interrupts::SetGate(SyscallNumber, (uint64_t) HandleSyscall, 0x08, 0xEF);
			Log("Syscalls are available on interrupt %2x", SyscallNumber);


			using Kernel::HardwareAbstraction::Multitasking::Thread;
			using Kernel::HardwareAbstraction::Multitasking::Process;

			KernelProcess = Multitasking::CreateProcess("mx_kernel", 0x0, KernelCoreThread, 2);
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

		Log("[mx] kernel online, initialising subsystems");

		// after everything is done, make sure shit works
		{
			// uint64_t m = KernelHeap::GetFirstHeapMetadataPhysPage();
			uint64_t h = KernelHeap::GetFirstHeapPhysPage();

			// Virtual::ForceInsertALPTuple(KernelHeapMetadata, 1, m);
			Virtual::ForceInsertALPTuple(KernelHeapAddress, 1, h);
		}




		// Initialise the other, less-essential things.
		PCI::Initialise();
		Log("PCI devices enumerated");

		// needs to be up before we start doing any I/O
		IO::Initialise();
		Log("IO scheduler online");

		Storage::ATA::Initialise();
		Log("ATA driver online");

		ACPI::Initialise();
		Log("ACPI enumeration complete");

		JobDispatch::Initialise();
		Log("Central Job Dispatcher started");

		// Detect and initialise the appropriate driver for the current machine.
		{
			PCI::PCIDevice* VideoDev;
			VideoDev = PCI::GetDeviceByClassSubclass(0x3, 0xFF);

			bool found = false;
			if(VideoDev)
			{
				if(PCI::MatchVendorDevice(VideoDev, 0x15AD, 0x0405))
				{
					// VMWare's SVGA II Card. (Also available on QEMU)
					// Prefer this, since it has hardware acceleration.
					HALT("VMWare SVGA II Graphics card not implemented");
					found = true;
				}
				else if(PCI::MatchVendorDevice(VideoDev, 0x1234, 0x1111) || PCI::MatchVendorDevice(VideoDev, 0x80EE, 0xBEEF))
				{
					// QEMU, Bochs and VBox's BGA card.
					DeviceManager::AddDevice(new BochsGraphicsAdapter(VideoDev), DeviceType::FramebufferVideoCard);
					Log("Bochs Graphics Adapter (BGA) compatible card found, driver loaded");
					found = true;
				}
			}

			if(!found)
			{
				Log(1, "Warning: no compatible video card found, functionality will be limited.");
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

		PrintFmt("Initialising RTC...\n");
		Devices::RTC::Initialise(+8);
		Log("RTC Initialised");


		KernelRandom = new Random_PseudoRandom((uint32_t) Time::Now());

		// setup framebuffer
		{
			uint16_t PrefResX = 1024;
			uint16_t PrefResY = 600;

			// it better exist
			GenericVideoDevice* vd = (GenericVideoDevice*) DeviceManager::GetDevice(DeviceType::FramebufferVideoCard);

			try_again:
			if(vd)
			{

				LFBAddr = vd->GetFramebufferAddress();
				if(LFBAddr == 0)
				{
					vd = 0;
					Log(1, "Video card returned framebuffer address of 0x0, aborting initialisation");

					goto try_again;	// eewwww
				}

				LFBBufferAddr = LFBAddr;

				// Set video mode
				PrintFmt("\nInitialising Linear Framebuffer at %x...", LFBAddr);
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
			}
			Console::Initialise();
		}



		// manually jump start the vfs system
		{
			using namespace Filesystems;

			VFS::Initialise();
			Log("VFS created");

			// open fds for stdin, stdout and stderr.
			VFS::InitIO();

			// todo: detect fs type.
			{
				Devices::Storage::ATADrive* f1 = Devices::Storage::ATADrive::ATADrives->front();
				FSDriverFAT* fs = new FSDriverFAT(f1->Partitions.front());

				// mount root fs from partition 0 at /
				VFS::Mount(f1->Partitions.front(), fs, "/");
				Log("Root FS (fat32) Mounted at /");
			}

		}


		// init network stuff
		// if we have an nic, that is
		if(DeviceManager::GetDevice(DeviceType::EthernetNIC) != 0)
		{
			using namespace Network;
			ARP::Initialise();
			IP::Initialise();
			TCP::Initialise();
			UDP::Initialise();
			DHCP::Initialise();			// todo: dhcp is a little broken
			DNS::Initialise();			// todo: dns is also wonky
		}

		PS2::Initialise();
		TTY::Initialise();
		Log("StandardIO initialised");

		// Console::ClearScreen();

		#define TEST_USERSPACE_PROG		0
		#define TEST_LARGE_FILE_READ	0
		#define TEST_NETWORK_IRC		0
		#define TEST_MUTEXES			0







		Log("Initialising LaunchDaemons from /System/Library/LaunchDaemons...");

		#if TEST_USERSPACE_PROG
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
		#endif


		PrintFmt("[mx] has completed initialisation.\n");
		Log("Kernel init complete\n----------------------------\n");




		// print((astl::string("POOOOP") + "%").c_str());
		PrintF("Hello, % number %x!", "world", 504);



		#if TEST_LARGE_FILE_READ
		{
			using namespace Filesystems;

			fd_t file = OpenFile("/texts/big.txt", 0);
			assert(file > 0);

			struct stat s;
			Stat(file, &s);

			uint64_t st = 0;
			uint64_t et = 0;

			Log(3, "(%d) s.st_size: %d", file, s.st_size);

			const uint64_t blocksz = 16384;
			uint8_t* fl = new uint8_t[blocksz + 1];
			uint8_t* whole = new uint8_t[s.st_size + 1];

			uint64_t total = s.st_size;
			Log(3, "start: %d ms", st = Time::Now());

			for(uint64_t cur = 0; cur < total; )
			{
				uint64_t read = Read(file, fl, blocksz);

				memcpy(whole + cur, fl, read);
				cur += read;

				PrintFmt("\r\t\t\t\t\t\t\t\t\t\t\t\r(%02.2f%%) %d, %d/%d", (((double) cur / (double) total) * 100.0),
					read, cur, total);
			}

			Log(3, "end: %d ms", et = Time::Now());
			Log(3, "time taken: %d ms", et - st);
		}
		#endif










		#if TEST_NETWORK_IRC
		{
			using namespace Network;
			// IPv4Address fn = DNS::QueryDNSv4(rde::string("www.example.com"));
			// rde::vector<IPv4Address> fns = DNS::QueryDNSv4(rde::string("irc.freenode.net"));
			// assert(fns.size() > 0);

			IPv4Address fn;
			fn.b1 = 31;
			fn.b2 = 13;
			fn.b3 = 222;
			fn.b4 = 109;

			PrintFmt("irc.freenode.net is at %d.%d.%d.%d\n", fn.b1, fn.b2, fn.b3, fn.b4);

			fd_t thesock = OpenSocket(SocketProtocol::TCP, 0);
			ConnectSocket(thesock, fn, 6667);
			aSock = thesock;

			auto other = []()
			{
				uint8_t* output = new uint8_t[256];
				while(true)
				{
					if(GetSocketBufferFill(aSock) > 0)
					{
						memset(output, 0, 256);
						size_t read = ReadSocket(aSock, output, 256);

						output[(read == 256) ? (read - 1) : read] = 0;
						PrintFmt("%s", output);
					}
				}
			};

			Multitasking::Thread* thr = 0;
			Multitasking::AddToQueue(thr = Multitasking::CreateKernelThread(other));

			uint8_t* data = new uint8_t[256];
			memset(data, 0, 256);

			strncpy((char*) data, "NICK zhiayang|tcp\r\n", 256);
			WriteSocket(thesock, data, strlen((char*) data));
			PrintFmt("> %s", data);

			memset(data, 0, 256);
			strncpy((char*) data, "USER zhiayang 8 * : zhiayang\r\n", 256);
			WriteSocket(thesock, data, strlen((char*) data));
			PrintFmt("> %s", data);

			SLEEP(15000);

			memset(data, 0, 256);
			strncpy((char*) data, "JOIN #learnprogramming\r\n", 256);
			WriteSocket(thesock, data, strlen((char*) data));
			PrintFmt("> %s", data);


			SLEEP(1000);

			{
				static const char* msgs[] =
				{
					"PRIVMSG #learnprogramming :testing\r\n",
					// "PRIVMSG #flax-lang :another mindless crime.\r\n",
					// "PRIVMSG #flax-lang :behind the curtain,\r\n",
					// "PRIVMSG #flax-lang :in the pantomime.\r\n",
					// "PRIVMSG #flax-lang :hold the line,\r\n",
					// "PRIVMSG #flax-lang :does anybody want to take it anymore\r\n"
				};


				for(int i = 0; i < 1; i++)
				{
					memset(data, 0, 256);
					strncpy((char*) data, msgs[i], 256);
					WriteSocket(thesock, data, strlen((char*) data));
					PrintFmt("> %s", data);

					SLEEP(800);
				}
			}





			memset(data, 0, 256);
			strncpy((char*) data, "QUIT\r\n", 256);
			WriteSocket(thesock, data, strlen((char*) data));
			PrintFmt("> %s", data);

			SLEEP(500);
			Kill(thr);
			CloseSocket(thesock);
		}
		#endif







		#if TEST_MUTEXES
		{
			static Mutex* test = new Mutex;
			auto func1 = []()
			{
				PrintFmt("locking mutex\n");
				LOCK(test);

				PrintFmt("sleeping for 2 seconds\n");
				SLEEP(2000);
				PrintFmt("lock released\n");
				UNLOCK(test);
			};

			auto func2 = []()
			{
				PrintFmt("waiting for lock...");
				SLEEP(500);
				PrintFmt("trying mutex\n");

				while(!TryLockMutex(test))
					PrintFmt("x");

				PrintFmt("locked!\n");
				UNLOCK(test);
			};

			Multitasking::AddToQueue(Multitasking::CreateKernelThread(func2));
			Multitasking::AddToQueue(Multitasking::CreateKernelThread(func1));
		}
		#endif













		// kernel stops here
		// for now.

		{
			uint16_t xpos = Console::GetCharsPerLine();
			uint64_t state = 0;

			while(true)
			{
				HardwareAbstraction::Multitasking::DisableScheduler();
				uint16_t x = Console::GetCursorX();
				uint16_t y = Console::GetCursorY();

				Console::MoveCursor(xpos, 0);

				if(state % 2 == 0)		Console::PrintChar('+');
				else if(state % 2 == 1)	Console::PrintChar(' ');

				state++;
				Console::MoveCursor(x, y);

				HardwareAbstraction::Multitasking::EnableScheduler();

				SLEEP(250);
			}
		}
	}

	void Idle()
	{
		while(true)
		{
			asm volatile("pause; hlt");
			YieldCPU();
		}
	}

	void HaltSystem(const char* message, const char* filename, uint64_t line, const char* reason)
	{
		Log("System Halted: %s, %s:%d, RA(0): %x, RA(1): %x, RA(2): %x, RA(3): %x", message, filename, line,
			__builtin_return_address(0), __builtin_return_address(1), __builtin_return_address(0), __builtin_return_address(0));

		PrintFmt("\n\nFATAL ERROR: %s\nReason: %s\n%s -- Line %d (%x)\n\n[mx] has met an unresolvable error, and will now halt.", message, !reason ? "None" : reason, filename, line, __builtin_return_address(0));


		UHALT();
	}

	void HaltSystem(const char* message, const char* filename, const char* line, const char* reason)
	{
		Log("System Halted: %s, %s:%s, RA(0): %x, RA(1): %x, RA(2): %x, RA(3): %x", message, filename, line,
			__builtin_return_address(0), __builtin_return_address(1), __builtin_return_address(2), __builtin_return_address(3));

		PrintFmt("\n\nFATAL ERROR: %s\nReason: %s\n%s -- Line %s (%x)\n\n[mx] has met an unresolvable error, and will now halt.", message, !reason ? "None" : reason, filename, line, __builtin_return_address(0));

		UHALT();
	}


	void AssertCondition(const char* file, int line, const char* func, const char* expr)
	{
		(void) line;
		(void) func;
		HaltSystem("assert() Failed!", file, (uint64_t) line, expr);
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
		Log("[mx] Version %d.%d.%d r%02d -- Build %d\n", VER_MAJOR, VER_MINOR, VER_REVSN, VER_MINRV, X_BUILD_NUMBER);
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
	return KernelHeap::ReallocateChunk(ptr, size);
}








