// userspace/Syscall.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "../HeaderFiles/SystemCall.hpp"

namespace Library
{
	namespace SystemCall
	{
		// void PrintChar(uint8_t ch)
		// {
		// 	asm volatile("mov $0x1, %%r10; mov %[c], %%rdi; int $0xF8" :: [c]"g"((uint64_t) ch) : "%r10");
		// }

		// void* AllocateChunk(uint64_t sz)
		// {
		// 	void* ret = 0;
		// 	asm volatile("mov $0x3, %%r10; mov %[c], %%rdi; int $0xF8; mov %%rax, %[re]" : [re]"=r"(ret) : [c]"r"(sz) : "%r10");
		// 	return ret;
		// }

		// void FreeChunk(void* p)
		// {
		// 	asm volatile("mov $0x4, %%r10; mov %[c], %%rdi; int $0xF8" :: [c]"g"(p) : "%r10");
		// }

		// uint64_t QueryChunkSize(void* p)
		// {
		// 	uint64_t ret = 0;
		// 	asm volatile("mov $0x5, %%r10; mov %[c], %%rdi; int $0xF8; mov %%rax, %[re]" : [re]"=r"(ret) : [c]"r"(p) : "%r10");
		// 	return ret;
		// }

		// 3, 4 and 5 were for malloc, free and query.
		// stupid.

		// 7 and 8 were meant for getting CR3 and executing invlpg.
		// obvious security hole there.

		// void InstallIRQHandlerWithRegs(uint64_t irq, uint64_t handleraddr)
		// {
		// 	asm volatile("mov $0x9, %%r10; mov %[c], %%rdi; mov %[h], %%rsi; int $0xF8" :: [c]"r"(irq), [h]"r"(handleraddr) : "%r10");
		// }

		// void InstallIRQHandler(uint64_t irq, uint64_t handleraddr)
		// {
		// 	asm volatile("mov $0xA, %%r10; mov %[c], %%rdi; mov %[h], %%rsi; int $0xF8" :: [c]"r"(irq), [h]"r"(handleraddr) : "%r10");
		// }


		// uint8_t ReadKeyboardKey()
		// {
		// 	uint64_t ret = 0;
		// 	asm volatile("mov $0xB, %%r10; int $0xF8; mov %%rax, %[re]" : [re]"=r"(ret) :: "%r10");
		// 	return (uint8_t) ret;
		// }

		// 11 was for readkeyboardkey.

		// void CreateThread(void (*t)())
		// {
		// 	asm volatile("mov $0xC, %%r10; mov %[c], %%rdi; int $0xF8" :: [c]"r"(t) : "%r10");
		// }

		// uint64_t GetFramebufferAddress()
		// {
		// 	uint64_t ret = 0;
		// 	asm volatile("mov $0xD, %%r10; int $0xF8; mov %%rax, %[re]" : [re]"=r"(ret) :: "%r10");
		// 	return ret;
		// }

		// void SetConsoleBackColour(uint32_t c)
		// {
		// 	uint64_t cl = (uint64_t) c;
		// 	asm volatile("mov $0xE, %%r10; mov %[c], %%rdi; int $0xF8" :: [c]"r"(cl) : "%r10");
		// }

		// uint64_t GetFramebufferResX()
		// {
		// 	uint64_t ret = 0;
		// 	asm volatile("mov $0xF, %%r10; int $0xF8; mov %%rax, %[re]" : [re]"=r"(ret) :: "%r10");
		// 	return ret;
		// }

		// uint64_t GetFramebufferResY()
		// {
		// 	uint64_t ret = 0;
		// 	asm volatile("mov $0x10, %%r10; int $0xF8; mov %%rax, %[re]" : [re]"=r"(ret) :: "%r10");
		// 	return ret;
		// }

		// void MapVirtualAddress(uint64_t v, uint64_t p, uint64_t f)
		// {
		// 	asm volatile("mov $0x11, %%r10; mov %[vt], %%rdi; mov %[pt], %%rsi; mov %[ft], %%rdx; int $0xF8" :: [vt]"r"(v), [pt]"r"(p), [ft]"r"(f) : "%r10");
		// }

		// void UnmapVirtualAddress(uint64_t v)
		// {
		// 	asm volatile("mov $0x12, %%r10; mov %[vt], %%rdi; int $0xF8" :: [vt]"r"(v) : "%r10");
		// }


		// uint64_t AllocatePage()
		// {
		// 	uint64_t ret = 0;
		// 	asm volatile("mov $0x13, %%r10; int $0xF8; mov %%rax, %[r]" : [r]"=r"(ret) :: "%r10");
		// 	return ret;
		// }

		// void FreePage(uint64_t p)
		// {
		// 	asm volatile("mov $0x14, %%r10; mov %[vt], %%rdi; int $0xF8" :: [vt]"r"(p) : "%r10");
		// }

		// uint32_t GetBackgroundColour()
		// {
		// 	uint64_t ret = 0;
		// 	asm volatile("mov $0x15, %%r10; int $0xF8; mov %%rax, %[r]" : [r]"=r"(ret) :: "%r10");
		// 	return (uint32_t) ret;
		// }

		// uint32_t GetTextColour()
		// {
		// 	uint64_t ret = 0;
		// 	asm volatile("mov $0x16, %%r10; int $0xF8; mov %%rax, %[r]" : [r]"=r"(ret) :: "%r10");
		// 	return (uint32_t) ret;
		// }

		// void SetTextColour(uint32_t c)
		// {
		// 	uint64_t cl = (uint64_t) c;
		// 	asm volatile("mov $0x17, %%r10; mov %[c], %%rdi; int $0xF8" :: [c]"r"(cl) : "%r10");
		// }

		// uint64_t OpenFile(const char* path, uint8_t mode)
		// {
		// 	uint64_t ret = 0;
		// 	asm volatile("mov $0x18, %%r10; mov %[p], %%rdi; mov %[m], %%rsi; int $0xF8; mov %%rax, %[r]" : [r]"=r"(ret) : [p]"r"(path), [m]"r"((uint64_t) mode) : "%r10", "%rdi", "%rsi");
		// 	return ret;
		// }

		// void CloseFile(uint64_t fd)
		// {
		// 	// asm volatile("mov $0x19, %%r10; mov %[f], %%rdi; int $0xF8" :: [f]"r"(fd) : "%r10", "%rdi");
		// 	CloseAny(fd);
		// }


		// uint8_t* ReadFile(uint64_t fd, uint8_t* buffer, uint64_t length)
		// {
		// 	// uint64_t ret = 0;
		// 	// asm volatile("mov $0x1A, %%r10; mov %[f], %%rdi; mov %[b], %%rsi; mov %[l], %%rdx; int $0xF8; mov %%rax, %[r]" : [r]"=r"(ret) : [f]"r"(fd), [b]"r"((uint64_t) buffer), [l]"r"(length) : "%r10", "%rdi", "%rsi", "%rdx");

		// 	// return (uint8_t*) ret;

		// 	ReadFromAny(fd, buffer, length);
		// 	return buffer;
		// }

		// uint64_t OpenFolder(const char* path)
		// {
		// 	uint64_t ret = 0;
		// 	asm volatile("mov $0x1C, %%r10; mov %[p], %%rdi; int $0xF8; mov %%rax, %[r]" : [r]"=r"(ret) : [p]"r"(path) : "%r10", "%rdi");
		// 	return ret;
		// }

		// void CloseFolder(uint64_t fd)
		// {
		// 	// asm volatile("mov $0x1D, %%r10; mov %[f], %%rdi; int $0xF8" :: [f]"r"(fd) : "%r10", "%rdi");
		// 	CloseAny(fd);
		// }

		// Library::LinkedList<char>* ListObjects(uint64_t fd, Library::LinkedList<char>* output, uint64_t* items)
		// {
		// 	uint64_t ret = 0;
		// 	asm volatile("mov $0x1E, %%r10; mov %[f], %%rdi; mov %[o], %%rsi; mov %[i], %%rdx; int $0xF8; mov %%rax, %[r]" : [r]"=r"(ret) : [f]"r"(fd), [o]"r"((uint64_t) output), [i]"r"((uint64_t) items) : "%r10", "%rdi", "%rsi", "%rdx");

		// 	return (Library::LinkedList<char>*) ret;
		// }

		// uint64_t GetFileSize(uint64_t fd)
		// {
		// 	uint64_t ret = 0;
		// 	asm volatile("mov $0x1F, %%r10; mov %[f], %%rdi; int $0xF8; mov %%rax, %[r]" : [r]"=r"(ret) : [f]"r"(fd) : "%r10", "%rdi");
		// 	return ret;
		// }

		// bool CheckFileExistence(const char* path)
		// {
		// 	uint64_t ret = 0;
		// 	asm volatile("mov $0x20, %%r10; mov %[p], %%rdi; int $0xF8; mov %%rax, %[r]" : [r]"=r"(ret) : [p]"r"(path) : "%r10", "%rdi");
		// 	return (bool) ret;
		// }

		// bool CheckFolderExistence(const char* path)
		// {
		// 	uint64_t ret = 0;
		// 	asm volatile("mov $0x21, %%r10; mov %[p], %%rdi; int $0xF8; mov %%rax, %[r]" : [r]"=r"(ret) : [p]"r"(path) : "%r10", "%rdi");
		// 	return (bool) ret;
		// }



		// void SendMemoryMessageToProcess(uint64_t TargetPID, MessageTypes Type, void* Data, uint64_t DataSize, void (*Callback)())
		// {
		// 	asm volatile("mov $0x22, %%r10; mov %[tpid], %%rdi; mov %[tp], %%rsi; mov %[dat], %%rdx; mov %[sz], %%rcx; mov %[c], %%r8; int $0xF8" :: [tpid]"r"(TargetPID), [tp]"r"((uint64_t) Type), [dat]"r"(Data), [sz]"r"(DataSize), [c]"r"((uint64_t) Callback) : "%r10", "%rdi", "%rsi", "%rdx", "%rcx", "%r8");
		// }

		// // 0x23, send memory message

		// void SendSimpleMessageToProcess(uint64_t TargetPID, MessageTypes typ, uint64_t D1, uint64_t D2, uint64_t D3, void (*cb)())
		// {
		// 	asm volatile("mov $0x24, %%r10; mov %[tpid], %%rdi; mov %[tp], %%rsi; mov %[d1], %%rdx; mov %[d2], %%rcx; mov %[d3], %%r8; mov %[c], %%r9; int $0xF8" :: [tpid]"r"(TargetPID), [tp]"r"((uint64_t) typ), [d1]"r"(D1), [d2]"r"(D2), [d3]"r"(D3), [c]"r"((uint64_t) cb) : "%r10", "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9");
		// }

		// // 0x25, send memory message to process
		// // 0x26, get memory message

		// SimpleMessage* GetSimpleMessage()
		// {
		// 	uint64_t ret = 0;
		// 	asm volatile("mov $0x27, %%r10; int $0xF8; mov %%rax, %[r]" : [r]"=r"(ret) :: "%r10");
		// 	return (SimpleMessage*) ret;
		// }

		// void Yield()
		// {
		// 	asm volatile("int $0xF7");
		// }

		// void Block()
		// {
		// 	asm volatile("mov $0x29, %%r10; int $0xF8" ::: "%r10");
		// 	Yield();
		// }

		// void ClearScreen()
		// {
		// 	asm volatile("mov $0x2A, %%r10; int $0xF8" ::: "%r10");
		// }

		// void SpawnProcess(const char* path, const char* name)
		// {
		// 	asm volatile("mov $0x2B, %%r10; mov %[ex], %%rdi; mov %[pn], %%rsi; int $0xF8" :: [ex]"r"(path), [pn]"r"(name): "%r10", "%rdi", "%rsi");
		// }

		// uint64_t GetCursorX()
		// {
		// 	uint64_t ret = 0;
		// 	asm volatile("mov $0x2C, %%r10; int $0xF8; mov %%rax, %[r]" : [r]"=r"(ret) :: "%r10");
		// 	return ret;
		// }

		// uint64_t GetCursorY()
		// {
		// 	uint64_t ret = 0;
		// 	asm volatile("mov $0x2D, %%r10; int $0xF8; mov %%rax, %[r]" : [r]"=r"(ret) :: "%r10");
		// 	return ret;
		// }

		// void SetCursorPos(uint64_t x, uint64_t y)
		// {
		// 	asm volatile("mov $0x2E, %%r10; mov %[c], %%rdi; mov %[h], %%rsi; int $0xF8" :: [c]"r"(x), [h]"r"(y) : "%r10", "%rdi", "%rsi");
		// }

		// uint64_t GetCharWidth()
		// {
		// 	uint64_t ret = 0;
		// 	asm volatile("mov $0x2F, %%r10; int $0xF8; mov %%rax, %[r]" : [r]"=r"(ret) :: "%r10");
		// 	return ret;
		// }

		// uint64_t GetCharHeight()
		// {
		// 	uint64_t ret = 0;
		// 	asm volatile("mov $0x30, %%r10; int $0xF8; mov %%rax, %[r]" : [r]"=r"(ret) :: "%r10");
		// 	return ret;
		// }

		// void PutPixelAtXY(uint64_t x, uint64_t y, uint64_t Colour)
		// {
		// 	asm volatile("mov $0x31, %%r10; mov %[x], %%rdi; mov %[y], %%rsi; mov %[c], %%rdx; int $0xF8" :: [x]"r"(x), [y]"r"(y), [c]"r"(Colour) : "%r10", "%rdi", "%rsi", "%rdx");
		// }

		// void PutCharNoMove(uint8_t ch)
		// {
		// 	asm volatile("mov $0x32, %%r10; mov %[c], %%rdi; int $0xF8" :: [c]"g"((uint64_t) ch) : "%r10");
		// }

		// void OpenIPCSocket(uint64_t pid, uint64_t size)
		// {
		// 	asm volatile("mov $0x33, %%r10; mov %[c], %%rdi; mov %[h], %%rsi; int $0xF8" :: [c]"r"(pid), [h]"r"(size) : "%r10", "%rdi", "%rsi");
		// }

		// void CloseIPCSocket(uint64_t sd)
		// {
		// 	// asm volatile("mov $0x34, %%r10; mov %[c], %%rdi; int $0xF8" :: [c]"g"(sd) : "%r10");

		// 	// prefer closeany
		// 	CloseAny(sd);
		// }

		// uint8_t* ReadIPCSocket(uint64_t sd, uint8_t* buffer, uint64_t length)
		// {
		// 	// uint64_t ret = 0;
		// 	// asm volatile("mov $0x35, %%r10; mov %[f], %%rdi; mov %[b], %%rsi; mov %[l], %%rdx; int $0xF8; mov %%rax, %[r]" : [r]"=r"(ret) : [f]"r"(sd), [b]"r"((uint64_t) buffer), [l]"r"(length) : "%r10", "%rdi", "%rsi", "%rdx");

		// 	// return (uint8_t*) ret;

		// 	// deprecated.
		// 	// call readany directly.

		// 	ReadFromAny(sd, buffer, length);
		// 	return buffer;
		// }

		// uint8_t* WriteIPCSocket(uint64_t sd, uint8_t* buffer, uint64_t length)
		// {
		// 	// uint64_t ret = 0;
		// 	// asm volatile("mov $0x36, %%r10; mov %[f], %%rdi; mov %[b], %%rsi; mov %[l], %%rdx; int $0xF8; mov %%rax, %[r]" : [r]"=r"(ret) : [f]"r"(sd), [b]"r"((uint64_t) buffer), [l]"r"(length) : "%r10", "%rdi", "%rsi", "%rdx");

		// 	// return (uint8_t*) ret;


		// 	WriteToAny(sd, buffer, length);
		// 	return buffer;
		// }



		// uint64_t MMap_Anonymous(uint64_t addr, uint64_t size, uint64_t prot, uint64_t flags)
		// {
		// 	uint64_t ret = 0;
		// 	asm volatile("mov $0x37, %%r10; mov %[ad], %%rdi; mov %[sz], %%rsi; mov %[pt], %%rdx; mov %[fl], %%rcx; int $0xF8; mov %%rax, %[r]"
		// 		: [r]"=r"(ret) : [ad]"r"(addr), [sz]"r"((uint64_t) size), [pt]"r"(prot), [fl]"r"(flags) : "%r10", "%rdi", "%rsi", "%rdx", "%rcx");

		// 	return ret;
		// }

		// uint64_t MMap_File(uint64_t fd, uint64_t length, uint64_t offset, uint64_t prot, uint64_t flags)
		// {
		// 	(void) fd;
		// 	(void) length;
		// 	(void) offset;
		// 	(void) prot;
		// 	(void) flags;

		// 	return 0;
		// }

		// uint64_t ReadFromAny(uint64_t sd, uint8_t* buffer, uint64_t length)
		// {
		// 	uint64_t ret = 0;
		// 	asm volatile("mov $0x39, %%r10; mov %[f], %%rdi; mov %[b], %%rsi; mov %[l], %%rdx; int $0xF8; mov %%rax, %[r]" : [r]"=r"(ret) : [f]"r"(sd), [b]"r"((uint64_t) buffer), [l]"r"(length) : "%r10", "%rdi", "%rsi", "%rdx");

		// 	return ret;
		// }

		// uint64_t WriteToAny(uint64_t sd, uint8_t* buffer, uint64_t length)
		// {
		// 	uint64_t ret = 0;
		// 	asm volatile("mov $0x3A, %%r10; mov %[f], %%rdi; mov %[b], %%rsi; mov %[l], %%rdx; int $0xF8; mov %%rax, %[r]" : [r]"=r"(ret) : [f]"r"(sd), [b]"r"((uint64_t) buffer), [l]"r"(length) : "%r10", "%rdi", "%rsi", "%rdx");

		// 	return ret;
		// }

		// uint64_t OpenAny(const char* path, uint64_t flags)
		// {
		// 	uint64_t ret = 0;
		// 	asm volatile("mov $0x3B, %%r10; mov %[p], %%rdi; mov %[m], %%rsi; int $0xF8; mov %%rax, %[r]" : [r]"=r"(ret) : [p]"r"(path), [m]"r"(flags) : "%r10", "%rdi", "%rsi");
		// 	return ret;
		// }

		// void CloseAny(uint64_t fd)
		// {
		// 	asm volatile("mov $0x3C, %%r10; mov %[f], %%rdi; int $0xF8" :: [f]"r"(fd) : "%r10", "%rdi");
		// }
	}
}









