// KernelHeap.cpp
// Copyright (c) 2013 - 2016, zhiayang@gmail.com
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
#define _assert(h, x)		do { if(!(x)) { Log("header %lx failed (fields:\nmagic = %lx\nsize =  %x\nownr1 = %lx\nownr2 = %lx\nfootr = %lx\n\nmagic = %lx, headr = %lx)", h, h->magic, h->size, 0xFFFFFFFF00000000 | h->owner, 0xFFFFFFFF00000000 | h->owner1, h->footer, h->footer ? h->footer->magic : 0, h->footer ? h->footer->header : 0); assert((x)); } } while(0)
#else
#define _assert(h, x)
#endif

#define returnAddr(x)	((uint64_t) __builtin_return_address(x))


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
		uint32_t owner;
		uint32_t owner1;
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
	static Mutex* mtx;

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
	static Header* create(uint64_t addr, uint64_t size, uint64_t owner1, uint64_t owner2)
	{
		assert(addr > 0);
		assert(size > 0);

		// Log("creating header at %x, %x bytes (%x)", addr, size, returnAddr());
		Header* header = (Header*) addr;
		memset(header, 0, sizeof(Header));

		header->magic = HEADER_FREE;
		header->owner = (owner1 & 0xFFFFFFFF);
		header->owner1 = (owner2 & 0xFFFFFFFF);
		header->size = size;

		Footer* footer = (Footer*) (addr + sizeof(Header) + size);
		memset(footer, 0, sizeof(Footer));

		footer->magic = FOOTER_MAGIC;
		footer->header = header;

		header->footer = footer;

		NumberOfChunksInHeap++;
		return sane(header);
	}

	static Footer* getFooter(Header* header)
	{
		return sane(header)->footer;
	}

	static Header* getHeader(Footer* footer)
	{
		return sane(footer)->header;
	}

	static Header* getLast();
	static Header* next(Header* header)
	{
		// check we're not the last
		if(header != getLast())
		{
			return sane((Header*) (sane(header)->footer + 1));
		}
		else
		{
			return 0;
		}
	}

	// static Header* next(Footer* footer)
	// {
	// 	return sane((Header*) (sane(footer) + 1));
	// }

	static Header* prev(Header* header)
	{
		// check we're not first.
		if((uintptr_t) header != KernelHeapAddress)
		{
			Footer* pfoot = sane((Footer*) ((uintptr_t) header - sizeof(Footer)));
			return pfoot->header;
		}
		else
		{
			return 0;
		}
	}

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

		FirstFreeHeader = create(KernelHeapAddress, 0x1000 - sizeof(Header) - sizeof(Footer), returnAddr(0), returnAddr(1));
		DidInitialise = true;

		mtx = new Mutex();
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
			create(KernelHeapAddress + (SizeOfHeapInPages * 0x1000), numPages * 0x1000 - sizeof(Header) - sizeof(Footer), returnAddr(0),
				returnAddr(1));
		}

		SizeOfHeapInPages += numPages;
	}




















	void* AllocateChunk(uint64_t size)
	{
		if(size == 0) return 0;

		auto mutex = AutoMutex(mtx);
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
				return AllocateChunk(size);
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
				return AllocateChunk(size);
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

			create((uintptr_t) h + sizeof(Header) + h->size + sizeof(Footer), newSize, returnAddr(0), returnAddr(1));
			// Log("new header: %x, %x", nh, nh->size);
		}


		h->magic = HEADER_USED;
		h->owner = returnAddr(0) & 0xFFFFFFFF;
		h->owner1 = returnAddr(1)  & 0xFFFFFFFF;

		// Log("chunk %x goes to owner %x", h, h->owner);
		return (void*) (h + 1);
	}



	void TryMergeLeft(Header* hdr)
	{
		assert(isFree(hdr));

		hdr = sane(hdr);
		if(prev(hdr) != 0)
		{
			Header* left = prev(hdr);
			if(isFree(left))
			{
				// do it.
				// 1. modify its header to point to our footer
				// 2. modify our footer to point to its header
				// 2. modify it ssize to include our size, and sizeof(Header) + sizeof(Footer)

				void* oldFooter = left->footer;
				void* oldHeader = hdr;

				left->footer = getFooter(hdr);
				getFooter(hdr)->header = left;

				left->size += hdr->size + sizeof(Header) + sizeof(Footer);

				// done.
				// cleanup:
				// memset its footer and our header to 0.
				memset(oldFooter, 0, sizeof(Footer));
				memset(oldHeader, 0, sizeof(Header));
			}
		}
	}

	void TryMergeRight(Header* hdr)
	{
		assert(isFree(hdr));

		hdr = sane(hdr);
		if(next(hdr) != 0)
		{
			Header* right = next(hdr);
			if(isFree(right))
			{
				// do it.
				// 1. modify our header to point to its footer
				// 2. modify its footer to point to our header
				// 3. modify our size to include its size, and sizeof(Header) + sizeof(Footer)

				void* oldFooter = hdr->footer;
				void* oldHeader = right;

				hdr->footer = getFooter(right);
				getFooter(right)->header = hdr;

				hdr->size += right->size + sizeof(Header) + sizeof(Footer);


				// done.
				// cleanup:
				// memset its footer and our header to 0.
				memset(oldFooter, 0, sizeof(Footer));
				memset(oldHeader, 0, sizeof(Header));
			}
		}
	}


	void FreeChunk(void* ptr)
	{
		if(ptr == 0)
		{
			Log(1, "free() called with ptr == 0, might be a bug: return %p / %p", returnAddr(0), returnAddr(1));
			return;
		}

		auto mutex = AutoMutex(mtx);

		Header* hdr = (Header*) ptr;
		hdr -= 1;

		sane(hdr);

		// make sure it's actually used.
		// todo: investigate a curious double-free bug that happens when we terminate
		// a process from userspace.

		// _assert(hdr, !isFree(hdr));

		// set it to free.
		hdr->magic = HEADER_FREE;

		// try merging, and we're done.
		// merge right first, because that preserves *our* header, and destroys
		// the other one -- this ensures we can try merging left as well.
		TryMergeRight(hdr);
		TryMergeLeft(hdr);
	}

	void* ReallocateChunk(void* ptr, uint64_t size)
	{
		if(ptr == 0)
		{
			Log(1, "called realloc() with ptr == 0, possible bug: return %p / %p", returnAddr(0), returnAddr(1));
			return AllocateChunk(size);
		}

		if(size == 0)
		{
			Log(1, "called realloc() with size == 0, possible bug: return %p / %p", returnAddr(0), returnAddr(1));
			FreeChunk(ptr);
			return 0;
		}

		auto mutex = AutoMutex(mtx);


		// now get the actual header.
		Header* hdr = (Header*) ptr;
		hdr -= 1;

		sane(hdr);

		// make sure it's actually used.
		_assert(hdr, !isFree(hdr));

		// check the capacity of the chunk vs what we need.
		if(hdr->size >= size)
		{
			return ptr;	// nothing to be done.
		}
		else
		{
			// we need to allocate a new one, then free it.
			void* newplace = AllocateChunk(size);
			memmove(newplace, ptr, hdr->size);

			FreeChunk(ptr);
			return newplace;
		}
	}

	void Print()
	{
		Header* hdr = (Header*) KernelHeapAddress;
		sane(hdr);

		do
		{
			Log("Chunk %0.5x, data %0.5x, size %x, %s", (uintptr_t) hdr - KernelHeapAddress,
				(uintptr_t) hdr - KernelHeapAddress + sizeof(Header), hdr->size, isFree(hdr) ? "free" : "used");

		} while((hdr = next(hdr)) != 0);
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








































