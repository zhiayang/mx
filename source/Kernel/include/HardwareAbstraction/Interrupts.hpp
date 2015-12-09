// Interrupts.hpp
// Copyright (c) 2013 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#pragma once
#include <stdint.h>
#include <HardwareAbstraction/Multitasking.hpp>

namespace Kernel {
namespace HardwareAbstraction
{
	namespace Interrupts
	{
		struct RegisterStruct_type
		{
			uint64_t cr2;
			uint64_t rsp;
			uint64_t rdi, rsi, rbp;
			uint64_t rax, rbx, rcx, rdx, r8, r9, r10, r11, r12, r13, r14, r15;

			uint64_t InterruptID, ErrorCode;
			uint64_t rip, cs, rflags, useresp, ss;
		};

		class IRQHandlerPlug
		{
			public:
				IRQHandlerPlug(void (*x)(void*), void* a)
				{
					 this->handle = x;
					 this->arg = a;
				}

				void (*handle)(void* arg);
				void* arg;
		};

		class IRQHandlerPlugList
		{
			public:
				IRQHandlerPlugList(uint64_t n)
				{
					this->IRQNum = n;
				}

				uint64_t IRQNum;
				rde::vector<IRQHandlerPlug*> HandlerList;
		};


		extern rde::vector<IRQHandlerPlugList*>* IRQHandlerList;

		void SetGate(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags);
		void Initialise();
		void InstallDefaultHandlers();
		void InstallIRQHandler(uint64_t irq, void(*handler)(void*), void* arg = 0);
		void UninstallIRQHandler(uint64_t irq);

		void MaskInterrupt(uint8_t interrupt);
		void UnmaskInterrupt(uint8_t interrupt);
	}
}
}
