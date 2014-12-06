// KernelHeap.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


// Heap things.

#include <Kernel.hpp>
#include <Memory.hpp>
#include <StandardIO.hpp>
#include <String.hpp>

using namespace Kernel;
using namespace Kernel::HardwareAbstraction::MemoryManager;

using namespace Library;




#if 0



namespace Kernel {
namespace HardwareAbstraction {
namespace MemoryManager {
namespace KernelHeap
{
	const uint32_t SplitMargin		= 32;			// minimum size of the resulting block before we do a split
	const uint32_t HeaderMagic		= 0x71ff11A1;	// magic number.
	const bool ClearReturnedMemory	= true;

	struct Header;
	struct Footer
	{
		Header* head;
		Header* getNextHeader()
		{
			return (Header*) (this + 1);
		}
	};

	struct Header
	{
		uint32_t magic;
		uint32_t size;


		// currently the 'used' bit
		// if meta & 0x1, block is used.

		// bits 1 - 31 are the index, shifted left by one.
		uint64_t meta;


		uint64_t position()
		{
			return this->meta >> 1;
		}

		void setPosition(uint64_t newpos)
		{
			// get rid of the existing position
			this->meta &= 0x1;

			// set the new one.
			this->meta |= (newpos << 1);
		}

		uint8_t* data()
		{
			return (uint8_t*) (this + 1);
		}

		Header* next()
		{
			return (Header*) ((uint64_t) this + sizeof(Header) + this->size + sizeof(Footer));
		}

		Footer* footer()
		{
			return (Footer*) ((uint64_t) this + sizeof(Header) + this->size);
		}

		bool free()
		{
			return !(this->meta & 0x1);
		}

		// give dobby a sock
		void setFree()
		{
			this->meta &= ~0x1;
		}

		// confiscate that sock
		void setUsed()
		{
			if(!this->free())
			{
				Log(3, "ERROR MAYBE? tried to use an already used block\nDEBUG: RETURN ADDRESS %x", __builtin_return_address(0));
				// HALT("ERROR MAYBE? Tried to set an already used block to used");
			}

			this->meta |= 0x1;
		}
	};


	static uint32_t roundSize(uint32_t val, uint32_t target)	{ return (val + target - 1) / target * target; }
	static Header* assertSane(Header* h)						{ assert(h && h->magic == HeaderMagic); return h; }
	static Header* setupBlock(uint64_t addr, uint32_t size)
	{
		assert(addr);
		Header* h = (Header*) addr;
		h->magic = HeaderMagic;
		h->meta = 0;				// set to free
		h->size = size;

		assertSane(h);
		assert(h->free());

		Footer* f = (Footer*) (addr + sizeof(Header) + h->size);
		f->head = h;

		return h;
	}

	static Header* firstBlock = 0;
	static Header* firstFreeBlock = 0;
	static uint64_t blockCount = 0;
	static uint64_t heapSizeInPages = 0;


	Header* ExpandHeap(uint32_t minSize)
	{
		// get the last block;
		Header* last = firstBlock;
		for(uint64_t i = 0; i < blockCount - 1; i++)
		{
			assertSane(last);
			last = last->next();
		}

		int numPages = roundSize(minSize, 0x1000) / 0x1000;
		uint64_t addr = KernelHeapAddress + heapSizeInPages * 0x1000;

		for(int i = 0; i < numPages; i++)
			Virtual::MapAddress(addr + (i * 0x1000), Physical::AllocatePage(), 0x7);

		Header* ret = 0;
		if(last->free())
		{
			// simply allocate new page(s), then return the last block.
			last->size += numPages * 0x1000;

			// setup the new footer at the new place
			last->footer()->head = last;
			ret = last;
		}
		else
		{
			// create a new one.
			// uint64_t newHeader = (uint64_t) last->next();
			uint64_t newHeader = addr;
			// if(newHeader != addr)
			// {
			// 	Log(3, "Expected '%x', got '%x'... dumping:", addr, newHeader);
			// 	Print();
			// 	HALT("Address mismatch");
			// }

			Header* nb = setupBlock(newHeader, (numPages * 0x1000) - sizeof(Header) - sizeof(Footer));
			ret = assertSane(nb);
			ret->setPosition(blockCount);
			blockCount++;
		}

		heapSizeInPages += numPages;
		return ret;
	}


	void* AllocateChunk(uint64_t size)
	{
		if(size == 0)
			HALT("Please fuck off");

		uint32_t blockSize = roundSize((uint32_t) size, 32);
		Header* ff = firstFreeBlock;
		assertSane(ff);

		// check if we can use it.
		Header* toUse = 0;
		if(ff->size >= blockSize && ff->free())
		{
			toUse = ff;
		}
		else
		{
			// search for more!
			Header* next = ff;
			int i = 0;
			while(true)
			{

				assertSane(next);
				if(next->size >= blockSize && next->free())
					break;


				if(i == blockCount - 1)
				{
					next = ExpandHeap(blockSize);
					assertSane(next);

					assert(next->size >= blockSize);
					break;
				}

				next = next->next();
				i++;
			}

			toUse = next;
		}

		assertSane(toUse);

		if(toUse->position() != blockCount - 1 && toUse->next()->free())
			firstFreeBlock = toUse->next();


		// check if we can split it, and whether it's wise
		if(toUse->size > blockSize + sizeof(Header) + sizeof(Footer))
		{
			uint32_t resultingSize = toUse->size - blockSize - sizeof(Header) - sizeof(Footer);
			if(resultingSize >= SplitMargin)
			{
				// first resize the old block
				toUse->size = blockSize;
				toUse->footer()->head = toUse;

				// then create the new one
				uint64_t newAddr = (uint64_t) toUse->data() + toUse->size + sizeof(Footer);
				Header* newBlock = setupBlock(newAddr, resultingSize);
				if(ClearReturnedMemory)
					Memory::Set(newBlock->data(), 0, newBlock->size);

				newBlock->setPosition(blockCount);
				blockCount++;
			}
		}

		toUse->setUsed();
		assertSane(toUse);

		return toUse->data();
	}

	void FreeChunk(void* ptr)
	{
	}

	uint64_t QuerySize(void* ptr)
	{
		Header* head = (Header*) ((uint64_t) ptr - sizeof(Header));
		assertSane(head);

		return head->size;
	}

	void Print()
	{
		Header* first = firstBlock;
		for(uint64_t i = 0, m = blockCount; i < m; i++)
		{
			StandardIO::PrintFormatted("Header(%d, %d/%d) at %x => size(%d : %x), %s\n", i, first->position(), m, first - KernelHeapAddress, first->size, first->size, first->free() ? "f" : "u");
			first = first->next();
		}
	}

	void Initialise()
	{
		// set up the first block.
		uint64_t addr = KernelHeapAddress;
		Virtual::MapAddress(addr, Physical::AllocatePage(), 0x7);

		firstBlock = setupBlock(addr, 0x1000 - sizeof(Header) - sizeof(Footer));
		firstFreeBlock = firstBlock;
		blockCount = 1;
		heapSizeInPages = 1;
	}
}
}
}
}

#endif



