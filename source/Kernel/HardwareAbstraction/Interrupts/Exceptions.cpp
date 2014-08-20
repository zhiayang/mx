// Interrupts.c
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


// Does good things about interrupt handling.

#include <Kernel.hpp>
#include <HardwareAbstraction/Interrupts.hpp>
#include <HardwareAbstraction/Devices/IOPort.hpp>
#include <Memory.hpp>
#include <StandardIO.hpp>
#include <List.hpp>
#include <Colours.hpp>
#include <Symbolicate.hpp>

using namespace Kernel;
using namespace Library;
using Library::LinkedList;
using namespace StandardIO;
using namespace Kernel::HardwareAbstraction::Devices;


namespace Kernel {
namespace HardwareAbstraction {
namespace Interrupts
{
	static const char* ExceptionMessages[] =
	{
		"Division By Zero",
		"Debug",
		"Non Maskable Interrupt",
		"Breakpoint",
		"Into Detected Overflow",
		"Out of Bounds",
		"Invalid Opcode",
		"No Coprocessor",

		"Double Fault",
		"Coprocessor Segment Overrun",
		"Bad TSS",
		"Segment Not Present",
		"Stack Fault",
		"General Protection Fault",
		"Page Fault",
		"Reserved",

		"x87 FPU Fault",
		"Alignment Check",
		"Machine Check",
		"SSE Fault",
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved",

		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved"
	};




	extern "C" void Fault0();
	extern "C" void Fault1();
	extern "C" void Fault2();
	extern "C" void Fault3();
	extern "C" void Fault4();
	extern "C" void Fault5();
	extern "C" void Fault6();
	extern "C" void Fault7();
	extern "C" void Fault8();
	extern "C" void Fault9();
	extern "C" void Fault10();
	extern "C" void Fault11();
	extern "C" void Fault12();
	extern "C" void Fault13();
	extern "C" void Fault14();
	extern "C" void Fault15();
	extern "C" void Fault16();
	extern "C" void Fault17();
	extern "C" void Fault18();
	extern "C" void Fault19();
	extern "C" void Fault20();
	extern "C" void Fault21();
	extern "C" void Fault22();
	extern "C" void Fault23();
	extern "C" void Fault24();
	extern "C" void Fault25();
	extern "C" void Fault26();
	extern "C" void Fault27();
	extern "C" void Fault28();
	extern "C" void Fault29();
	extern "C" void Fault30();
	extern "C" void Fault31();

	extern "C" void Interrupt0();
	extern "C" void Interrupt1();
	extern "C" void Interrupt2();
	extern "C" void Interrupt3();
	extern "C" void Interrupt4();
	extern "C" void Interrupt5();
	extern "C" void Interrupt6();
	extern "C" void Interrupt7();
	extern "C" void Interrupt8();
	extern "C" void Interrupt9();
	extern "C" void Interrupt10();
	extern "C" void Interrupt11();
	extern "C" void Interrupt12();
	extern "C" void Interrupt13();
	extern "C" void Interrupt14();
	extern "C" void Interrupt15();

	constexpr uint8_t PIC1_CMD		= 0x20;
	constexpr uint8_t PIC1_DATA		= 0x21;
	constexpr uint8_t PIC2_CMD		= 0xA0;
	constexpr uint8_t PIC2_DATA		= 0xA1;

	extern "C" void HAL_AsmLoadIDT(uint64_t);
	extern "C" void ThreadExceptionTerminate();

	struct __attribute__((packed)) IDTEntry
	{
		uint16_t base_lo;
		uint16_t sel;
		uint8_t always0_ist;
		uint8_t flags;
		uint16_t base_mid;
		uint32_t base_hi;
		uint32_t always0_1;
	};

	struct __attribute__((packed)) IDTPointer
	{
		uint16_t limit;
		uint64_t base;
	};




	static IDTEntry idt[256];
	static IDTPointer idtp;
	extern LinkedList<IRQHandlerPlugList>* IRQHandlerList;


	void SetGate(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags)
	{
		// The interrupt routine's base address
		idt[num].base_lo = (base & 0xFFFF);
		idt[num].base_mid = (base >> 16) & 0xFFFF;
		idt[num].base_hi = (base >> 32) & 0xFFFFFFFF;

		// The segment or 'selector' that this IDT entry will use
		// is set here, along with any access flags

		idt[num].sel = sel;
		if(num < 32)
			idt[num].always0_ist = 0x0;

		else
			idt[num].always0_ist = 0x0;

		idt[num].always0_1 = 0;
		idt[num].flags = flags;
	}

	// Installs the IDT
	void Initialise()
	{
		// Sets the special IDT pointer up, just like in 'gdt.c'
		idtp.limit = (sizeof(IDTEntry) * 256) - 1;
		idtp.base = (uintptr_t)&idt;

		// Clear out the entire IDT, initializing it to zeros
		Memory::Set(&idt, 0, sizeof(IDTEntry) * 256);

		// Add any new ISRs to the IDT here using idt_set_gate

		InstallDefaultHandlers();


		// Points the processor's internal register to the new IDT
		HAL_AsmLoadIDT((uint64_t)&idtp);

		IRQHandlerList = new LinkedList<IRQHandlerPlugList>();
	}

	void InstallDefaultHandlers()
	{
		SetGate(0, (uint64_t) Fault0, 0x08, 0x8E);
		SetGate(1, (uint64_t) Fault1, 0x08, 0x8E);
		SetGate(2, (uint64_t) Fault2, 0x08, 0x8E);
		SetGate(3, (uint64_t) Fault3, 0x08, 0x8E);
		SetGate(4, (uint64_t) Fault4, 0x08, 0x8E);
		SetGate(5, (uint64_t) Fault5, 0x08, 0x8E);
		SetGate(6, (uint64_t) Fault6, 0x08, 0x8E);
		SetGate(7, (uint64_t) Fault7, 0x08, 0x8E);

		SetGate(8, (uint64_t) Fault8, 0x08, 0x8E);
		SetGate(9, (uint64_t) Fault9, 0x08, 0x8E);
		SetGate(10, (uint64_t) Fault10, 0x08, 0x8E);
		SetGate(11, (uint64_t) Fault11, 0x08, 0x8E);
		SetGate(12, (uint64_t) Fault12, 0x08, 0x8E);
		SetGate(13, (uint64_t) Fault13, 0x08, 0x8E);
		SetGate(14, (uint64_t) Fault14, 0x08, 0x8E);
		SetGate(15, (uint64_t) Fault15, 0x08, 0x8E);

		SetGate(16, (uint64_t) Fault16, 0x08, 0x8E);
		SetGate(17, (uint64_t) Fault17, 0x08, 0x8E);
		SetGate(18, (uint64_t) Fault18, 0x08, 0x8E);
		SetGate(19, (uint64_t) Fault19, 0x08, 0x8E);
		SetGate(20, (uint64_t) Fault20, 0x08, 0x8E);
		SetGate(21, (uint64_t) Fault21, 0x08, 0x8E);
		SetGate(22, (uint64_t) Fault22, 0x08, 0x8E);
		SetGate(23, (uint64_t) Fault23, 0x08, 0x8E);

		SetGate(24, (uint64_t) Fault24, 0x08, 0x8E);
		SetGate(25, (uint64_t) Fault25, 0x08, 0x8E);
		SetGate(26, (uint64_t) Fault26, 0x08, 0x8E);
		SetGate(27, (uint64_t) Fault27, 0x08, 0x8E);
		SetGate(28, (uint64_t) Fault28, 0x08, 0x8E);
		SetGate(29, (uint64_t) Fault29, 0x08, 0x8E);
		SetGate(30, (uint64_t) Fault30, 0x08, 0x8E);
		SetGate(31, (uint64_t) Fault31, 0x08, 0x8E);


		// Remap the IRQs from 0 - 7 -> 8 - 15 to 32-47
		IOPort::WriteByte(PIC1_CMD, 0x11);
		IOPort::WriteByte(PIC2_CMD, 0x11);
		IOPort::WriteByte(PIC1_DATA, 0x20);
		IOPort::WriteByte(PIC2_DATA, 0x28);
		IOPort::WriteByte(PIC1_DATA, 0x04);
		IOPort::WriteByte(PIC2_DATA, 0x02);
		IOPort::WriteByte(PIC1_DATA, 0x01);
		IOPort::WriteByte(PIC2_DATA, 0x01);
		IOPort::WriteByte(PIC1_DATA, 0x0);
		IOPort::WriteByte(PIC2_DATA, 0x0);

		SetGate(32, (uint64_t) Interrupt0, 0x08, 0x8E);
		SetGate(33, (uint64_t) Interrupt1, 0x08, 0x8E);
		SetGate(34, (uint64_t) Interrupt2, 0x08, 0x8E);
		SetGate(35, (uint64_t) Interrupt3, 0x08, 0x8E);
		SetGate(36, (uint64_t) Interrupt4, 0x08, 0x8E);
		SetGate(37, (uint64_t) Interrupt5, 0x08, 0x8E);
		SetGate(38, (uint64_t) Interrupt6, 0x08, 0x8E);
		SetGate(39, (uint64_t) Interrupt7, 0x08, 0x8E);
		SetGate(40, (uint64_t) Interrupt8, 0x08, 0x8E);
		SetGate(41, (uint64_t) Interrupt9, 0x08, 0x8E);
		SetGate(42, (uint64_t) Interrupt10, 0x08, 0x8E);
		SetGate(43, (uint64_t) Interrupt11, 0x08, 0x8E);
		SetGate(44, (uint64_t) Interrupt12, 0x08, 0x8E);
		SetGate(45, (uint64_t) Interrupt13, 0x08, 0x8E);
		SetGate(46, (uint64_t) Interrupt14, 0x08, 0x8E);
		SetGate(47, (uint64_t) Interrupt15, 0x08, 0x8E);
	}

	void MaskInterrupt(uint8_t interrupt)
	{
		uint16_t port = 0;
		uint8_t value = 0;

		if(interrupt < 8)
		{
			port = PIC1_DATA;
		}
		else
		{
			port = PIC2_DATA;
			interrupt -= 8;
		}

		value = IOPort::ReadByte(port) | (uint8_t)(1 << interrupt);
		IOPort::WriteByte(port, value);
	}

	void UnmaskInterrupt(uint8_t interrupt)
	{
		uint16_t port = 0;
		uint8_t value = 0;

		if(interrupt < 8)
		{
			port = PIC1_DATA;
		}
		else
		{
			port = PIC2_DATA;
			interrupt -= 8;
		}

		value = IOPort::ReadByte(port) & ~((uint8_t)(1 << interrupt));
		IOPort::WriteByte(port, value);
	}














	void PrintRegisterDump(RegisterStruct_type* r)
	{
		PrintFormatted("%wRegisters:\n", Colours::Blue);


		PrintFormatted("rax:\t%w%#16.8x%r\trbx:\t%w%#16.8x\n", Colours::Green, r->rax, Colours::Green, r->rbx);
		PrintFormatted("rcx:\t%w%#16.8x%r\trdx:\t%w%#16.8x\n", Colours::Green, r->rcx, Colours::Green, r->rdx);
		PrintFormatted("r08:\t%w%#16.8x%r\tr09:\t%w%#16.8x\n", Colours::Green, r->r8, Colours::Green, r->r9);
		PrintFormatted("r10:\t%w%#16.8x%r\tr11:\t%w%#16.8x\n", Colours::Green, r->r10, Colours::Green, r->r11);
		PrintFormatted("r12:\t%w%#16.8x%r\tr13:\t%w%#16.8x\n", Colours::Green, r->r12, Colours::Green, r->r13);
		PrintFormatted("r14:\t%w%#16.8x%r\tr15:\t%w%#16.8x\n", Colours::Green, r->r14, Colours::Green, r->r15);
		PrintFormatted("rdi:\t%w%#16.8x%r\trsi:\t%w%#16.8x\n", Colours::Cyan, r->rdi, Colours::Cyan, r->rsi);
		PrintFormatted("rbp:\t%w%#16.8x%r\trsp:\t%w%#16.8x\n", Colours::Yellow, r->rbp, Colours::Yellow, r->rsp);
		PrintFormatted("rip:\t%w%#16.8x%r\tcs: \t%w%#16.8x\n", Colours::Cyan, r->rip, Colours::Silver, r->cs);
		PrintFormatted("ss: \t%w%#16.8x%r\tu-rsp:\t%w%#16.8x\n", Colours::Silver, r->ss, Colours::Blue, r->useresp);
		PrintFormatted("rflags:\t%w%#16.8x%r\tcr2:\t%w%#16.8x\n", Colours::Magenta, r->rflags, Colours::Red, r->cr2);
	}






	extern "C" void ExceptionHandler_C(RegisterStruct_type* r)
	{
		// asm volatile("xchg %bx, %bx");

		uint64_t cr2;
		uint64_t cr3;
		asm volatile("mov %%cr2, %0" : "=r" (cr2));
		asm volatile("mov %%cr3, %0" : "=r" (cr3));
		Log(1, "%s Exception: RIP: %x, Error Code: %x, CR3: %x, CR2: %x, ErrCode: %x, TID: %d", ExceptionMessages[r->InterruptID], r->rip, r->ErrorCode, cr3, r->cr2, r->ErrorCode, Multitasking::GetCurrentThread()->ThreadID);

		// // fetch faulting address.
		// // risky.
		// {
		// 	Symbolicate::LineNamePair* pair = Symbolicate::ResolveAddress(r->rip);
		// 	if(pair)
		// 	{
		// 		Log("Symbolicated location: %s : %d", pair->name->CString(), pair->line);
		// 	}
		// }


		if(Multitasking::GetCurrentThread()->State & 0x1)
		{
			Log(1, "Terminated thread %d belonging to parent %s, for exception: %s", Multitasking::GetCurrentThread()->ThreadID, Multitasking::GetCurrentThread()->Parent->Name, ExceptionMessages[r->InterruptID]);


			Multitasking::Thread* thr = Multitasking::GetCurrentThread();

			// copy over the crash state.
			thr->CrashState.rax = r->rax;
			thr->CrashState.rbx = r->rbx;
			thr->CrashState.rcx = r->rcx;
			thr->CrashState.rdx = r->rdx;

			thr->CrashState.r8 = r->r8;
			thr->CrashState.r9 = r->r9;
			thr->CrashState.r10 = r->r10;
			thr->CrashState.r11 = r->r11;
			thr->CrashState.r12 = r->r12;
			thr->CrashState.r13 = r->r13;
			thr->CrashState.r14 = r->r14;
			thr->CrashState.r15 = r->r15;

			thr->CrashState.rbp = r->rbp;
			thr->CrashState.rsi = r->rsi;
			thr->CrashState.rdi = r->rdi;


			r->rip = (uint64_t) Multitasking::TerminateCurrentThread;
			return;
		}


		PrintFormatted("\n\n\n%wCPU Exception: %w%s%w; Error Code: %w%x", Colours::Yellow, Colours::Red, ExceptionMessages[r->InterruptID], Colours::Yellow, Colours::Red, r->ErrorCode);



		PrintFormatted("\nOrion-X4 has met an unresolvable error, and will now halt.\n");


		PrintFormatted("Debug Information:\n\n");

		if(r->InterruptID == 14)
		{
			// The error code gives us details of what happened.
			uint8_t PageNotPresent	= !(r->ErrorCode & 0x1);		// Page not present
			uint8_t PageAccess		= r->ErrorCode & 0x2;		// Write operation?
			uint8_t PageSupervisor		= r->ErrorCode & 0x4;		// Processor was in user-mode?
			uint8_t PageReservedBits	= r->ErrorCode & 0x8;		// Overwritten CPU-reserved bits of page entry?
			uint8_t PageInstructionFetch	= r->ErrorCode & 0x10;		// Caused by an instruction fetch?


			PrintFormatted("%wPage Fault Error Codes:\n", Colours::Green);
			PrintFormatted("\tCR2: %x, CR3: %x\n", cr2, cr3);


			if(PageNotPresent)
				PrintFormatted("\tPage not present\n");

			if(PageAccess)
				PrintFormatted("\tWriting to read-only page\n");

			if(PageSupervisor)
				PrintFormatted("\tAccessing Supervisor page from User Mode\n");

			if(PageReservedBits)
				PrintFormatted("\tOverwritten reserved bits\n");

			if(PageInstructionFetch)
				PrintFormatted("\tPF Caused by instruction fetch\n");
		}



		PrintFormatted("\n");


		// if(DEBUG && !SKIPREGDUMP)
			// PrintRegisterDump(r);

		UHALT();
	}
}

}
}
















