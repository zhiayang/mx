// // Symbolicate.cpp
// // Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// // Licensed under the Apache License Version 2.0.

// #include <Kernel.hpp>
// #include <StringStream.hpp>
// #include <StandardIO.hpp>
// #include <Utility.hpp>
// #include <Symbolicate.hpp>
// #include <HardwareAbstraction/Filesystems.hpp>
// #include <HardwareAbstraction/Multitasking.hpp>

// using namespace Library;
// using namespace Library::StandardIO;
// using namespace Kernel::HardwareAbstraction;
// using namespace Kernel::HardwareAbstraction::Filesystems::VFS;

// namespace Kernel {
// namespace Symbolicate
// {
// 	struct _entry
// 	{
// 		uint32_t cuindex;
// 		uint32_t linenum;
// 		uint64_t addr;

// 	} __attribute__ ((packed));


// 	static uint64_t totalentries = 0;
// 	static HashMap<uint64_t, LineNamePair*>* map;
// 	static Vector<string*>* cus;

// 	void WorkerThread()
// 	{
// 		// File sym = File("/boot/kernel.alm");
// 		uint64_t fd = OpenFile("/boot/kernel.alm", 0);
// 		if(fd == 0)
// 		{
// 			Log(1, "/boot/kernel.alm doesn't exist, kernel cannot provide meaningful crash reports.");
// 			return;
// 		}

// 		uint8_t* buf = new uint8_t[FileSize(fd)];
// 		// sym.Read(buf);
// 		ReadFile(fd, buf, 0);

// 		uint64_t bread = 0;
// 		// read cus.
// 		string* cur;
// 		char* custr = (char*) buf;
// 		while(*((uint8_t*) custr) != 0xFF)
// 		{
// 			// Log(3, "entry");
// 			cur = new string(custr);
// 			cus->push_back(cur);

// 			custr += cur->Length() + 1;
// 			bread += cur->Length() + 1;
// 		}

// 		// we have all CUs.
// 		// skip the nulls.
// 		// simulate our convertsym program, skip until 32 byte aligned.
// 		while(bread % 32 > 0)
// 		{
// 			bread++;
// 			custr++;
// 		}

// 		uint8_t* entries = (uint8_t*) custr;
// 		_entry* ent = (_entry*) entries;

// 		while(ent->addr != 0xFFFFFFFFFFFFFFFF && ent->cuindex != 0xFFFFFFFF && ent->linenum != 0xFFFFFFFF)
// 		{
// 			// grab things.
// 			LineNamePair* pair = new LineNamePair();
// 			pair->line = ent->linenum;
// 			pair->name = new string((*cus)[ent->cuindex]);

// 			map->Put(ent->addr, pair);

// 			entries += sizeof(_entry);
// 			ent = (_entry*) entries;

// 			totalentries++;
// 			// PrintFormatted("\r                                           \r%d entries - %x", totalentries, ent->cuindex);
// 		}

// 		Log("Kernel symbols parsed, %d entries.", totalentries);
// 		CloseFile(fd);
// 	}

// 	void Initialise()
// 	{
// 		map = new HashMap<uint64_t, LineNamePair*>();
// 		cus = new Vector<string*>();

// 		// starvation heuristics should ensure we don't overwhelm system initialisation.
// 		#if 0
// 			Multitasking::AddToQueue(Multitasking::CreateKernelThread(WorkerThread, 2));
// 		#endif
// 	}

// 	LineNamePair* ResolveAddress(uint64_t addr)
// 	{
// 		LineNamePair** ret = map->get(addr);
// 		if(ret)
// 			return *ret;

// 		else
// 			return nullptr;
// 	}
// }
// }














