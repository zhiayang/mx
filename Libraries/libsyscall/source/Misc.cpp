// Misc.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <stdint.h>
namespace Library
{
	namespace SystemCall
	{
		uint64_t GetFramebufferAddress()
		{
			uint64_t ret = 0;
			asm volatile("mov $0xD, %%r10; int $0xF8; mov %%rax, %[re]" : [re]"=r"(ret) :: "%r10");
			return ret;
		}

		void SetConsoleBackColour(uint32_t c)
		{
			uint64_t cl = (uint64_t) c;
			asm volatile("mov $0xE, %%r10; mov %[c], %%rdi; int $0xF8" :: [c]"r"(cl) : "%r10");
		}

		uint64_t GetFramebufferResX()
		{
			uint64_t ret = 0;
			asm volatile("mov $0xF, %%r10; int $0xF8; mov %%rax, %[re]" : [re]"=r"(ret) :: "%r10");
			return ret;
		}

		uint64_t GetFramebufferResY()
		{
			uint64_t ret = 0;
			asm volatile("mov $0x10, %%r10; int $0xF8; mov %%rax, %[re]" : [re]"=r"(ret) :: "%r10");
			return ret;
		}

		uint32_t GetBackgroundColour()
		{
			uint64_t ret = 0;
			asm volatile("mov $0x15, %%r10; int $0xF8; mov %%rax, %[r]" : [r]"=r"(ret) :: "%r10");
			return (uint32_t) ret;
		}

		uint32_t GetTextColour()
		{
			uint64_t ret = 0;
			asm volatile("mov $0x16, %%r10; int $0xF8; mov %%rax, %[r]" : [r]"=r"(ret) :: "%r10");
			return (uint32_t) ret;
		}

		void SetTextColour(uint32_t c)
		{
			uint64_t cl = (uint64_t) c;
			asm volatile("mov $0x17, %%r10; mov %[c], %%rdi; int $0xF8" :: [c]"r"(cl) : "%r10");
		}

		void ClearScreen()
		{
			asm volatile("mov $0x2A, %%r10; int $0xF8" ::: "%r10");
		}

		uint64_t GetCursorX()
		{
			uint64_t ret = 0;
			asm volatile("mov $0x2C, %%r10; int $0xF8; mov %%rax, %[r]" : [r]"=r"(ret) :: "%r10");
			return ret;
		}

		uint64_t GetCursorY()
		{
			uint64_t ret = 0;
			asm volatile("mov $0x2D, %%r10; int $0xF8; mov %%rax, %[r]" : [r]"=r"(ret) :: "%r10");
			return ret;
		}

		void SetCursorPos(uint64_t x, uint64_t y)
		{
			asm volatile("mov $0x2E, %%r10; mov %[c], %%rdi; mov %[h], %%rsi; int $0xF8" :: [c]"r"(x), [h]"r"(y) : "%r10", "%rdi", "%rsi");
		}

		uint64_t GetCharWidth()
		{
			uint64_t ret = 0;
			asm volatile("mov $0x2F, %%r10; int $0xF8; mov %%rax, %[r]" : [r]"=r"(ret) :: "%r10");
			return ret;
		}

		uint64_t GetCharHeight()
		{
			uint64_t ret = 0;
			asm volatile("mov $0x30, %%r10; int $0xF8; mov %%rax, %[r]" : [r]"=r"(ret) :: "%r10");
			return ret;
		}

		void PutPixelAtXY(uint64_t x, uint64_t y, uint64_t Colour)
		{
			asm volatile("mov $0x31, %%r10; mov %[x], %%rdi; mov %[y], %%rsi; mov %[c], %%rdx; int $0xF8" :: [x]"r"(x), [y]"r"(y), [c]"r"(Colour) : "%r10", "%rdi", "%rsi", "%rdx");
		}

		void PutCharNoMove(uint8_t ch)
		{
			asm volatile("mov $0x32, %%r10; mov %[c], %%rdi; int $0xF8" :: [c]"g"((uint64_t) ch) : "%r10");
		}
	}
}








