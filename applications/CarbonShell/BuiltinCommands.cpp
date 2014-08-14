// BuiltinCommands.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#include <StandardIO.hpp>
#include <SystemCall.hpp>
#include <String.hpp>
#include <FileIO.hpp>

#include "CarbonShell.hpp"

using Library::string;
using Library::LinkedList;
using namespace Library;
using namespace Library::StandardIO;

namespace CarbonShell
{
	void PrintHelp();

	namespace BuiltinCommands
	{
		LinkedList<Command>* CommandList;


		void PrintNoExist(const char* com, const char* dir)
		{
			string* t = new string();
			t->Set(dir);
			t->Append(": no such file directory");
			CarbonShell::PrintError(com, t);
			delete t;
		}

		void InitialiseBuiltins()
		{
			CommandList = new LinkedList<Command>();

			// add commands.
			CommandList->InsertBack(new Command(Echo, new string("echo"), new string("Prints back all its arguments.")));
			CommandList->InsertBack(new Command(Help, new string("help"), new string("Lists built-in commands. Optionally provides detailed usage.")));
			CommandList->InsertBack(new Command(Reboot, new string("reboot"), new string("Restarts the computer.")));
			CommandList->InsertBack(new Command(ChangeDirectory, new string("cd"), new string("Changes the current directory.")));
			CommandList->InsertBack(new Command(PrintVersion, new string("version"), new string("Prints some version information.")));
			CommandList->InsertBack(new Command(ClearScreen, new string("clear"), new string("Clears the screen.")));
			CommandList->InsertBack(new Command(RunExecutable, new string("run"), new string("ha")));
			CommandList->InsertBack(new Command(KillProcess, new string("kill"), new string("ha")));
		}

		bool Echo(string* s)
		{
			// handle the fact that the thing may be quote enclosed.
			if((*s)[0] == '"' && (*s)[s->Length() - 1] == '"')
			{
				s->SubString(1, s->Length() - 2);
			}

			PrintFormatted("%s\n\n", s->CString());
			return true;
		}

		bool Help(string* s)
		{
			if(Library::String::Compare(s->CString(), ""))
			{
				uint64_t padding = 16;
				for(uint64_t i = 0; i < CommandList->Size(); i++)
				{
					PrintFormatted("%s", CommandList->Get(i)->Trigger()->CString());
					for(uint64_t g = 0; g < padding - CommandList->Get(i)->Trigger()->Length(); g++)
					{
						Library::SystemCall::PrintChar(' ');
					}

					PrintFormatted("%s\n", CommandList->Get(i)->Description()->CString());
				}

				PrintFormatted("\n");
				return true;
			}

			// SystemCall::SpawnProcess("/Applications/HelloSpamTest.oex", "HelloSpamTest");
			// SystemCall::Yield();

			return true;
		}

		bool RunExecutable(string* s)
		{
			using namespace FileIO;
			(void) s;
			SystemCall::SpawnProcess(s->CString(), s->CString());
			SystemCall::Yield();

			return true;
		}

		bool KillProcess(string* s)
		{
			using namespace FileIO;
			(void) s;
			kill(2, 41);

			return true;
		}


		bool Reboot(string* s)
		{
			(void) s;
			PrintFormatted("Reboot not supported at this time.\n");
			return true;
		}

		bool ChangeDirectory(string* s)
		{
			// check for absolute first.
			if(s->StartsWith('/'))
			{
				if(Library::SystemCall::CheckFolderExistence(s->CString()))
				{
					CarbonShell::SetCurrentDirectory(s);
				}
				else
				{
					PrintNoExist("cd", s->CString());
				}
			}
			else if(s->Contains(".."))
			{
				// we need to do some looped solver.
				string* t = new string(CarbonShell::GetCurrentDirectory()->CString());
				LinkedList<string>* list = new LinkedList<string>();
				LinkedList<string>* nlist = new LinkedList<string>();

				string* s1 = new string(s->CString());

				list = s1->Split('/', list);
				nlist = t->Split('/', nlist);

				for(uint64_t i = 0, m = list->Size(); i < m; i++)
				{
					if(nlist->Size() > 0 && String::Compare(list->Front()->CString(), ".."))
					{
						nlist->RemoveBack();
					}
					else if(!String::Compare(list->Front()->CString(), ".."))
					{
						string* thispoint = new string("/");
						for(uint64_t k = 0; k < nlist->Size(); k++)
						{
							thispoint->Append(nlist->Get(k));
							thispoint->Append("/");
						}
						thispoint->Append(list->Front());

						if(Library::SystemCall::CheckFolderExistence(thispoint->CString()))
						{
							nlist->InsertBack(list->Front());
						}
						else
						{
							PrintNoExist("cd", s1->CString());
						}

						// delete thispoint;
					}

					list->RemoveFront();
				}

				string* final = new string();
				for(uint64_t i = 0; i < nlist->Size(); i++)
				{
					// PrintFormatted("/%s", nlist->Get(i));
					final->Append('/');
					final->Append(nlist->Get(i));
				}

				if(final->Length() == 0)
					final->Set("/");

				CarbonShell::SetCurrentDirectory(final);
				// delete final;
				// delete t;
				// delete list;
				// delete nlist;
			}
			else
			{
				string* t = new string(CarbonShell::GetCurrentDirectory()->CString());
				t->Append('/');
				t->Append(s);

				if(Library::SystemCall::CheckFolderExistence(t->CString()))
				{
					CarbonShell::SetCurrentDirectory(t);
				}
				else
				{
					PrintNoExist("cd", s->CString());
				}

				// delete t;
			}

			return true;
		}

		bool PrintVersion(string* s)
		{
			(void) s;
			CarbonShell::PrintVersion();
			PrintFormatted("Built on %s at %s, with GCC %d.%d.%d compatible clang++.\n\n", __DATE__, __TIME__, __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);


			return true;
		}

		bool ClearScreen(string* s)
		{
			(void) s;
			SystemCall::ClearScreen();
			CarbonShell::PrintVersion();

			CarbonShell::PrintHelp();


			return true;
		}
	}
}







