// CarbonShell.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <StandardIO.hpp>
#include <SystemCall.hpp>
#include <String.hpp>
#include <Memory.hpp>
#include <Heap.hpp>
#include <Colours.hpp>
#include <FileIO.hpp>

#include "CarbonShell.hpp"
#include "../../.build.h"


using namespace Library;
using namespace Library::StandardIO;
using Library::string;

namespace CarbonShell
{
	static string* command = 0;
	static string* topleveldir = 0;
	string* currentdir = 0;

	static uint64_t VER_MAJOR;
	static uint64_t VER_MINOR;
	static uint64_t VER_REVSN;
	static uint64_t VER_MINRV;
	const bool PrintFullPath = true;

	static bool IsTyping = false;
	static uint64_t CursorX = 0;
	static uint64_t CursorY = 0;
	static uint64_t CmdPos = 0;
	static uint64_t CmdLength = 0;
	static uint64_t PromptLength = 0;
	static uint32_t CursorColour = 0xFFFFFFFF;

	static uint64_t CharsPerLine = 0;
	static uint64_t CharHeight = 16;
	static uint64_t CharWidth = 9;

	void DrawCursor()
	{
		// get pixel pos
		uint64_t x = CursorX * CharWidth + 3;
		uint64_t y = CursorY * CharHeight;

		uint64_t height = CharHeight;
		for(uint64_t z = 0; z < height - 1; z++)
		{
			SystemCall::PutPixelAtXY(x, y + z, CursorColour);
		}
	}

	void ClearCursor()
	{
		// get pixel pos
		uint64_t x = CursorX * CharWidth + 3;
		uint64_t y = CursorY * CharHeight;

		uint64_t height = CharHeight;
		for(uint64_t z = 0; z < height - 1; z++)
		{
			SystemCall::PutPixelAtXY(x, y + z, 0x0);
		}
	}

	void BlinkCursor()
	{
		SystemCall::Sleep(100);
		while(true)
		{
			CursorColour = ~CursorColour;

			if(IsTyping)
				CursorColour = 0xFFFFFFFF;

			DrawCursor();
			SystemCall::Sleep(500);
			IsTyping = false;
		}
	}


	void PrintPrompt()
	{
		string* s = new string();
		PrintToString(s, "iaperturex:%s root# ", (PrintFullPath ? currentdir : topleveldir)->CString());
		PromptLength = s->Length();

		PrintFormatted(s->CString());

		CursorX = SystemCall::GetCursorX();
		CursorY = SystemCall::GetCursorY();
	}

	void PrintVersion()
	{
		PrintFormatted("%w[mx]%r Version %w%d.%d.%d %wr%02d%r -- Build %w%d\n", Colours::Chartreuse, Colours::DarkCyan, VER_MAJOR, VER_MINOR, VER_REVSN, Colours::Orange, VER_MINRV, Colours::Cyan, X_BUILD_NUMBER);
	}

	void PrintHelp()
	{
		PrintFormatted("Type %whelp%r for help.\n\n", Colours::Purple);
	}

	Library::string* GetCurrentDirectory()
	{
		return currentdir;
	}

	void SetCurrentDirectory(Library::string* c)
	{
		currentdir->Clear();
		// do some parsing.
		// clean up the thing
		LinkedList<string>* list = new LinkedList<string>();
		list = c->Split('/', list);

		for(uint64_t i = 0; i < list->Size(); i++)
		{
			currentdir->Append('/');
			currentdir->Append(list->Get(i));
		}

		topleveldir->Clear();


		if(list->Size() == 0)
		{
			currentdir->Set("/");
			topleveldir->Set("/");
		}

		else
			topleveldir->Set(list->Back());

		delete list;
	}



	void CarbonShell()
	{
		command = new string();
		currentdir = new string("/");
		topleveldir = new string("/");

		// clear the screen.
		Memory::Set64((void*) SystemCall::GetFramebufferAddress(), 0x00, (SystemCall::GetFramebufferResX() * SystemCall::GetFramebufferResY() * 4) / 8);
		SystemCall::SetConsoleBackColour(0x00);
		SystemCall::ClearScreen();

		BuiltinCommands::InitialiseBuiltins();

		// start keyboard loop
		PrintVersion();
		PrintHelp();
		PrintPrompt();

		CursorX = SystemCall::GetCursorX();
		CursorY = SystemCall::GetCursorY();
		CharsPerLine = SystemCall::GetFramebufferResX() / CharWidth - 1;

		// request to know about keyboard events.
		// SystemCall::SendSimpleMessage(0, Kernel::IPC::MessageTypes::RequestServiceDispatch, 2, 0, 0, 0);
		SystemCall::CreateThread(BlinkCursor);


		while(true)
		{
			uint8_t c = 0;

			if(!(c = *SystemCall::ReadIPCSocket(0, &c, 1)))
				continue;


			IsTyping = true;
			CursorColour = 0xFFFFFFFF;
			ClearCursor();


			// todo: rewrite this entire shit.

			if(c == '\n')
			{
				if(command->Length() == 0)
				{
					PrintFormatted("\n");
					PrintPrompt();
					continue;
				}

				PrintFormatted("\n");
				ParseCommand(command);
				PrintPrompt();

				command->Clear();
				CmdLength = 0;
				CmdPos = 0;
			}
			else if(c == '\b' && CmdPos > 0)
			{
				command->Remove((int) CmdPos - 1);
				CursorX--;
				CmdPos--;
			}
			else if(((c ^ 0x80) == 0x50) && CmdPos > 0)
			{
				CursorX--;
				CmdPos--;
			}
			else if(((c ^ 0x80) == 0x4F) && CmdPos < (uint64_t) command->Length())
			{
				CursorX++;
				CmdPos++;
			}
			else if(c <= 0x80 && c != '\b')
			{
				command->Insert((int) CmdPos, (char) c);
				CursorX++;
				CmdPos++;
			}

			SystemCall::SetCursorPos(PromptLength, CursorY);
			for(uint64_t i = 0; i < CmdLength; i++)
				PrintString(" ");

			SystemCall::SetCursorPos(PromptLength, CursorY);
			PrintString(command->CString());
			CmdLength = command->Length();

			DrawCursor();
		}
	}

	extern "C" int main()
	{
		Library::Heap::Initialise();

		// 'compute' the version number from the build number.
		{
			uint64_t v = X_BUILD_NUMBER;

			VER_MAJOR = 4;
			VER_MINOR = v / 10000;
			VER_REVSN = (v - (VER_MINOR * 10000)) / 100;
			VER_MINRV = (v - (VER_MINOR * 10000) - (VER_REVSN * 100)) / 1;
		}

		CarbonShell::CarbonShell();

		return 0;
	}
}






