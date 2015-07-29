// KernelHeap.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

// Heap things.

#include <Kernel.hpp>

using namespace Kernel;
using namespace Kernel::HardwareAbstraction::MemoryManager;

using namespace Library;

#define MaximumHeapSizeInPages		0xFFFFFFFF
#define Alignment					32
#define MetadataSize				32
#define Paranoid					1
#define MapFlags					0x7
#define ClearMemory					1
#define AllowMagicMismatch			1


#define HEADER_FREE					0xFFFFFFFFEE014EAA
#define HEADER_USED					0xFFFFFFFFEEDEAD00
#define FOOTER_MAGIC				0x00000000EEF055AA



#if Paranoid
#define _assert(h, x)		do { if(!(x)) { Log("header %x failed (owner: %x)", h, h->owner); assert((x)); } } while(0)
#else
#define _assert(h, x)
#endif

#define returnAddr()	((uint64_t) __builtin_return_address(0))


extern "C" void _sane(void* p);

namespace Kernel {
namespace HardwareAbstraction {
namespace MemoryManager {
namespace KernelHeap
{
	bool DidInitialise = false;

	struct Footer;
	struct Header
	{
		uint64_t magic;
		uint64_t size;
		uint64_t owner;
		Footer* footer;
	};

	struct Footer
	{
		uint64_t magic;
		Header* header;
	};

	static uint64_t NumberOfChunksInHeap;
	static uint64_t SizeOfHeapInPages;
	static Header* FirstFreeHeader;

	static uint64_t FirstHeapPhysPage;





	static Header* sane(Header* header)
	{
		assert(header);

		_assert(header, header->magic == HEADER_FREE || header->magic == HEADER_USED);
		_assert(header, header->size > 0);

		// check footer too.
		assert(header->footer);

		_assert(header, header->footer->magic == FOOTER_MAGIC);
		_assert(header, header->footer->header == header);

		return header;
	}

	extern "C" void _sane(void* p)
	{
		sane((Header*) p);
	}

	static Footer* sane(Footer* footer)
	{
		assert(footer);
		sane(footer->header);

		return footer;
	}

	static void ExpandHeap(uint64_t numPages);
	static Header* create(uint64_t addr, uint64_t size, uint64_t owner)
	{
		assert(addr > 0);
		assert(size > 0);

		// Log("creating header at %x, %x bytes (%x)", addr, size, returnAddr());
		Header* header = (Header*) addr;
		memset(header, 0, sizeof(Header));

		header->magic = HEADER_FREE;
		header->owner = owner;
		header->size = size;

		Footer* footer = (Footer*) (addr + sizeof(Header) + size);
		memset(footer, 0, sizeof(Footer));

		footer->magic = FOOTER_MAGIC;
		footer->header = header;

		header->footer = footer;

		NumberOfChunksInHeap++;
		return sane(header);
	}

	// static Footer* getFooter(Header* header)
	// {
	// 	return sane(header)->footer;
	// }

	static Header* getHeader(Footer* footer)
	{
		return sane(footer)->header;
	}

	static Header* next(Header* header)
	{
		return sane((Header*) (sane(header)->footer + 1));
	}

	// static Header* next(Footer* footer)
	// {
	// 	return sane((Header*) (sane(footer) + 1));
	// }

	// static Header* prev(Header* header)
	// {
	// 	Footer* pfoot = sane((Footer*) ((uintptr_t) header - sizeof(Footer)));
	// 	return pfoot->header;
	// }

	// static Header* prev(Footer* footer)
	// {
	// 	sane(footer);
	// 	return prev(footer->header);
	// }

	static Header* getLast()
	{
		Footer* foot = sane((Footer*) ((KernelHeapAddress + (SizeOfHeapInPages * 0x1000)) - sizeof(Footer)));
		return foot->header;
	}

	static bool isFree(Header* header)
	{
		return sane(header)->magic == HEADER_FREE;
	}





	void Initialise()
	{
		// do it.
		FirstHeapPhysPage = Physical::AllocatePage();

		// also map it.
		Virtual::MapAddress(KernelHeapAddress, FirstHeapPhysPage, MapFlags);

		NumberOfChunksInHeap = 1;
		SizeOfHeapInPages = 1;

		FirstFreeHeader = create(KernelHeapAddress, 0x1000 - sizeof(Header) - sizeof(Footer), returnAddr());
		DidInitialise = true;
	}

	void ExpandHeap(uint64_t numPages)
	{
		uint64_t phys = Physical::AllocatePage(numPages);

		Virtual::MapRegion(KernelHeapAddress + (SizeOfHeapInPages * 0x1000), phys, numPages, MapFlags);


		Footer* _last = sane((Footer*) ((KernelHeapAddress + (SizeOfHeapInPages * 0x1000)) - sizeof(Footer)));
		Header* last = getHeader(_last);

		if(isFree(last))
		{
			// we're in luck.
			last->size += (numPages * 0x1000);

			Footer* nfoot = (Footer*) ((uintptr_t) last + sizeof(Header) + last->size);
			nfoot->magic = FOOTER_MAGIC;
			nfoot->header = last;
		}
		else
		{
			// aww. create a new one.
			create(KernelHeapAddress + (SizeOfHeapInPages * 0x1000), numPages * 0x1000 - sizeof(Header) - sizeof(Footer), returnAddr());
		}

		SizeOfHeapInPages += numPages;
	}




















	void* AllocateChunk(uint64_t size, const char* fileAndLine)
	{
		size = ((size + (Alignment - 1)) / Alignment) * Alignment;

		Header* h = sane(FirstFreeHeader);

		while(!isFree(h) || h->size < size)
		{
			h = next(h);

			if(h == getLast() && (!isFree(h) || h->size < size))
			{
				// Log("going to expand heap");
				ExpandHeap((size + 0xFFF) / 0x1000);

				// Log("Expanded heap by %d bytes", ((size + 0xFFF) / 0x1000) * 0x1000);
				return AllocateChunk(size, fileAndLine);
			}
		}

		assert(isFree(h));
		sane(h);

		if(h->size > size + (2 * sizeof(Header)) + sizeof(Footer))
		{
			uint64_t origSize = h->size;
			uint64_t newSize = origSize - size - sizeof(Header) - sizeof(Footer);

			// edge case: if the header we're about to create would exceed
			// the size of the heap, expand it first.

			// Log("h: %x, origSize: %x, size: %x, newsize: %x, max: %x", h, origSize, size, newSize,
			// 	KernelHeapAddress + (SizeOfHeapInPages * 0x1000));

			if((uintptr_t) h + sizeof(Header) + size + sizeof(Footer) + sizeof(Header) + newSize + sizeof(Footer)
				> KernelHeapAddress + (SizeOfHeapInPages * 0x1000))
			{
				ExpandHeap(((newSize + sizeof(Header) + sizeof(Footer)) + 0xFFF) / 0x1000);
				return AllocateChunk(size, fileAndLine);
			}


			// split it.
			// 1. preserve original header.
			// 2. create new footer, reflecting allocated size
			// 3. create new header, for new chunk
			// 4. create new footer, for new chunk.

			h->size = size;

			Footer* f = (Footer*) ((uintptr_t) h + sizeof(Header) + h->size);
			f->header = h;
			f->magic = FOOTER_MAGIC;
			h->footer = f;


			// create new header + footer.
			// Log("splitting. h: %x, h->size: %x, origSize: %x, (hd: %x, ft: %x)", h, h->size, origSize, sizeof(Header), sizeof(Footer));

			create((uintptr_t) h + sizeof(Header) + h->size + sizeof(Footer), newSize, returnAddr());
			// Log("new header: %x, %x", nh, nh->size);
		}


		h->magic = HEADER_USED;
		h->owner = returnAddr();

		// Log("chunk %x goes to owner %x", h, h->owner);
		return (void*) (h + 1);
	}

	void FreeChunk(void* ptr)
	{
		assert(ptr);

	}

	void* ReallocateChunk(void* ptr, uint64_t size)
	{
		HALT("");
		return 0;
	}

	void Print()
	{
	}

	// uint64_t GetFirstHeapMetadataPhysPage();
	uint64_t GetFirstHeapPhysPage()
	{
		return FirstHeapPhysPage;
	}
}
}
}
}








































