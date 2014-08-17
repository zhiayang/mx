// FAT32.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#include <Kernel.hpp>
#include <math.h>
#include <stdlib.h>
#include <orion.h>
#include <string.h>

#include <rdestl/vector.h>
#include <rdestl/sstream.h>
#include <rdestl/algorithm.h>

using namespace Library;
using namespace Library::StandardIO;
using namespace Kernel::HardwareAbstraction::Devices::Storage;
using namespace Kernel::HardwareAbstraction::Filesystems::VFS;

#define PATH_DELIMTER		'/'

rde::vector<rde::string>* split(rde::string& s, char delim)
{
	auto ret = new rde::vector<rde::string>();
	rde::string item;

	for(auto c : s)
	{
		if(c == delim)
		{
			if(!item.empty())
				ret->push_back(item);
		}
		else
		{
			item.append(c);
		}
	}
	if(ret->size() == 0 && item.length() > 0)
		ret->push_back(item);

	return ret;
}

namespace Kernel {
namespace HardwareAbstraction {
namespace Filesystems
{
	struct DirectoryEntry
	{
		char name[8];
		char ext[3];
		uint8_t attrib;
		uint8_t userattrib;

		char undelete;
		uint16_t createtime;
		uint16_t createdate;
		uint16_t accessdate;
		uint16_t clusterhigh;

		uint16_t modifiedtime;
		uint16_t modifieddate;
		uint16_t clusterlow;
		uint32_t filesize;

	} __attribute__ ((packed));

	struct LFNEntry
	{
		uint8_t seqnum;
		uint16_t name1[5];
		uint8_t attrib;
		uint8_t type;
		uint8_t checksum;

		uint16_t name2[6];
		uint16_t zero;
		uint16_t name3[2];

	} __attribute__ ((packed));


	#define ATTR_READONLY	0x1
	#define ATTR_HIDDEN		0x2
	#define ATTR_SYSTEM		0x4
	#define ATTR_VOLLABEL	0x8
	#define ATTR_FOLDER		0x10
	// #define ATTR_ARCHIVE		0x20

	#define ATTR_LFN		(ATTR_READONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLLABEL)

	#define FIRSTCHAR_DELETED	0xE5

	struct vnode_data
	{
		rde::string* name;
		uint32_t entrycluster;
		rde::vector<uint32_t>* clusters;
	};

	static vnode_data* tovnd(void* p)
	{
		return (vnode_data*) p;
	}

	static vnode_data* tovnd(vnode* node)
	{
		return (vnode_data*) (node->info->data);
	}

	static uint8_t LFNChecksum(char* ShortName)
	{
		uint8_t ret = 0;
		for(int i = 0; i < 11; i++ )
		{
			// ret = (ret >>> 1) + ShortName[i]
			// where >>> is rotate right
			ret = ((ret & 1) ? 0x80 : 0x00) + (ret >> 1) + ShortName[i];
		}
		return ret;
	}














	uint64_t FSDriverFat32::ClusterToLBA(uint32_t cluster)
	{
		return this->FirstUsableCluster + cluster * this->SectorsPerCluster - (2 * this->SectorsPerCluster);
	}

	FSDriverFat32::FSDriverFat32(Partition* _part) : FSDriver(_part)
	{
		COMPILE_TIME_ASSERT(sizeof(DirectoryEntry) == sizeof(LFNEntry));

		using namespace Devices::Storage::ATA::PIO;
		using Devices::Storage::ATADrive;

		// read the fields from LBA 0
		auto atadev = this->partition->GetStorageDevice();

		uint64_t buf = MemoryManager::Physical::AllocateDMA(1);
		IO::Read(atadev, this->partition->GetStartLBA(), buf, 512);

		uint8_t* fat = (uint8_t*) buf;

		this->BytesPerSector		= *((uint16_t*)((uintptr_t) fat + 11));

		this->SectorsPerCluster		= *((uint8_t*)((uintptr_t) fat + 13));
		this->ReservedSectors		= *((uint16_t*)((uintptr_t) fat + 14));
		this->NumberOfFATs		= *((uint8_t*)((uintptr_t) fat + 16));
		this->NumberOfDirectories	= *((uint16_t*)((uintptr_t) fat + 17));


		if((uint16_t)(*((uint16_t*)(fat + 17))) > 0)
			this->TotalSectors	= *((uint16_t*)((uintptr_t) fat + 17));

		else
			this->TotalSectors	= *((uint32_t*)((uintptr_t) fat + 32));


		this->HiddenSectors		= *((uint32_t*)((uintptr_t) fat + 28));

		this->FATSectorSize		= *((uint32_t*)((uintptr_t) fat + 36));
		this->RootDirectoryCluster	= *((uint32_t*)((uintptr_t) fat + 44));
		this->FSInfoCluster		= *((uint16_t*)((uintptr_t) fat + 48));

		this->BackupBootCluster	= *((uint16_t*)((uintptr_t) fat + 50));
		this->FirstUsableCluster	= this->partition->GetStartLBA() + this->ReservedSectors + (this->NumberOfFATs * this->FATSectorSize);


		char* name = new char[256];
		name[0] = (char) fat[71];
		name[1] = (char) fat[72];
		name[2] = (char) fat[73];
		name[3] = (char) fat[74];
		name[4] = (char) fat[75];
		name[5] = (char) fat[76];
		name[6] = (char) fat[77];
		name[7] = (char) fat[78];
		name[8] = (char) fat[79];
		name[9] = (char) fat[80];
		name[10] = (char) fat[81];

		name = String::TrimWhitespace(name);
		MemoryManager::Physical::FreeDMA(buf, 1);
	}

	FSDriverFat32::~FSDriverFat32()
	{
	}


	bool FSDriverFat32::Traverse(vnode* node, const char* path, char** symlink)
	{
		(void) symlink;

		// vnode is initialised (ie. not null), but its fields are empty.
		assert(node);
		assert(path);
		assert(node->info);


		auto vd = new vnode_data;
		node->info->data = (void*) vd;

		rde::string pth;
		pth.append(path);


		// setup cn
		tovnd(node)->entrycluster = 0;
		tovnd(node)->clusters = 0;
		tovnd(node)->name = 0;

		assert(node->info);

		auto dirs = split(pth, PATH_DELIMTER);
		assert(dirs);

		// remove the last.
		auto file = dirs->back();
		vnode* cn = node;

		for(auto v : *dirs)
		{
			// iterative traverse.
			assert(cn);
			auto cdcontent = this->ReadDir(cn);

			assert(cdcontent);
			// check each.
			for(auto d : *cdcontent)
			{
				auto vnd = tovnd(d->info->data);
				assert(vnd);
				assert(vnd->name);

				if(cn->type == VNodeType::File && *vnd->name == file)
				{
					node->info->data = d->info->data;
					node->info->driver = d->info->driver;
					node->info->id = d->info->id;
					node->data = d->data;

					return true;
				}
				else if(!tovnd(d->info->data)->name->compare(v))
				{
					cn = d;

					// break to continue in outer loop.
					break;
				}
			}
		}

		return false;
	}

	size_t FSDriverFat32::Read(vnode* node, void* buf, off_t offset, size_t length)
	{
		assert(node);
		assert(buf);
		if(length == 0)
			return 0;

		assert(node->info);
		assert(node->info->data);
		assert(node->info->driver == this);

		vnode_data* vnd = tovnd(node);
		uint64_t numclus = 0;
		if(!vnd->clusters)
			vnd->clusters = this->GetClusterChain(node, &numclus);

		assert(vnd->clusters->size() == (int) numclus);

		// because we can read from offsets, don't read all clusters if we can.
		uint64_t skippedclus = offset / (this->SectorsPerCluster * 512);
		uint64_t clusoffset = offset - (skippedclus * this->SectorsPerCluster * 512);

		uint64_t cluslen = (length + (this->SectorsPerCluster * 512 - 1)) / (this->SectorsPerCluster * 512);
		// if we started in the middle of a file, we need to read an additional cluster.
		if(offset > 0)
			cluslen++;

		uint64_t dma = MemoryManager::Physical::AllocateDMA((cluslen * this->SectorsPerCluster * 512 + 0xFFF) / 0x1000);
		uint64_t obuf = dma;

		for(auto i = skippedclus; i < skippedclus + cluslen; i++)
		{
			IO::Read(this->partition->GetStorageDevice(), this->ClusterToLBA((*vnd->clusters)[(int) i]), dma, this->SectorsPerCluster * 512);
			dma += this->SectorsPerCluster * 512;
		}

		dma = obuf;
		memcpy(buf, (void*) (dma + clusoffset), length);
		return length;
	}

	size_t FSDriverFat32::Write(vnode* node, const void* buf, off_t offset, size_t length)
	{
		(void) node;
		(void) buf;
		(void) offset;
		(void) length;
		return 0;
	}

	void FSDriverFat32::Stat(vnode* node, stat* stat)
	{
		(void) node;
		(void) stat;
	}

	rde::vector<VFS::vnode*>* FSDriverFat32::ReadDir(VFS::vnode* node)
	{
		assert(node);
		assert(node->info);
		assert(node->info->data);


		if(tovnd(node)->entrycluster == 0)
			tovnd(node)->entrycluster = 2;

		// grab its clusters.
		auto clusters = tovnd(node->info->data)->clusters;
		uint64_t numclus = 0;
		if(!clusters)
			clusters = this->GetClusterChain(node, &numclus);

		assert((int) numclus == clusters->size());

		// try and read each cluster into a contiguous buffer.
		uint64_t dirsize = numclus * this->SectorsPerCluster * 512;
		uint64_t buf = MemoryManager::Physical::AllocateDMA(((numclus * this->SectorsPerCluster * 512) + 0xFFF) / 0x1000);
		auto obuf = buf;


		assert(clusters);
		for(auto v : *clusters)
		{
			IO::Read(this->partition->GetStorageDevice(), this->ClusterToLBA(v), buf, this->SectorsPerCluster * 512);
			buf += this->SectorsPerCluster * 512;
		}
		buf = obuf;

		auto ret = new rde::vector<VFS::vnode*>();
		for(uint64_t addr = buf; addr < buf + dirsize; )
		{
			auto name = new rde::string();
			uint8_t lfncheck = 0;

			// check if we're on an LFN
			uint8_t* raw = (uint8_t*) addr;
			auto dirent = (DirectoryEntry*) raw;
			if((uint8_t) dirent->name[0] == FIRSTCHAR_DELETED)
			{
				addr += sizeof(LFNEntry);
				continue;
			}
			else if(dirent->attrib == ATTR_LFN && dirent->clusterlow == 0)
			{
				int nument = 0;
				delete name;
				name = this->ReadLFN(addr, nument);
				lfncheck = ((LFNEntry*) dirent)->checksum;

				addr += (nument * sizeof(LFNEntry));
				raw = (uint8_t*) addr;
				dirent = (DirectoryEntry*) raw;
			}

			if(dirent->name[0] != 0)
			{
				if(name->empty() || lfncheck != LFNChecksum((char*) &dirent->name[0]))
				{
					bool lowext = false;
					bool lownm = false;
					// check for windows-specific lowercase/uppercase bits
					uint8_t cas = dirent->userattrib;
					if(cas & 0x8)	lownm = true;
					if(cas & 0x10)	lowext = true;

					for(int i = 0; i < 8 && dirent->name[i] != ' '; i++)
						name->append(lownm ? (char) tolower(dirent->name[i]) : dirent->name[i]);

					if(!(dirent->attrib & ATTR_FOLDER) && dirent->ext[0] != ' ')
						name->append('.');

					for(int i = 0; i < 3 && dirent->ext[i] != ' '; i++)
						name->append(lowext ? (char) tolower(dirent->ext[i]) : dirent->ext[i]);
				}

				vnode* vn = VFS::CreateNode(this);
				if(dirent->attrib & ATTR_READONLY)	vn->attrib |= Attributes::ReadOnly;
				if(dirent->attrib & ATTR_HIDDEN)	vn->attrib |= Attributes::Hidden;

				vn->type = (dirent->attrib & ATTR_FOLDER ? VNodeType::Folder : VNodeType::File);

				// setup fs data
				// we need to fill in the 'entrycluster' value in the vnode
				// so that getclusterchain() can get the rest.

				auto fsd = new vnode_data;
				fsd->name = name;
				fsd->entrycluster = ((uint32_t) (dirent->clusterhigh << 16)) | dirent->clusterlow;

				vn->info->data = (void*) fsd;

				ret->push_back(vn);
			}
			else
				break;

			addr += sizeof(LFNEntry);
		}

		return ret;
	}








	rde::vector<uint32_t>* FSDriverFat32::GetClusterChain(VFS::vnode* node, uint64_t* numclus)
	{
		// read the cluster chain

		assert(node);
		assert(node->info);
		assert(node->info->data);
		assert(node->info->driver == this);

		uint32_t Cluster = tovnd(node)->entrycluster;
		uint32_t cchain = 0;
		auto ret = new rde::vector<uint32_t>();

		uint64_t lastsec = 0;
		auto buf = MemoryManager::Physical::AllocateDMA(2);
		auto obuf = buf;
		do
		{
			uint32_t FatSector = (uint32_t) this->partition->GetStartLBA() + this->ReservedSectors + ((Cluster * 4) / 512);
			uint32_t FatOffset = (Cluster * 4) % 512;

			// check if we even need to read.
			// since we read 8K, we get 15 more free sectors
			// optimisation.
			if(lastsec == 0 || FatSector > lastsec + 15)
			{
				// reset the internal offset
				buf = obuf;
				IO::Read(this->partition->GetStorageDevice(), FatSector, buf, 0x2000);
			}
			else
			{
				// but if it is 'cached' in a sense, we need to update the 'buf' value to point to the actual place.
				buf += (FatSector - lastsec) * 512;
			}

			lastsec = FatSector;

			uint8_t* clusterchain = (uint8_t*) buf;
			cchain = *((uint32_t*)&clusterchain[FatOffset]) & 0x0FFFFFFF;

			// Log("sector %x, fat entry offset %x, cluster %x, nextclus = %x", FatSector, FatSector * 512 + FatOffset, Cluster, cchain);


			// cchain is the next cluster in the list.
			ret->push_back(Cluster);

			Cluster = cchain;
			(*numclus)++;

		} while((cchain != 0) && !((cchain & 0x0FFFFFFF) >= 0x0FFFFFF8));

		buf = obuf;
		MemoryManager::Physical::FreeDMA(buf, 2);
		tovnd(node)->clusters = ret;

		return ret;
	}

	rde::string* FSDriverFat32::ReadLFN(uint64_t addr, int& ret_nument)
	{
		LFNEntry* ent = (LFNEntry*) addr;
		uint8_t seqnum = ent->seqnum;

		rde::string* ret = new rde::string;
		rde::vector<char>* items = new rde::vector<char>();
		// first seqnum & ~0x40 is the number of entries
		uint8_t nument = seqnum & ~0x40;
		for(int i = 0; i < nument; i++)
		{
			ent = (LFNEntry*) addr;
			assert(ent->attrib == 0xF);

			// manually copy sigh
			items->push_back((char) ent->name3[1]);
			items->push_back((char) ent->name3[0]);
			items->push_back((char) ent->name2[5]);
			items->push_back((char) ent->name2[4]);
			items->push_back((char) ent->name2[3]);
			items->push_back((char) ent->name2[2]);
			items->push_back((char) ent->name2[1]);
			items->push_back((char) ent->name2[0]);
			items->push_back((char) ent->name1[4]);
			items->push_back((char) ent->name1[3]);
			items->push_back((char) ent->name1[2]);
			items->push_back((char) ent->name1[1]);
			items->push_back((char) ent->name1[0]);

			addr += sizeof(LFNEntry);
		}

		for(auto c = items->size() - 1; c > 0; c--)
		{
			if((*items)[c] == 0)
				break;

			ret->append((*items)[c]);
		}
		ret_nument = nument;
		return ret;
	}
}














	// void FAT32::WriteFile(VFS::File* file, uint64_t Address, uint64_t length)
	// {
	// 	using namespace HardwareAbstraction::Devices::Storage::ATA::PIO;

	// 	uint64_t buffer = MemoryManager::Physical::AllocateDMA(1);
	// 	uint64_t bc = this->SectorsPerCluster * 512;
	// 	uint32_t Cluster = (uint32_t) file->Parent()->Cluster();
	// 	uint32_t cchain = 0;
	// 	DirectoryEntry* dirent = 0;

	// 	this->ParentPartition->GetStorageDevice()->Read(this->FirstUsableCluster + Cluster * this->SectorsPerCluster - (2 * this->SectorsPerCluster), buffer, bc);
	// 	do
	// 	{
	// 		// read the cluster
	// 		// loop through dirents.

	// 		uint8_t required = 1 + (uint8_t)(String::Length(file->Name()) + 12) / 13;
	// 		uint64_t found = 0;
	// 		for(uint64_t o = 0; o < this->SectorsPerCluster * 512; o += 32)
	// 		{
	// 			uint8_t firstchar = *((uint8_t*)(buffer + o));

	// 			if(firstchar == 0x0 || firstchar == 0xE5)
	// 			{
	// 				found++;

	// 				if(required == found)
	// 				{
	// 					dirent = (DirectoryEntry*)(buffer + o);
	// 					Memory::Set(dirent, 0x0, 32);

	// 					// make the name.
	// 					for(uint64_t t = 0; t < (uint64_t) math::min(String::Length(file->Name()), 8); t++)
	// 					{
	// 						dirent->name[t] = file->Name()[t] & 0xDF;
	// 					}


	// 					dirent->ext[0] = 'L';
	// 					dirent->ext[1] = 'F';
	// 					dirent->ext[2] = 'N';

	// 					// create a bunch of LFNs.

	// 					uint64_t chars = 0;
	// 					uint64_t off = o - 32;
	// 					for(uint8_t m = 1; m < required; m++, off -= 32)
	// 					{
	// 						uint8_t* dat = (uint8_t*)(buffer + off);
	// 						Memory::Set(dat, 0x0, 32);

	// 						*dat = m | (m + 1 == required ? 0x40 : 0x0);

	// 						// calc checksum
	// 						uint8_t sum = 0;

	// 						for (int i = 11; i > 0; i--)
	// 							sum = (uint8_t)((sum & 1) << 7) + (sum >> 1) + ((uint8_t*) dirent)[11 - i];

	// 						*(dat + 0xD) = sum;
	// 						*(dat + 0xB) = 0x0F;

	// 						*((uint16_t*)(dat + 1)) = file->Name()[chars + 0];
	// 						*((uint16_t*)(dat + 3)) = file->Name()[chars + 1];
	// 						*((uint16_t*)(dat + 5)) = file->Name()[chars + 2];
	// 						*((uint16_t*)(dat + 7)) = file->Name()[chars + 3];
	// 						*((uint16_t*)(dat + 9)) = file->Name()[chars + 4];


	// 						*((uint16_t*)(dat + 0x0E)) = file->Name()[chars + 5];
	// 						*((uint16_t*)(dat + 0x10)) = file->Name()[chars + 6];
	// 						*((uint16_t*)(dat + 0x12)) = file->Name()[chars + 7];
	// 						*((uint16_t*)(dat + 0x14)) = file->Name()[chars + 8];
	// 						*((uint16_t*)(dat + 0x16)) = file->Name()[chars + 9];
	// 						*((uint16_t*)(dat + 0x18)) = file->Name()[chars + 10];

	// 						*((uint16_t*)(dat + 0x1A)) = 0;


	// 						*((uint16_t*)(dat + 0x1C)) = file->Name()[chars + 11];
	// 						*((uint16_t*)(dat + 0x1E)) = file->Name()[chars + 12];

	// 						chars += 13;
	// 					}

	// 					break;
	// 				}
	// 			}
	// 			else
	// 			{
	// 				found = 0;
	// 			}
	// 		}

	// 		if(!dirent)
	// 		{
	// 			uint32_t FatSector = (uint32_t) this->ParentPartition->GetStartLBA() + this->ReservedSectors + (Cluster * 4 / 512);
	// 			uint32_t FatOffset = (Cluster * 4) % 512;

	// 			// unfortunately we cannot read the entire FAT at once.
	// 			this->ParentPartition->GetStorageDevice()->Read(FatSector, buffer, 512);

	// 			uint8_t* clusterchain = (uint8_t*) buffer;
	// 			cchain = *((uint32_t*)&clusterchain[FatOffset]) & 0x0FFFFFFF;

	// 			this->ParentPartition->GetStorageDevice()->Read(this->FirstUsableCluster + Cluster * this->SectorsPerCluster - (2 * this->SectorsPerCluster), buffer, bc);
	// 			Cluster = cchain;
	// 		}

	// 	} while(!dirent && (cchain != 0) && !((cchain & 0x0FFFFFFF) >= 0x0FFFFFF8));


	// 	if(!dirent)
	// 	{
	// 		// this is where we allocate a new cluster for the parent folder,
	// 		// then call ourselves and return immediately after.
	// 		// this lets the above code find the cluster, instead of duplicating code.

	// 		AllocateCluster(Cluster);
	// 		return this->WriteFile(file, Address, length);
	// 	}




	// 	// file exists now.
	// 	// get a new cluster for the file.
	// 	uint32_t firstclus = AllocateCluster();
	// 	uint64_t clustersize = this->SectorsPerCluster * 512;
	// 	uint64_t bytesleft = length;
	// 	uint32_t prevcluster = firstclus;
	// 	Log("Allocated cluster %d", firstclus);
	// 	while(bytesleft > length)
	// 	{
	// 		prevcluster = AllocateCluster(prevcluster);
	// 		Log("Allocated cluster %d", prevcluster);
	// 		bytesleft -= math::min(clustersize, bytesleft);
	// 	}

	// 	// now we should have a cluster chain long enough to encompass the entire file's contents.
	// 	// or at least the buffer length.

	// 	// write the cluster to the file's dirent.
	// 	{
	// 		dirent->clusterlow = firstclus & 0xFFFF;
	// 		dirent->clusterhigh = (firstclus >> 16) & 0xFFFF;
	// 		dirent->filesize = (uint32_t) length;
	// 		Log("%x, %x, %x, %x", Cluster, firstclus, buffer, bc);
	// 		this->ParentPartition->GetStorageDevice()->Write(this->FirstUsableCluster + Cluster * this->SectorsPerCluster - (2 * this->SectorsPerCluster), buffer, bc);
	// 	}




	// 	cchain = 0;
	// 	uint64_t buf = MemoryManager::Physical::AllocateDMA((length + 0xFFF) / 0x1000);
	// 	uint64_t BufferOffset = 0;
	// 	uint32_t curclus = firstclus;
	// 	uint64_t left = length;
	// 	Memory::Copy64((void*) buf, (void*) Address, length / 8);
	// 	Memory::Copy((void*) (buf + length), (void*) (Address + length), length - (length / 8));

	// 	uint64_t clusters = MemoryManager::Physical::AllocateDMA(1);
	// 	do
	// 	{
	// 		uint32_t FatSector = (uint32_t) this->ParentPartition->GetStartLBA() + this->ReservedSectors + (curclus * 4 / 512);
	// 		uint32_t FatOffset = (curclus * 4) % 512;

	// 		// unfortunately we cannot read the entire FAT at once.
	// 		this->ParentPartition->GetStorageDevice()->Read(FatSector, clusters, 512);

	// 		uint8_t* clusterchain = (uint8_t*) clusters;
	// 		cchain = *((uint32_t*)&clusterchain[FatOffset]) & 0x0FFFFFFF;

	// 		// cchain is the next cluster in the list.
	// 		uint64_t bytespercluster = this->SectorsPerCluster * this->BytesPerSector;

	// 		this->ParentPartition->GetStorageDevice()->Write(this->FirstUsableCluster + curclus * this->SectorsPerCluster - (2 * this->SectorsPerCluster), buf + BufferOffset, math::min(bytespercluster, left));

	// 		BufferOffset += bytespercluster;
	// 		left -= bytespercluster;
	// 		curclus = cchain;

	// 	} while((cchain != 0) && !((cchain & 0x0FFFFFFF) >= 0x0FFFFFF8));


	// 	MemoryManager::Physical::FreeDMA(clusters, 1);
	// 	MemoryManager::Physical::FreeDMA(buffer, 1);

	// 	MemoryManager::Physical::FreeDMA(buf, (length + 0xFFF) / 0x1000);
	// }









}
}
