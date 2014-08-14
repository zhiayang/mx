// // Bootscreen.cpp
// // Copyright(c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// // Licensed under the Apache License Version 2.0.

// #include <Kernel.hpp>
// #include <Console.hpp>
// #include <HardwareAbstraction/VideoOutput.hpp>
// #include <Bootscreen.hpp>
// #include <BitmapLibrary/BitmapLibrary.hpp>
// #include <Memory.hpp>
// #include <String.hpp>
// #include <StandardIO.hpp>
// #include <math.h>

// using namespace Library;
// using namespace Kernel::HardwareAbstraction::VideoOutput;
// using namespace Kernel::HardwareAbstraction;

// namespace Kernel {
// namespace Bootscreen
// {
// 	void Initialise()
// 	{
// 		using Kernel::HardwareAbstraction::Filesystems::VFS::File;

// 		File* bmpimg = new File("/System/Library/Resources/logo@0.5x.bmp");
// 		assert(bmpimg);

// 		uint8_t* addr = new uint8_t[bmpimg->FileSize()];
// 		bmpimg->Read(addr);

// 		File* nbmp = new File("/System/Library/Resources/name.bmp");
// 		assert(nbmp);

// 		uint8_t* nma = new uint8_t[nbmp->FileSize()];
// 		nbmp->Read(nma);



// 		// 35 31 32 IN SRGB
// 		// 45 40 43
// 		// 26 23 24 IN RGB

// 		Console::SetColour(0xFFFFFFFF);
// 		Memory::Set32((void*) GetFramebufferAddress(), 0x00, (LinearFramebuffer::GetResX() * LinearFramebuffer::GetResY()) / 2);
// 		LinearFramebuffer::BackColour = 0x00;


// 		BitmapLibrary::BitmapImage* bmp = new BitmapLibrary::BitmapImage((void*) addr);
// 		BitmapLibrary::BitmapImage* ni = new BitmapLibrary::BitmapImage((void*) nma);


// 		bmp->Render((uint16_t)((LinearFramebuffer::GetResX() - bmp->DIB->GetBitmapWidth()) / 2), (uint16_t)((LinearFramebuffer::GetResY() - math::abs(bmp->DIB->GetBitmapHeight())) / 2 - 40), LinearFramebuffer::GetResX(), (uint32_t*) GetFramebufferAddress());

// 		ni->Render((uint16_t)((LinearFramebuffer::GetResX() - ni->DIB->GetBitmapWidth()) / 2), (uint16_t)((LinearFramebuffer::GetResY() - math::abs(ni->DIB->GetBitmapHeight())) / 2 + 120), LinearFramebuffer::GetResX(), (uint32_t*) GetFramebufferAddress());

// 		delete[] nma;
// 		delete[] addr;
// 	}

// 	void StartProgressBar()
// 	{
// 		string thing = string("[                                                  ]");

// 		uint64_t l = String::Length(thing);
// 		uint16_t xp = (uint16_t)(Console::GetCharsPerLine() - l) / 2 + 1;

// 		uint16_t ox = Console::GetCursorX(), oy = Console::GetCursorY();

// 		uint16_t y = (uint16_t)(0.85 * Console::GetCharsPerColumn()) + 1;


// 		for(int i = 0; i < 50; i++)
// 		{
// 			thing[i + 1] = '=';
// 			{
// 				Console::MoveCursor(0, y);
// 				string spaces;

// 				// clear the line.
// 				for(uint64_t k = 0; k < Console::GetCharsPerLine(); k++)
// 				{
// 					spaces += ' ';
// 				}

// 				StandardIO::PrintString(spaces);
// 				Console::MoveCursor(xp, y);

// 				StandardIO::PrintString(thing);
// 				Console::MoveCursor(ox, oy);
// 			}
// 			SLEEP(50);
// 		}
// 	}

// 	void PrintMessage(const char* msg)
// 	{
// 		uint64_t l = String::Length(msg);
// 		uint16_t xp = (uint16_t)(Console::GetCharsPerLine() - l) / 2 + 1;

// 		uint16_t ox = Console::GetCursorX(), oy = Console::GetCursorY();

// 		uint16_t y = (uint16_t)(0.85 * Console::GetCharsPerColumn());

// 		Console::MoveCursor(0, y);
// 		string spaces;

// 		// clear the line.
// 		for(uint64_t k = 0; k < Console::GetCharsPerLine(); k++)
// 		{
// 			spaces += ' ';
// 		}

// 		StandardIO::PrintString(spaces);
// 		Console::MoveCursor(xp, y);



// 		StandardIO::PrintString(msg);
// 		Console::MoveCursor(ox, oy);
// 	}
// }
// }










