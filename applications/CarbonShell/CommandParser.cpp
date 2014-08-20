// CommandParser.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#include <StandardIO.hpp>
#include <SystemCall.hpp>
#include <String.hpp>
#include <List.hpp>

#include "CarbonShell.hpp"

using Library::LinkedList;
using Library::string;
using namespace Library::StandardIO;
using namespace Library;

namespace CarbonShell
{
	void PrintError(const char* com, const char* reason)
	{
		PrintFormatted("-carbon: %s: %s\n", com, reason);
	}

	void PrintError(const char* com, string* reason)
	{
		PrintError(com, reason->CString());
	}

	void PrintError(string* com, const char* reason)
	{
		PrintError(com->CString(), reason);
	}

	void PrintError(string* com, string* reason)
	{
		PrintError(com, reason->CString());
	}




	bool ParseCommand(Library::string* cmd)
	{
		LinkedList<string>* tokens = ParseEscapes(cmd);

		// concat the arguments.
		string* args = new string();
		for(uint64_t i = 1; i < tokens->Size(); i++)
		{
			args->Append(tokens->Get(i));

			if(i < tokens->Size() - 1)
				args->Append(' ');
		}

		// find the command to execute.
		for(uint64_t i = 0; i < BuiltinCommands::CommandList->Size(); i++)
		{
			if(*BuiltinCommands::CommandList->Get(i)->Trigger() == *tokens->Get(0))
			{
				bool r = BuiltinCommands::CommandList->Get(i)->Execute()(args);
				delete args;
				delete tokens;

				return r;
			}
		}

		PrintError(tokens->Get(0), "command not found");
		return false;
	}


	LinkedList<string>* ParseEscapes(string* cmd)
	{
		// custom split function.
		LinkedList<string>* ret = new LinkedList<string>();
		string* curtok = new string();

		bool IsEscape = false;
		for(int i = 0; i < cmd->Length(); i++)
		{
			if((*cmd)[i] == '\\')
			{
				IsEscape = true;
				continue;
			}

			else
			{
				// check if we're escaping
				if(IsEscape)
				{
					IsEscape = false;
					curtok->Append((*cmd)[i]);
				}

				else if((*cmd)[i] == ' ' && !IsEscape)
				{
					ret->InsertBack(curtok);
					curtok = new string();
				}

				else
				{
					curtok->Append((*cmd)[i]);
				}
			}
		}

		// add the last bit.
		ret->InsertBack(curtok);

		return ret;
	}
}









