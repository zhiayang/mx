// CommandParser.hpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <stdint.h>
#include <String.hpp>
#include <List.hpp>

namespace CarbonShell
{
	bool ParseCommand(Library::string* cmd);
	void PrintPrompt();
	void PrintVersion();
	extern Library::string* currentdir;

	Library::LinkedList<Library::string>* ParseEscapes(Library::string* st);
	void SetCurrentDirectory(Library::string* c);
	Library::string* GetCurrentDirectory();

	void PrintError(const char* com, const char* reason);
	void PrintError(const char* com, Library::string* reason);
	void PrintError(Library::string* com, const char* reason);
	void PrintError(Library::string* com, Library::string* reason);

	namespace BuiltinCommands
	{
		// commands
		class Command
		{
			public:
				Command(bool (*c)(Library::string*), Library::string* trigs, Library::string* d)
				{
					this->cmd = c;
					this->triggers = trigs;
					this->desc = d;
				}

				bool (*Execute())(Library::string*)
				{
					return this->cmd;
				}

				Library::string* Trigger()
				{
					return this->triggers;
				}

				Library::string* Description()
				{
					return this->desc;
				}

			private:
				bool (*cmd)(Library::string* args);
				Library::string* triggers;
				Library::string* desc;
		};


		extern Library::LinkedList<Command>* CommandList;
		void InitialiseBuiltins();

		// builtins
		bool Echo(Library::string*);
		bool Help(Library::string*);
		bool Reboot(Library::string*);
		bool ChangeDirectory(Library::string*);
		bool PrintVersion(Library::string*);
		bool ClearScreen(Library::string*);
		bool RunExecutable(Library::string*);
		bool KillProcess(Library::string*);
	}
}








