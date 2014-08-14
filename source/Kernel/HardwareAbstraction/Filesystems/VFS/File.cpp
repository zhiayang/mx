// File.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <String.hpp>
#include <List.hpp>
#include <StandardIO.hpp>

namespace Kernel {
namespace HardwareAbstraction {
namespace Filesystems {
namespace VFS
{
	File::File(const char* c, uint64_t s, uint64_t cl, Filesystem* rfs, uint8_t attr) : FSObject(FSObjectTypes::File)
	{
		this->_path = new char[Library::String::Length(c)];
		Library::String::CopyLength((char*) this->_path, c, Library::String::Length(c));
		this->_FileSize = s;
		this->_Cluster = cl;
		this->rootfs = rfs;
		this->_exists = true;
		this->_attributes = attr;

		{
			using Library::LinkedList;
			using Library::string;

			LinkedList<string>* list = new LinkedList<string>();
			string* nm = new string(this->_path);

			list = nm->Split('/', list);

			char* rn = list->Get(list->Size() - 1)->CString();

			this->_name = new char[Library::String::Length(rn)];
			Library::String::CopyLength((char*) this->_name, rn, Library::String::Length(rn));

			string* par = new string();
			par->Append('/');
			for(uint64_t i = 0; i < list->Size() - 1; i++)
			{
				if(i != 0)
					par->Append('/');

				par->Append(list->Get(i));
			}

			Folder* pare = new Folder(par->CString());
			this->parent = pare;

			delete nm;
			delete list;
			delete[] rn;

		}
		// Log(3, "constructed via 1, cl = %d", this->_Cluster);
	}

	File::File(const char* path) : FSObject(FSObjectTypes::File)
	{
		this->rootfs = GetFilesystem(path);
		if(!this->rootfs)
		{
			Log(3, "Error: file '%s' does not exist.", path);
			this->_exists = false;
			return;
		}

		bool e = true;
		FSObject* fso = (File*) this->rootfs->GetFSObject(path);
		if(!fso)
		{
			Log(3, "(%x)", __builtin_return_address(0));
			HALT("lolwut");
			e = false;
		}
		else if(fso->Type != FSObjectTypes::File)
		{
			e = false;
		}

		File* f = (File*) fso;
		this->_path = new char[Library::String::Length(path)];
		Library::String::CopyLength((char*) this->_path, path, Library::String::Length(path));
		this->_FileSize = f->_FileSize;
		this->_Cluster = f->_Cluster;
		this->_attributes = f->_attributes;
		delete f;

		this->_exists = e;


		{
			using Library::LinkedList;
			using Library::string;

			LinkedList<string>* list = new LinkedList<string>();
			string* nm = new string(this->_path);

			list = nm->Split('/', list);

			char* rn = list->Get(list->Size() - 1)->CString();

			this->_name = new char[Library::String::Length(rn)];
			Library::String::CopyLength((char*) this->_name, rn, Library::String::Length(rn));

			string* par = new string();
			par->Append('/');
			for(uint64_t i = 0; i < list->Size() - 1; i++)
			{
				if(i != 0)
					par->Append('/');

				par->Append(list->Get(i));
			}

			Folder* pare = new Folder(par->CString());
			this->parent = pare;

			delete nm;
			delete list;
			delete[] rn;
		}
		// Log(3, "constructed via 2, cl = %d", this->_Cluster);
	}

	const char* File::Name()
	{
		return this->_name;
	}

	uint64_t File::FileSize()
	{
		return this->_FileSize;
	}

	bool File::Exists()
	{
		return this->_exists;
	}

	const char* File::Path()
	{
		return this->_path;
	}

	uint64_t File::Cluster()
	{
		return this->_Cluster;
	}

	Filesystem* File::RootFS()
	{
		return this->rootfs;
	}

	uint8_t File::Attributes()
	{
		return this->_attributes;
	}

	uint64_t File::Read(uint8_t* buffer, uint64_t length)
	{
		if(!this->_exists)
		{
			Log(3, "Error: cannot read from non-existent file");
			return 0;
		}

		this->RootFS()->GetFSDriver()->ReadFile(this, (uint64_t) buffer, length == 0 ? this->_FileSize : length);
		return math::min(length, this->_FileSize);
	}

	Folder* File::Parent()
	{
		return (Folder*) this->parent;
	}
}
}
}
}
