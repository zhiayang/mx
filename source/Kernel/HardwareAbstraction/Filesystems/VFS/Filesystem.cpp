// ATA.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <String.hpp>
#include <List.hpp>
#include <StandardIO.hpp>

using Library::LinkedList;
using Library::string;
using namespace Library;

namespace Kernel {
namespace HardwareAbstraction {
namespace Filesystems {
namespace VFS
{
	LinkedList<Filesystem>* Filesystem::Filesystems = 0;

	Filesystem* GetFilesystem(const char* path)
	{
		// check if the path starts with '/' -- if it does, then we're on the current filesystem, just
		// return kernel::rootfs

		if(path[0] == '/')
		{
			return Kernel::RootFS;
		}

		else
		{
			// Log(3, "(%c) - (%c) - (%c) - (%c) - (%c) - (%c) - (%c) - (%c) - (%c) - (%c)", path[0], path[1], path[2], path[3], path[4], path[5], path[6], path[7], path[8], path[9]);
			// search the list of filesystems.
			// make sure it's mounted at /Volumes.
			using Library::LinkedList;
			using Library::string;

			LinkedList<string>* list = new LinkedList<string>();
			string* nm = new string(path);
			list = nm->Split('/', list);

			if(list->Size() < 2 || String::Compare(list->Get(0)->CString(), "Volumes"))
			{
				Log(1, "Invalid filesystem path.");
				return 0;
			}

			// search the list of filesystems for the right one.
			for(uint64_t i = 0; i < Filesystem::Filesystems->Size(); i++)
			{
				if(String::Compare(list->Get(1)->CString(), Filesystem::Filesystems->Get(i)->Name()))
				{
					return Filesystem::Filesystems->Get(i);
				}
			}

			delete nm;
			delete list;

			return 0;
		}
	}




	Filesystem::Filesystem(const char* name, FSDriver* fsd) : FSObject(FSObjectTypes::Filesystem)
	{
		if(this->Filesystems == 0)
			this->Filesystems = new LinkedList<Filesystem>();

		this->_name = new char[Library::String::Length(name)];
		Library::String::CopyLength((char*) this->_name, name, Library::String::Length(name));
		this->fsdriver = fsd;
		this->Filesystems->InsertBack(this);
	}

	const char* Filesystem::Name()
	{
		return this->_name;
	}

	FSDriver* Filesystem::GetFSDriver()
	{
		return this->fsdriver;
	}


	FSObject* Filesystem::GetFSObject(const char* path)
	{
		if(path[0] != '/')
		{
			Log("Path must start with '/', assuming first folder is not root");
		}

		// Log(3, "called getfsobject");

		LinkedList<string>* names = new LinkedList<string>();
		string* p = new string(path);

		p->Split('/', names);

		Folder* folder = this->fsdriver->GetRootFolder();
		if(path[0] == '/' && String::Length(path) == 1)
		{
			delete names;
			return folder;
		}

		LinkedList<FSObject>* list;

		uint64_t np = 0;

		repeatloop:
		assert(this->fsdriver);
		Log(3, "repeat");
		list = this->fsdriver->GetFSObjects(folder);
		Log(3, "size: %d", list->Size());

		for(uint64_t i = 0; i < list->Size(); i++)
		{
			Log(3, "%s - %s", list->Get(i)->Name(), names->Get(np)->CString());
			if(String::Compare(list->Get(i)->Name(), names->Get(np)->CString()))
			{
				if(list->Get(i)->Type == FSObjectTypes::File)
				{
					// if this is a file, return it.
					Log(3, "ret file");
					return list->Get(i);
				}
				else if(list->Get(i)->Type == FSObjectTypes::Folder)
				{
					folder = (Folder*) list->Get(i);

					if(np == names->Size() - 1)
					{
						delete names;
						delete list;
						Log(3, "ret folder");
						return folder;
					}

					np++;

					// avoid memory leak
					delete list;

					// evil, feel it.
					Log(3, "goto repeat");
					goto repeatloop;
				}
				else
				{
					Log(3, "unknown type: %d", list->Get(i)->Type);
					HALT("undefined");
					// this is a filesystem.
					// handle separately.
				}
			}
		}

		// Log(3, "Error: File %s not found.", path);
		delete names;
		delete list;
		return nullptr;
	}

	FSObject* Filesystem::Parent()
	{
		return this;
	}

	const char* Filesystem::Path()
	{
		// TODO: return something sane.
		return (new string("/"))->CString();
	}

	Filesystem* Filesystem::RootFS()
	{
		return this;
	}

	uint8_t Filesystem::Attributes()
	{
		return 0;
	}

	bool Filesystem::Exists()
	{
		return true;
	}
}
}
}
}
