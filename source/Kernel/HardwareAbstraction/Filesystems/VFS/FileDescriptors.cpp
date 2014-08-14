// FileDescriptors.cpp
// Copyright 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under Creative Commons Attribution ShareAlike 3.0 Unported.

#include <Kernel.hpp>
#include <String.hpp>
#include <StandardIO.hpp>

namespace Kernel {
namespace HardwareAbstraction {
namespace Filesystems {
namespace VFS
{
	const uint8_t Attr_ReadOnly		= 0x1;
	const uint8_t Attr_Hidden		= 0x2;

	const uint64_t MaxDescriptors		= 256;
	const uint64_t ReservedStreams	= 4;

	uint64_t GetAndIncrementDescriptor(Multitasking::Process* proc)
	{
		// TODO: handle running out of descriptors
		uint64_t ret = proc->CurrentFDIndex;

		// find the next currentfdindex.
		for(uint64_t i = ReservedStreams; i < MaxDescriptors; i++)
		{
			if(proc->FileDescriptors[i].Pointer == 0)
			{
				proc->CurrentFDIndex = i;
				break;
			}
		}

		return ret;
	}

	static FSObject* VerifyDescriptor(Multitasking::Process* proc, uint64_t fd)
	{
		if(fd >= MaxDescriptors || fd < ReservedStreams ||  proc->FileDescriptors[fd].Pointer == 0)
		{
			Log(2, "Error: Descriptor %d does not exist, or is reserved.", fd);
			return nullptr;
		}

		return proc->FileDescriptors[fd].Pointer;
	}

	static File* VerifyFile(Multitasking::Process* proc, uint64_t fd)
	{
		FSObject* fso = VerifyDescriptor(proc, fd);
		if(fso->Type != FSObjectTypes::File)
		{
			Log(2, "Error: Object %s, FD(%d) is not a file.", fso->Name(), fd);
			return nullptr;
		}

		return (File*) fso;
	}

	static Folder* VerifyFolder(Multitasking::Process* proc, uint64_t fd)
	{
		FSObject* fso = VerifyDescriptor(proc, fd);
		if(fso->Type != FSObjectTypes::Folder)
		{
			Log(2, "Error: Object %s, FD(%d) is not a file.", fso->Name(), fd);
			return nullptr;
		}

		return (Folder*) fso;
	}






	uint64_t OpenFile(const char* path, uint8_t mode)
	{
		Multitasking::Process* proc = Multitasking::GetCurrentProcess();
		File* f = new File(path);
		// check mode.
		if(!(mode & Attr_ReadOnly) && f->Attributes() & Attr_ReadOnly)
		{
			Log(2, "Error: File %s is readonly, cannot be opened otherwise.", f->Name());
			mode &= ~(Attr_ReadOnly);
		}

		// create a new entry in the current process's fd list.
		proc->FileDescriptors[proc->CurrentFDIndex].Pointer = f;
		uint64_t ret = proc->CurrentFDIndex;


		// find the next currentfdindex.
		for(uint64_t i = ReservedStreams; i < MaxDescriptors; i++)
		{
			if(proc->FileDescriptors[i].Pointer == 0)
			{
				proc->CurrentFDIndex = i;
				break;
			}
		}

		// TODO: handle running out of descriptors

		return ret;
	}

	void CloseFile(uint64_t fd)
	{
		if(fd < 3)
		{
			Log(1, "Error: you cannot close the %s stream.", fd == 0 ? "stdio" : fd == 1 ? "stdin" : fd == 2 ? "stderr" : "stdlog");
			return;
		}

		Multitasking::Process* proc = Multitasking::GetCurrentProcess();
		if(!VerifyFile(proc, fd))
			return;

		// delete the object.
		proc->CurrentFDIndex = fd;
		delete (File*) proc->FileDescriptors[fd].Pointer;
		proc->FileDescriptors[fd].Pointer = 0;
	}

	uint64_t ReadFile(uint64_t fd, uint8_t* buffer, uint64_t length)
	{
		Multitasking::Process* proc = Multitasking::GetCurrentProcess();

		File* file = VerifyFile(proc, fd);
		if(!file->Exists())
		{
			Log(1, "Error: Object %s, FD (%d) does not exist, cannot read.", file->Name(), fd);
			return 0;
		}

		return file->Read(buffer, length);
	}

	uint64_t FileSize(uint64_t fd)
	{
		Multitasking::Process* proc = Multitasking::GetCurrentProcess();

		File* file = VerifyFile(proc, fd);
		if(!file->Exists())
		{
			Log(1, "Error: Object %s, FD (%d) does not exist, cannot read.", file->Name(), fd);
			return 0;
		}

		return file->FileSize();
	}

	uint64_t WriteFile(uint64_t fd, uint8_t* buffer, uint64_t length)
	{
		(void) fd;
		(void) buffer;
		(void) length;
		return 0;
	}










	uint64_t OpenFolder(const char* path)
	{
		using Multitasking::Process;
		Process* proc = Multitasking::GetCurrentProcess();

		// create a new File object.
		Folder* folder = new Folder(path);
		if(!folder->Exists())
			return 0;

		// skip until the current free one.
		proc->FileDescriptors[proc->CurrentFDIndex].Pointer = folder;

		uint64_t newpd = proc->CurrentFDIndex;
		for(uint64_t i = newpd; i < MaxDescriptors; i++)
		{
			if(proc->FileDescriptors[i].Pointer == 0 && i >= ReservedStreams)
			{
				proc->CurrentFDIndex = i;
				return newpd;
			}
		}

		Log(1, "Warning: Process %s(%d) has no free file descriptors left.", proc->Name, proc->ProcessID);
		return newpd;
	}

	void CloseFolder(uint64_t fd)
	{
		if(fd < 3)
		{
			Log(1, "Error: you cannot close the %s stream.", fd == 0 ? "stdio" : fd == 1 ? "stdin" : fd == 2 ? "stderr" : "stdlog");
			return;
		}


		Multitasking::Process* proc = Multitasking::GetCurrentProcess();

		if(!VerifyFolder(proc, fd))
			return;

		proc->CurrentFDIndex = fd;
		delete (Folder*) proc->FileDescriptors[fd].Pointer;
		proc->FileDescriptors[fd].Pointer = 0;
	}


	Library::LinkedList<char>* ListObjects(uint64_t fd, Library::LinkedList<char>* output, uint64_t* items)
	{
		if(fd < 3)
		{
			Log(1, "Error: the %s stream is not a folder.", fd == 0 ? "stdio" : fd == 1 ? "stdin" : fd == 2 ? "stderr" : "stdlog");
			return 0;
		}


		Multitasking::Process* proc = Multitasking::GetCurrentProcess();

		// else get ready.
		Folder* folder = VerifyFolder(proc, fd);
		Library::LinkedList<FSObject>* list = folder->RootFS()->GetFSDriver()->GetFSObjects(folder);
		for(uint64_t i = 0, s = list->Size(); i < s; i++)
		{
			if(list->Get(i)->Attributes() & Attr_Hidden || list->Get(i)->Name()[0] == '.')
				continue;

			output->InsertBack((char*) list->Get(i)->Name());
			(*items)++;
		}

		delete list;
		return output;
	}

}
}
}
}



















