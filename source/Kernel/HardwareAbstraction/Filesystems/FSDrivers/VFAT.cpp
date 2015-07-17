// FAT32.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#include <Kernel.hpp>
#include <math.h>
#include <stdlib.h>
#include <orion.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include <String.hpp>
#include <rdestl/vector.h>
#include <sys/stat.h>

#include <StandardIO.hpp>
#include <HardwareAbstraction/Filesystems.hpp>

using namespace Library;
using namespace Kernel::HardwareAbstraction::Devices::Storage;
using namespace Kernel::HardwareAbstraction::Filesystems::VFS;

#define PATH_DELIMTER		'/'


namespace Kernel {
namespace HardwareAbstraction {
namespace Filesystems
{





	// ********************************************
	// *********** BOTH FAT16 and FAT32 ***********
	// ********************************************

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

	#define FAT16	16
	#define FAT32	32

	struct vnode_data
	{
		rde::string name;
		uint32_t entrycluster;
		rde::vector<uint32_t> clusters;
		uint32_t filesize;

		DirectoryEntry dirent;
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
			ret = ((ret & 1) ? 0x80 : 0x00) + (ret >> 1) + (uint8_t) ShortName[i];
		}
		return ret;
	}

	static time_t datetounix(uint16_t dosdate, uint16_t dostime)
	{
		uint8_t year	= (dosdate & 0xFE00) >> 9;
		uint8_t month	= (dosdate & 0x1E0) >> 5;
		uint8_t day		= (dosdate & 0x1F);

		uint8_t hour	= (dostime & 0xF800) >> 11;
		uint8_t minute	= (dostime & 0x7E0) >> 5;
		uint8_t sec2	= (dostime & 0x1F);

		tm ts;
		ts.tm_year		= year;
		ts.tm_mon		= month;
		ts.tm_mday		= day;

		ts.tm_hour		= hour;
		ts.tm_min		= minute;
		ts.tm_sec		= sec2 * 2;

		return 0;
		// return mktime(&ts);
	}

	static rde::vector<rde::string*> split(rde::string& s, char delim)
	{
		rde::vector<rde::string*> ret;
		rde::string* item = new rde::string();

		for(auto c : s)
		{
			if(c == delim)
			{
				if(!item->empty())
				{
					ret.push_back(item);
					item = new rde::string();
				}
			}
			else
				item->append(c);
		}

		if(item->length() > 0)
			ret.push_back(item);

		return ret;
	}




























	uint64_t FSDriverFAT::ClusterToLBA(uint32_t cluster)
	{
		return this->FirstUsableSector + (cluster * this->SectorsPerCluster - (2 * this->SectorsPerCluster));
	}

	FSDriverFAT::FSDriverFAT(Partition* _part) : FSDriver(_part, FSDriverType::Physical)
	{
		COMPILE_TIME_ASSERT(sizeof(DirectoryEntry) == sizeof(LFNEntry));

		using namespace Devices::Storage::ATA::PIO;
		using Devices::Storage::ATADrive;

		// read the fields from LBA 0
		auto atadev = this->partition->GetStorageDevice();

		uint64_t buf = MemoryManager::Virtual::AllocatePage(1);
		IO::Read(atadev, this->partition->GetStartLBA(), buf, 512);

		uint8_t* fat = (uint8_t*) buf;

		this->BytesPerSector		= *((uint16_t*)((uintptr_t) fat + 11));
		this->SectorsPerCluster		= *((uint8_t*)((uintptr_t) fat + 13));
		this->ReservedSectors		= *((uint16_t*)((uintptr_t) fat + 14));
		this->NumberOfFATs			= *((uint8_t*)((uintptr_t) fat + 16));
		this->HiddenSectors			= *((uint32_t*)((uintptr_t) fat + 28));


		// NOTE: Fat16 and Fat32 differences follow.
		// refer to one "fat103" document.

		// here, we determine the size of the fat: 16 or 32.
		{
			uint16_t RootEntryCount = *((uint16_t*)((uintptr_t) fat + 17));
			this->RootDirectorySize = ((RootEntryCount * 32) + 511) / 512;

			uint32_t FATSize = 0;

			if(*((uint16_t*)((uintptr_t) fat + 22)) == 0)
			{
				FATSize = *((uint32_t*)((uintptr_t) fat + 36));
			}
			else
			{
				FATSize = *((uint16_t*)((uintptr_t) fat + 22));
			}

			uint64_t TotalSectors = 0;
			if(*((uint16_t*)((uintptr_t) fat + 19)) == 0)
			{
				TotalSectors = *((uint32_t*)((uintptr_t) fat + 32));
			}
			else
			{
				TotalSectors = *((uint16_t*)((uintptr_t) fat + 19));
			}

			uint64_t DataSec = TotalSectors - (this->ReservedSectors + (this->NumberOfFATs * FATSize) + this->RootDirectorySize);
			uint64_t NumClusters = DataSec / this->SectorsPerCluster;

			if(NumClusters < 4085)
			{
				HALT("FAT12 not supported");
			}
			else if(NumClusters < 65525)
			{
				this->FATKind = FAT16;
			}
			else
			{
				this->FATKind = FAT32;
			}
		}




		if(this->FATKind == FAT32)
		{
			this->FATSectorSize			= *((uint32_t*)((uintptr_t) fat + 36));
			this->RootDirectoryCluster	= *((uint32_t*)((uintptr_t) fat + 44));
			this->FSInfoCluster			= *((uint16_t*)((uintptr_t) fat + 48));
			this->backupBootCluster		= *((uint16_t*)((uintptr_t) fat + 50));
			this->FirstUsableSector 	= this->partition->GetStartLBA() + this->ReservedSectors
											+ (this->NumberOfFATs * this->FATSectorSize);

			char* name = new char[16];
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

			Log("fat32 root directory cluster: %d", this->RootDirectoryCluster);
			Log("** reserved: %d", this->ReservedSectors);
			Log("** numfats:  %d", this->NumberOfFATs);
			Log("** fatsecsz: %d", this->FATSectorSize);
			Log("** secprcls: %d", this->SectorsPerCluster);
		}
		else
		{
			this->FATSectorSize			= *((uint16_t*)((uintptr_t) fat + 22));
			this->RootDirectoryCluster	= (uint32_t) this->partition->GetStartLBA() + (this->ReservedSectors
											+ (this->NumberOfFATs * this->FATSectorSize));

			this->FirstUsableSector		= this->RootDirectoryCluster + this->RootDirectorySize;

			Log("fat16 root directory sector: %d", this->RootDirectoryCluster);
			Log("** reserved: %d", this->ReservedSectors);
			Log("** numfats:  %d", this->NumberOfFATs);
			Log("** fatsecsz: %d", this->FATSectorSize);
			Log("** secprcls: %d", this->SectorsPerCluster);

			char* name = new char[16];
			name[0] = (char) fat[43];
			name[1] = (char) fat[44];
			name[2] = (char) fat[45];
			name[3] = (char) fat[46];
			name[4] = (char) fat[47];
			name[5] = (char) fat[48];
			name[6] = (char) fat[49];
			name[7] = (char) fat[50];
			name[8] = (char) fat[51];
			name[9] = (char) fat[52];
			name[10] = (char) fat[53];

			name = String::TrimWhitespace(name);
		}



		MemoryManager::Virtual::FreePage(buf, 1);
		this->_seekable = true;


		// todo: handle not 512-byte sectors
		assert(this->BytesPerSector == 512);
		assert(((Devices::Storage::ATADrive*) atadev)->GetSectorSize() == 512);

		Log("FAT%d Driver on disk%ds%d has been initialised (Cluster size %d bytes)", this->FATKind, atadev->diskid,
			this->partition->GetPartitionNumber(), this->SectorsPerCluster * 512);
	}

	FSDriverFAT::~FSDriverFAT()
	{
	}

	void ls(FSDriver* fs, vnode* node, int nest)
	{
		rde::vector<vnode*> nodes = fs->ReadDir(node);
		if(nodes.size() == 0)
			return;

		vnode* n = 0;
		for(size_t k = 1; k < nodes.size(); k++)
		{
			n = nodes[k];
			if(tovnd(n)->name == ".." || tovnd(n)->name == ".")
				continue;

			rde::string out;
			for(int i = 0; i < nest; i++)
				out.append("  ");

			out.append("=> ");
			out.append(tovnd(n)->name);

			Log(out.c_str());


			if(n->type == VNodeType::Folder)
				ls(fs, n, nest + 1);
		}
	}

	bool FSDriverFAT::Traverse(vnode* node, const char* path, char** symlink)
	{
		(void) symlink;

		// vnode is initialised (ie. not null), but its fields are empty.
		assert(node);
		assert(path);
		assert(node->info);

		vnode_data* vd = new vnode_data;
		node->info->data = (void*) vd;


		// list everything
		if(strcmp(path, "/texts/big.txt") == 0)
		{
			Log("*** begin list");
			ls(this, node, 0);
			Log("*** end list");
		}



		rde::string pth = rde::string(path);

		// setup cn
		tovnd(node)->entrycluster = 0;

		assert(node->info);

		rde::vector<rde::string*> dirs = split(pth, PATH_DELIMTER);
		assert(dirs.size() > 0);

		size_t levels = dirs.size();
		size_t curlvl = 1;

		// remove the last.
		auto file = dirs.back();
		vnode* cn = node;

		for(auto v : dirs)
		{
			bool found = false;

			// iterative traverse.
			assert(cn);
			assert(cn->info);
			assert(cn->info->data);

			rde::vector<VFS::vnode*> cdcontent = this->ReadDir(cn);

			// check each.
			for(auto d : cdcontent)
			{
				auto vnd = tovnd(d->info->data);
				assert(vnd);

				rde::string vndlower = vnd->name;
				vndlower.make_lower();

				rde::string filelower = *file;
				filelower.make_lower();

				rde::string vlower = *v;
				vlower.make_lower();

				if(curlvl == levels && d->type == VNodeType::File && String::Compare(vndlower.c_str(), filelower.c_str()) == 0)
				{
					node->info->data = d->info->data;
					node->info->driver = d->info->driver;
					node->info->id = d->info->id;
					node->data = d->data;

					return true;
				}
				else if(String::Compare(vndlower.c_str(), vlower.c_str()) == 0)
				{
					found = true;
					cn = d;
					break;
				}
			}
			if(!found)
			{
				return false;
			}

			curlvl++;
		}

		return false;
	}


















	static rde::vector<rde::pair<uint64_t, uint64_t>> ConsolidateClusterChain(rde::vector<uint32_t> cchain)
	{
		typedef rde::pair<uint64_t, uint64_t> pair_t;

		rde::vector<pair_t> ret;
		rde::quick_sort(cchain.begin(), cchain.end());

		for(size_t i = 0; i < cchain.size(); i++)
		{
			pair_t p = { cchain[i], 1 };
			while(i + 1 < cchain.size())
			{
				i++;
				if(cchain[i] == (p.first + p.second))
				{
					p.second++;
				}
				else
				{
					break;
				}
			}

			ret.push_back(p);
		}

		return ret;
	}


	size_t FSDriverFAT::Read(vnode* node, void* buf, off_t offset, size_t length)
	{
		assert(node);
		assert(buf);
		if(length == 0)
			return 0;

		assert(node->info);
		assert(node->info->data);
		assert(node->info->driver == this);

		Memory::Set(buf, 0, length);

		vnode_data* vnd = tovnd(node);
		uint64_t numclus = 0;
		if(vnd->clusters.size() == 0)
			vnd->clusters = this->GetClusterChain(node, &numclus);

		assert(vnd->clusters.size() > 0);
		numclus = vnd->clusters.size();

		// check that offset is not more than size
		if(offset > vnd->filesize)
			return 0;

		// clamp length to the filesize.
		if(length > vnd->filesize - offset)
			length = vnd->filesize - offset;

		// because we can read from offsets, don't read all clusters if we can.
		uint64_t skippedclus = offset / (this->SectorsPerCluster * 512);
		uint64_t clusoffset = offset - (skippedclus * this->SectorsPerCluster * 512);

		uint64_t cluslen = (length + (this->SectorsPerCluster * 512 - 1)) / (this->SectorsPerCluster * 512);
		// if we started in the middle of a file, we need to read an additional cluster.
		if(offset > 0)
			cluslen++;

		uint64_t bufferPageSize = (cluslen * this->SectorsPerCluster * 512 + 0xFFF) / 0x1000;


		uint64_t rbuf = MemoryManager::Virtual::AllocatePage(bufferPageSize);
		uint64_t obuf = rbuf;


		auto clusterpairs = ConsolidateClusterChain(vnd->clusters);

		uint64_t skipped = 0;
		uint64_t have = 0;
		for(auto pair : clusterpairs)
		{
			// completely consume the pair if we need to skip
			if(skippedclus > skipped && pair.second <= (skippedclus - skipped))
			{
				skipped += pair.second;
				continue;
			}
			else if(skippedclus > skipped)
			{
				// 'partially' consume the pair.
				pair.first += (skippedclus - skipped);
			}


			// completely read the clusters in the pair
			if(cluslen > have)
			{

				uint64_t spc = this->SectorsPerCluster;
				uint64_t toread = ((cluslen - have) > pair.second) ? (pair.second) : (cluslen - have);

				IO::Read(this->partition->GetStorageDevice(), this->ClusterToLBA((uint32_t) pair.first), rbuf, toread * spc * 512);
				rbuf += (toread * spc * 512);
				have += toread;

				StandardIO::PrintFormatted("\r                               \r%8d bytes read", rbuf - obuf);
			}

			// exit condition
			if(have == cluslen)
				break;
		}

		rbuf = obuf;
		Memory::Copy(buf, (void*) (rbuf + clusoffset), length);
		MemoryManager::Virtual::FreePage(rbuf, bufferPageSize);
		return length;
	}



	size_t FSDriverFAT::Write(vnode* node, const void* buf, off_t offset, size_t length)
	{
		(void) node;
		(void) buf;
		(void) offset;
		(void) length;
		return 0;
	}

	void FSDriverFAT::Stat(vnode* node, struct stat* stat, bool statlink)
	{
		// we really just need the dirent.
		assert(node);
		assert(node->info);
		assert(node->info->data);
		assert(node->info->driver == this);

		(void) statlink;

		assert(stat);
		DirectoryEntry* dirent = &tovnd(node)->dirent;

		stat->st_dev		= 0;
		stat->st_ino		= 0;
		stat->st_mode		= 0;
		stat->st_nlink		= 0;
		stat->st_uid		= 0;
		stat->st_gid		= 0;
		stat->st_size		= tovnd(node)->filesize;
		stat->st_blksize	= (tovnd(node)->filesize + (512 - 1)) / 512;
		stat->st_blocks		= stat->st_blksize;
		stat->st_atime		= datetounix(dirent->accessdate, 0);
		stat->st_mtime		= datetounix(dirent->modifieddate, dirent->modifiedtime);
		stat->st_ctime		= datetounix(dirent->createdate, dirent->createtime);
	}



	rde::vector<VFS::vnode*> FSDriverFAT::ReadDir(VFS::vnode* node)
	{
		assert(node);
		assert(node->info);
		assert(node->info->data);
		assert(node->info->driver == this);

		uint64_t buf = 0;
		uint64_t dirsize = 0;
		uint64_t bufferPageSize = 0;

		if(tovnd(node)->entrycluster == 0)
		{
			if(this->FATKind == FAT16)
			{
				bufferPageSize = ((this->RootDirectorySize * 512) + 0xFFF) / 0x1000;
				dirsize = this->RootDirectorySize * 512;

				buf = MemoryManager::Virtual::AllocatePage(bufferPageSize);

				// in fat16, "this->RootDirectoryCluster" is actually the SECTOR of the root directory
				IO::Read(this->partition->GetStorageDevice(), this->RootDirectoryCluster, buf, this->RootDirectorySize * 512);
			}
			else
			{
				tovnd(node)->entrycluster = this->RootDirectoryCluster;
			}
		}
		else
		{
			// grab its clusters.
			auto clusters = tovnd(node)->clusters;
			uint64_t numclus = 0;

			if(clusters.size() == 0)
				clusters = this->GetClusterChain(node, &numclus);

			assert(clusters.size() > 0);
			numclus = clusters.size();

			// try and read each cluster into a contiguous buffer.
			bufferPageSize = ((numclus * this->SectorsPerCluster * 512) + 0xFFF) / 0x1000;

			dirsize = numclus * this->SectorsPerCluster * 512;
			buf = MemoryManager::Virtual::AllocatePage(bufferPageSize);
			auto obuf = buf;

			for(auto v : clusters)
			{
				IO::Read(this->partition->GetStorageDevice(), this->ClusterToLBA(v), buf, this->SectorsPerCluster * 512);
				buf += this->SectorsPerCluster * 512;
			}
			buf = obuf;
		}



		rde::vector<VFS::vnode*> ret;

		for(uint64_t addr = buf; addr < buf + dirsize;)
		{
			rde::string name;
			uint8_t lfncheck = 0;

			// check if we're on an LFN
			uint8_t* raw = (uint8_t*) addr;
			auto dirent = (DirectoryEntry*) raw;

			if(dirent->name[0] == 0)
				break;

			else if((uint8_t) dirent->name[0] == FIRSTCHAR_DELETED)
			{
				addr += sizeof(LFNEntry);
				continue;
			}
			else if(dirent->attrib == ATTR_LFN && dirent->clusterlow == 0)
			{
				uint64_t nument = 0;
				name = this->ReadLFN(addr, &nument);
				lfncheck = ((LFNEntry*) dirent)->checksum;

				addr += (nument * sizeof(LFNEntry));
				raw = (uint8_t*) addr;
				dirent = (DirectoryEntry*) raw;
			}

			if(dirent->name[0] != 0)
			{
				if(name.empty() || lfncheck != LFNChecksum((char*) &dirent->name[0]))
				{
					bool lowext = false;
					bool lownm = false;
					// check for windows-specific lowercase/uppercase bits
					uint8_t cas = dirent->userattrib;
					if(cas & 0x8)	lownm = true;
					if(cas & 0x10)	lowext = true;

					for(int i = 0; i < 8 && dirent->name[i] != ' '; i++)
						name.append(lownm ? (char) tolower(dirent->name[i]) : dirent->name[i]);

					if(!(dirent->attrib & ATTR_FOLDER) && dirent->ext[0] != ' ')
						name.append('.');

					for(int i = 0; i < 3 && dirent->ext[i] != ' '; i++)
						name.append(lowext ? (char) tolower(dirent->ext[i]) : dirent->ext[i]);

				}

				vnode* vn = VFS::CreateNode(this);

				if(dirent->attrib & ATTR_READONLY)	vn->attrib |= Attributes::ReadOnly;
				if(dirent->attrib & ATTR_HIDDEN)	vn->attrib |= Attributes::Hidden;

				vn->type = (dirent->attrib & ATTR_FOLDER ? VNodeType::Folder : VNodeType::File);

				// setup fs data
				// we need to fill in the 'entrycluster' value in the vnode
				// so that getclusterchain() can get the rest.
				// (if we need it. don't call getclusterchain() every time, especially for sibling directories that we're not interested in)

				auto fsd = new vnode_data;
				Memory::Set(fsd, 0, sizeof(vnode_data));

				fsd->name = name;


				if(this->FATKind == FAT32)	fsd->entrycluster = ((uint32_t) (dirent->clusterhigh << 16)) | dirent->clusterlow;
				else						fsd->entrycluster = dirent->clusterlow;


				fsd->filesize = dirent->filesize;

				Memory::Copy(&fsd->dirent, dirent, sizeof(DirectoryEntry));

				vn->info->data = (void*) fsd;

				ret.push_back(vn);
			}

			addr += sizeof(LFNEntry);
		}

		MemoryManager::Virtual::FreePage(buf, bufferPageSize);
		return ret;
	}



	bool FSDriverFAT::Create(VFS::vnode* node, const char* path, uint64_t flags, uint64_t perms)
	{
		(void) node;
		(void) path;
		(void) flags;
		(void) perms;

		return false;
	}

	bool FSDriverFAT::Delete(VFS::vnode* node, const char* path)
	{
		(void) node;
		(void) path;

		return false;
	}

	void FSDriverFAT::Flush(VFS::vnode*)
	{
	}




	rde::vector<uint32_t> FSDriverFAT::GetClusterChain(VFS::vnode* node, uint64_t* numclus)
	{
		// read the cluster chain

		assert(node);
		assert(node->info);
		assert(node->info->data);
		assert(node->info->driver == this);

		bool condition = false;
		uint32_t Cluster = tovnd(node)->entrycluster;
		uint32_t cchain = 0;
		const uint32_t lookahead = 0;
		rde::vector<uint32_t> ret;

		auto buf = MemoryManager::Virtual::AllocatePage(lookahead == 0 ? 1 : (512 * lookahead) / 0x1000);
		auto obuf = buf;

		uint32_t ClusterMultFactor = (this->FATKind == FAT16 ? 2 : 4);
		do
		{
			uint32_t FatSector = (uint32_t) this->partition->GetStartLBA() + this->ReservedSectors + ((Cluster * ClusterMultFactor) / 512);
			uint32_t FatOffset = (Cluster * ClusterMultFactor) % 512;

			buf = obuf;

			IO::Read(this->partition->GetStorageDevice(), FatSector, obuf, lookahead == 0 ? 512 : lookahead * 512);

			uint8_t* clusterchain = (uint8_t*) buf;

			if(this->FATKind == FAT32)
			{
				cchain = *((uint32_t*) &clusterchain[FatOffset]) & 0x0FFFFFFF;
				condition = (cchain != 0x0) && (cchain != 0x1) && !((cchain & 0x0FFFFFFF) >= 0x0FFFFFF7);
			}
			else
			{
				cchain = *((uint16_t*) &clusterchain[FatOffset]) & 0xFFFF;
				condition = (cchain != 0x0) && (cchain != 0x1) && !((cchain & 0xFFFF) >= 0xFFF7);
			}

			ret.push_back(Cluster);

			// cchain is the next cluster in the list.
			Cluster = cchain;
			(*numclus)++;

		} while(condition);

		MemoryManager::Virtual::FreePage(obuf, lookahead == 0 ? 1 : (512 * lookahead) / 0x1000);
		tovnd(node)->clusters = ret;

		// Log(3, "read %d clusters", *numclus);
		return ret;
	}

	rde::string FSDriverFAT::ReadLFN(uint64_t addr, uint64_t* ret_nument)
	{
		LFNEntry* ent = (LFNEntry*) addr;
		uint8_t seqnum = ent->seqnum;

		rde::string ret;
		rde::vector<char>* items = new rde::vector<char>();

		// first seqnum & ~0x40 is the number of entries
		uint8_t nument = seqnum & ~0x40;
		for(uint64_t i = 0; i < nument; i++)
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

		for(auto c = items->size(); c > 0; c--)
		{
			if((*items)[c - 1] == 0)
				break;

			ret.append((*items)[c - 1]);
		}
		*ret_nument = nument;
		return ret;
	}

	void FSDriverFAT::Close(VFS::vnode*)
	{
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
