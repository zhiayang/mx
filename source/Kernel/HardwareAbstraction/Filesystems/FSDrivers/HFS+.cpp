// // HFS+.cpp
// // Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// // Licensed under the Apache License Version 2.0.

// #include <Kernel.hpp>
// #include <StandardIO.hpp>
// using namespace Kernel::HardwareAbstraction::MemoryManager;
// using namespace Library::StandardIO;

// namespace Kernel {
// namespace HardwareAbstraction {
// namespace Filesystems
// {
// 	#define __builtin_bswap16

// 	HFSPlus::HFSPlus(Devices::Storage::Partition* parent) : FSDriver(FSTypes::hfsplus)
// 	{
// 		// allocate a page;
// 		uint64_t buffer = Physical::AllocateDMA(1);
// 		this->ParentPartition = parent;

// 		this->ParentPartition->GetStorageDevice()->Read(this->ParentPartition->GetStartLBA() + 2, buffer, 1024);

// 		this->volumeheader = new VolumeHeader_type;

// 		this->volumeheader->signature[0] = *((uint8_t*) buffer);
// 		this->volumeheader->signature[1] = *((uint8_t*)(buffer + 1));
// 		this->volumeheader->version = __builtin_bswap16(*((uint16_t*)(buffer + 2)));
// 		this->volumeheader->attributes = __builtin_bswap32(*((uint32_t*)(buffer + 4)));
// 		this->volumeheader->lastmountedversion = __builtin_bswap32(*((uint32_t*)(buffer + 8)));
// 		this->volumeheader->journalinfoblock = __builtin_bswap32(*((uint32_t*)(buffer + 12)));

// 		this->volumeheader->createdate = __builtin_bswap32(*((uint32_t*)(buffer + 16)));
// 		this->volumeheader->modifydate = __builtin_bswap32(*((uint32_t*)(buffer + 20)));
// 		this->volumeheader->backupdate = __builtin_bswap32(*((uint32_t*)(buffer + 24)));
// 		this->volumeheader->checkeddate = __builtin_bswap32(*((uint32_t*)(buffer + 28)));

// 		this->volumeheader->filecount = __builtin_bswap32(*((uint32_t*)(buffer + 32)));
// 		this->volumeheader->foldercount = __builtin_bswap32(*((uint32_t*)(buffer + 36)));

// 		this->volumeheader->blocksize = __builtin_bswap32(*((uint32_t*)(buffer + 40)));
// 		this->volumeheader->totalblocks = __builtin_bswap32(*((uint32_t*)(buffer + 44)));
// 		this->volumeheader->freeblocks = __builtin_bswap32(*((uint32_t*)(buffer + 48)));

// 		this->volumeheader->nextalloc = __builtin_bswap32(*((uint32_t*)(buffer + 52)));
// 		this->volumeheader->resourceclumpsize = __builtin_bswap32(*((uint32_t*)(buffer + 56)));
// 		this->volumeheader->dataclumpsize = __builtin_bswap32(*((uint32_t*)(buffer + 60)));
// 		// __builtin_bswap32(*((uint32_t*)(buffer + 64)));

// 		this->volumeheader->writecount = __builtin_bswap32(*((uint32_t*)(buffer + 68)));
// 		this->volumeheader->encodingbitmap = __builtin_bswap64(*((uint64_t*)(buffer + 72)));

// 		// this->volumeheader->journalinfoblock = __builtin_bswap32(*((uint32_t*)(buffer + 76)));

// 		// this->volumeheader->journalinfoblock = __builtin_bswap32(*((uint32_t*)(buffer + 80)));
// 		// this->volumeheader->journalinfoblock = __builtin_bswap32(*((uint32_t*)(buffer + 84)));
// 		// this->volumeheader->journalinfoblock = __builtin_bswap32(*((uint32_t*)(buffer + 88)));
// 		// this->volumeheader->journalinfoblock = __builtin_bswap32(*((uint32_t*)(buffer + 92)));
// 		// this->volumeheader->journalinfoblock = __builtin_bswap32(*((uint32_t*)(buffer + 96)));

// 		Physical::FreeDMA(buffer, 1);
// 	}
