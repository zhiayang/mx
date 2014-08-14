// Folder.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <String.hpp>
#include <List.hpp>

namespace Kernel {
namespace HardwareAbstraction {
namespace Filesystems {
namespace VFS
{
	Folder::Folder(const char* c, uint64_t cl, Filesystem* rfs, uint8_t attr) : FSObject(FSObjectTypes::Folder)
	{
		this->_name = new char[Library::String::Length(c)];
		Library::String::CopyLength((char*) this->_name, c, Library::String::Length(c));
		this->_cluster = cl;
		this->rootfs = rfs;
		this->_attributes = attr;
		this->_exists = true;
		this->Type = FSObjectTypes::Folder;
	}

	Folder::Folder(const char* path) : FSObject(FSObjectTypes::Folder)
	{
		this->_name = new char[Library::String::Length(path)];
		Library::String::CopyLength((char*) this->_name, path, Library::String::Length(path));
		this->rootfs = GetFilesystem(path);
		this->Type = FSObjectTypes::Folder;

		FSObject* fso = this->RootFS()->GetFSObject(path);
		if(fso->Type != FSObjectTypes::Folder)
		{
			this->_exists = false;
			return;
		}

		Folder* f = (Folder*) fso;
		this->_cluster = f->_cluster;
		delete f;

		this->_exists = true;
		this->_attributes = f->_attributes;
	}


	const char* Folder::Name()
	{
		using Library::LinkedList;
		using Library::string;

		LinkedList<string>* list = new LinkedList<string>();
		string* nm = new string(this->_name);

		list = nm->Split('/', list);
		if(list->Size() == 0)
			return this->_name;

		char* rn = list->Get(list->Size() - 1)->CString();

		delete nm;
		delete list;

		return (const char*) rn;
	}

	const char* Folder::Path()
	{
		return this->_name;
	}

	bool Folder::Exists()
	{
		return this->_exists;
	}

	uint8_t Folder::Attributes()
	{
		return this->_attributes;
	}

	uint64_t Folder::Cluster()
	{
		return this->_cluster;
	}

	Filesystem* Folder::RootFS()
	{
		return this->rootfs;
	}

	Folder* Folder::Parent()
	{
		return (Folder*) this->parent;
	}
}
}
}
}
