// LibAlloc.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <HardwareAbstraction/MemoryManager/LibAlloc.hpp>

#define VERSION 	"1.1"
#define ALIGNMENT	16ul//4ul				///< This is the byte alignment that memory must be allocated on. IMPORTANT for GTK and other stuff.

#define ALIGN_TYPE		uint64_t				/// unsigned short
#define ALIGN_INFO		sizeof(ALIGN_TYPE) * 2
///< Alignment information is stored right before the pointer. This is the number of bytes of information stored there.


#define USE_CASE1
#define USE_CASE2
#define USE_CASE3
#define USE_CASE4
#define USE_CASE5


/** This macro will conveniently align our pointer upwards */
#define ALIGN(ptr)														\
if(ALIGNMENT > 1)														\
{																		\
	uintptr_t _diff;													\
	ptr = (void*) ((uintptr_t) ptr + ALIGN_INFO);						\
	_diff = (uintptr_t) ptr & (ALIGNMENT - 1);							\
	if(_diff != 0)														\
	{																	\
		_diff = ALIGNMENT - _diff;										\
		ptr = (void*)((uintptr_t) ptr + _diff);							\
	}																	\
	*((ALIGN_TYPE*) ((uintptr_t) ptr - ALIGN_INFO)) = 					\
		_diff + ALIGN_INFO;												\
}


#define UNALIGN(ptr)													\
if(ALIGNMENT > 1)														\
{																		\
	uintptr_t _diff = *((ALIGN_TYPE*) ((uintptr_t) ptr - ALIGN_INFO));	\
	if(_diff < (ALIGNMENT + ALIGN_INFO))								\
	{																	\
		ptr = (void*) ((uintptr_t) ptr - _diff);						\
	}																	\
}



#define LIBALLOC_MAGIC	0xC001C0DE
#define LIBALLOC_DEAD	0xDEADDEAD

// #if defined DEBUG || defined INFO
// #include <stdio.h>
// #include <stdlib.h>

// #define FLUSH()		fflush(stdout)

// #endif


#undef DEBUG
#undef INFO

/** A structure found at the top of all system allocated
 * memory blocks. It details the usage of the memory block.
 */

struct liballoc_minor;
struct liballoc_major
{
	liballoc_major* prev;		///< Linked list information.
	liballoc_major* next;		///< Linked list information.
	unsigned int pages;			///< The number of pages in the block.
	unsigned int size;			///< The number of pages in the block.
	unsigned int usage;			///< The number of bytes used in the block.
	liballoc_minor* first;		///< A pointer to the first allocated memory in the block.
};


/** This is a structure found at the beginning of all
 * sections in a major block which were allocated by a
 * malloc, calloc, realloc call.
 */
struct	liballoc_minor
{
	liballoc_minor* prev;		///< Linked list information.
	liballoc_minor* next;		///< Linked list information.
	liballoc_major* block;		///< The owning block. A pointer to the major structure.
	unsigned int magic;			///< A magic number to idenfity correctness.
	unsigned int size; 			///< The size of the memory allocated. Could be 1 byte or more.
	unsigned int req_size;		///< The size of memory requested.
};


static struct liballoc_major* l_memRoot = nullptr;	///< The root memory block acquired from the system.
static struct liballoc_major* l_bestBet = nullptr;	///< The major with the most free memory.

static uint32_t l_pageSize			= 0x1000;		///< The size of an individual page. Set up in liballoc_init.
static uint32_t l_pageCount			= 16;			///< The number of pages to request per chunk. Set up in liballoc_init.
static uint64_t l_allocated			= 0;			///< Running total of allocated memory.
static uint64_t l_inuse				= 0;			///< Running total of used memory.

static int64_t l_warningCount		= 0;			///< Number of warnings encountered
static int64_t l_errorCount			= 0;			///< Number of actual errors
static int64_t l_possibleOverruns	= 0;			///< Number of possible overruns





// ***********   HELPER FUNCTIONS  *******************************

#define liballoc_memset Memory::Set
#define liballoc_memcpy Memory::Copy




// ***************************************************************

static struct liballoc_major* allocate_new_page(size_t size)
{
	uint32_t st;
	struct liballoc_major* maj;

	// This is how much space is required.
	st = (uint32_t) size + sizeof(liballoc_major);
	st += sizeof(liballoc_minor);

	if((st % l_pageSize) == 0)		st = st / (l_pageSize);			// Perfect amount of space?
	else							st = st / (l_pageSize) + 1;		// No, add the buffer.


	// Make sure it's >= the minimum size.
	if(st < l_pageCount) st = l_pageCount;

	maj = (liballoc_major*) liballoc_alloc(st);

	if(maj == NULL)
	{
		l_warningCount += 1;
		return nullptr;	// uh oh, we ran out of memory.
	}

	maj->prev	= nullptr;
	maj->next	= nullptr;
	maj->pages	= st;
	maj->size	= st * l_pageSize;
	maj->usage	= sizeof(liballoc_major);
	maj->first	= nullptr;

	l_allocated += maj->size;
	return maj;
}





void* kmalloc(size_t req_size)
{
	int startedBet = 0;
	uint64_t bestSize = 0;
	void* p = nullptr;
	uintptr_t diff;
	liballoc_major* maj;
	liballoc_minor* min;
	liballoc_minor* new_min;
	size_t size = req_size;

	// For alignment, we adjust size so there's enough space to align.
	if(ALIGNMENT > 1)
	{
		size += ALIGNMENT + ALIGN_INFO;
	}

	// So, ideally, we really want an alignment of 0 or 1 in order
	// to save space.
	liballoc_lock();

	if(size == 0)
	{
		l_warningCount += 1;
		liballoc_unlock();
		return kmalloc(1);
	}


	if(l_memRoot == nullptr)
	{
		// This is the first time we are being used.
		l_memRoot = allocate_new_page(size);
		if(l_memRoot == nullptr)
		{
			liballoc_unlock();
			return nullptr;
		}
	}

	// Now we need to bounce through every major and find enough space....

	maj = l_memRoot;
	startedBet = 0;

	// Start at the best bet....
	if(l_bestBet != nullptr)
	{
		bestSize = l_bestBet->size - l_bestBet->usage;

		if(bestSize > (size + sizeof(liballoc_minor)))
		{
			maj = l_bestBet;
			startedBet = 1;
		}
	}

	while(maj != nullptr)
	{
		diff = maj->size - maj->usage;
		// free memory in the block

		if(bestSize < diff)
		{
			// Hmm.. this one has more memory then our bestBet. Remember!
			l_bestBet = maj;
			bestSize = diff;
		}


		#ifdef USE_CASE1

		// CASE 1:  There is not enough space in this major block.
		if(diff < (size + sizeof(liballoc_minor)))
		{
			// Another major block next to this one?
			if(maj->next != nullptr)
			{
				maj = maj->next;		// Hop to that one.
				continue;
			}

			// If we started at the best bet
			if(startedBet == 1)
			{
				// let's start all over again.
				maj = l_memRoot;
				startedBet = 0;
				continue;
			}

			// Create a new major block next to this one and...
			maj->next = allocate_new_page(size);	// next one will be okay.
			if(maj->next == nullptr) break;			// no more memory.

			maj->next->prev = maj;
			maj = maj->next;

			// .. fall through to CASE 2 ..
		}

		#endif
		#ifdef USE_CASE2

		// CASE 2: It's a brand new block.
		if(maj->first == nullptr)
		{
			maj->first = (liballoc_minor*) ((uintptr_t) maj + sizeof(liballoc_major));


			maj->first->magic		= LIBALLOC_MAGIC;
			maj->first->prev		= NULL;
			maj->first->next		= NULL;
			maj->first->block		= maj;
			maj->first->size		= (uint32_t) size;
			maj->first->req_size	= (uint32_t) req_size;
			maj->usage				+= size + sizeof(liballoc_minor);

			l_inuse += size;


			p = (void*) ((uintptr_t) (maj->first) + sizeof(liballoc_minor));
			ALIGN(p);

			liballoc_unlock();		// release the lock
			return p;
		}

		#endif
		#ifdef USE_CASE3


		// CASE 3: Block in use and enough space at the start of the block.
		diff = (uintptr_t) (maj->first);
		diff -= (uintptr_t) maj;
		diff -= sizeof(liballoc_major);

		if(diff >= (size + sizeof(liballoc_minor)))
		{
			// Yes, space in front. Squeeze in.
			maj->first->prev = (liballoc_minor*) ((uintptr_t) maj + sizeof(liballoc_major));
			maj->first->prev->next = maj->first;
			maj->first = maj->first->prev;

			maj->first->magic		= LIBALLOC_MAGIC;
			maj->first->prev		= nullptr;
			maj->first->block		= maj;
			maj->first->size		= (uint32_t) size;
			maj->first->req_size	= (uint32_t) req_size;
			maj->usage				+= size + sizeof(liballoc_minor);

			l_inuse += size;

			p = (void*) ((uintptr_t) (maj->first) + sizeof(liballoc_minor));
			ALIGN(p);

			liballoc_unlock();		// release the lock
			return p;
		}

		#endif
		#ifdef USE_CASE4

		// CASE 4: There is enough space in this block. But is it contiguous?
		min = maj->first;

		// Looping within the block now...
		while(min != nullptr)
		{
				// CASE 4.1: End of minors in a block. Space from last and end?
				if(min->next == nullptr)
				{
					// the rest of this block is free...  is it big enough?
					diff = (uintptr_t) (maj) + maj->size;
					diff -= (uintptr_t) min;
					diff -= sizeof(liballoc_minor);
					diff -= min->size;

					// minus already existing usage..
					if(diff >= (size + sizeof(liballoc_minor)))
					{
						// yay....
						min->next		= (liballoc_minor*) ((uintptr_t) min + sizeof(liballoc_minor) + min->size);
						min->next->prev	= min;
						min				= min->next;
						min->next		= nullptr;
						min->magic		= LIBALLOC_MAGIC;
						min->block		= maj;
						min->size		= (uint32_t) size;
						min->req_size	= (uint32_t) req_size;
						maj->usage		+= size + sizeof(liballoc_minor);

						l_inuse += size;

						p = (void*) ((uintptr_t) min + sizeof(liballoc_minor));
						ALIGN(p);

						liballoc_unlock();		// release the lock
						return p;
					}
				}

				// CASE 4.2: Is there space between two minors?
				if(min->next != nullptr)
				{
					// is the difference between here and next big enough?
					diff = (uintptr_t) (min->next);
					diff -= (uintptr_t) min;
					diff -= sizeof(liballoc_minor);
					diff -= min->size;

					// minus our existing usage.
					if(diff >= (size + sizeof(liballoc_minor)))
					{
						// yay......
						new_min = (liballoc_minor*) ((uintptr_t) min + sizeof(liballoc_minor) + min->size);

						new_min->magic		= LIBALLOC_MAGIC;
						new_min->next		= min->next;
						new_min->prev		= min;
						new_min->size		= (uint32_t) size;
						new_min->req_size	= (uint32_t) req_size;
						new_min->block		= maj;
						min->next->prev		= new_min;
						min->next			= new_min;
						maj->usage			+= size + sizeof(liballoc_minor);

						l_inuse += size;

						p = (void*) ((uintptr_t) new_min + sizeof(liballoc_minor));
						ALIGN(p);

						liballoc_unlock();		// release the lock
						return p;
					}
				}	// min->next != NULL

				min = min->next;

		} // while min != NULL ...


		#endif
		#ifdef USE_CASE5

		// CASE 5: Block full! Ensure next block and loop.
		if(maj->next == nullptr)
		{
			if(startedBet == 1)
			{
				maj = l_memRoot;
				startedBet = 0;
				continue;
			}

			// we've run out. we need more...
			maj->next = allocate_new_page(size);	// next one guaranteed to be okay
			if(maj->next == nullptr) break;			//  uh oh,  no more memory.....

			maj->next->prev = maj;

		}

		#endif

		maj = maj->next;

	} // while (maj != NULL)



	liballoc_unlock();		// release the lock
	return NULL;
}



void kfree(void* ptr)
{
	liballoc_minor* min;
	liballoc_major* maj;

	if(ptr == nullptr)
	{
		l_warningCount += 1;
		return;
	}

	UNALIGN(ptr);
	liballoc_lock();		// lockit

	min = (liballoc_minor*) ((uintptr_t) ptr - sizeof(liballoc_minor));

	if(min->magic != LIBALLOC_MAGIC)
	{
		l_errorCount += 1;

		// Check for overrun errors. For all bytes of LIBALLOC_MAGIC
		if(((min->magic & 0xFFFFFF) == (LIBALLOC_MAGIC & 0xFFFFFF))
			|| ((min->magic & 0xFFFF) == (LIBALLOC_MAGIC & 0xFFFF))
			|| ((min->magic & 0xFF) == (LIBALLOC_MAGIC & 0xFF)))
		{
			l_possibleOverruns += 1;
			Kernel::Log("liballoc: ERROR: Possible 1-3 byte overrun for magic %x != %x\n", min->magic, LIBALLOC_MAGIC);
		}


		if(min->magic == LIBALLOC_DEAD)
		{
			Kernel::Log("liballoc: ERROR: multiple PREFIX(free)() attempt on %x from %x.\n", ptr, __builtin_return_address(0));
		}
		else
		{
			Kernel::Log("liballoc: ERROR: Bad PREFIX(free)( %x ) called from %x\n", ptr, __builtin_return_address(0));
		}

		// being lied to...
		liballoc_unlock();		// release the lock
		return;
	}

	maj = min->block;
	l_inuse -= min->size;

	maj->usage -= (min->size + sizeof(liballoc_minor));
	min->magic = LIBALLOC_DEAD;		// No mojo.

	if(min->next != nullptr) min->next->prev = min->prev;
	if(min->prev != nullptr) min->prev->next = min->next;
	if(min->prev == nullptr) maj->first = min->next;

	// Might empty the block. This was the first
	// minor.


	// We need to clean up after the majors now....

	if(maj->first == nullptr)	// Block completely unused.
	{
		if(l_memRoot == maj) l_memRoot = maj->next;
		if(l_bestBet == maj) l_bestBet = nullptr;

		if(maj->prev != nullptr) maj->prev->next = maj->next;
		if(maj->next != nullptr) maj->next->prev = maj->prev;
		l_allocated -= maj->size;

		liballoc_free(maj, maj->pages);
	}
	else
	{
		if(l_bestBet != NULL)
		{
			int bestSize = l_bestBet->size - l_bestBet->usage;
			int majSize = maj->size - maj->usage;

			if(majSize > bestSize) l_bestBet = maj;
		}

	}

	liballoc_unlock();		// release the lock
}





void* kcalloc(size_t nobj, size_t size)
{
	size_t real_size;
	void* p;

	real_size = nobj * size;

	p = kmalloc(real_size);
	liballoc_memset(p, 0, real_size);

	return p;
}



void* krealloc(void *p, size_t size)
{
	void *ptr;
	struct liballoc_minor *min;
	unsigned int real_size;

	// Honour the case of size == 0 => free old and return NULL
	if(size == 0)
	{
		kfree(p);
		return nullptr;
	}

	// In the case of a NULL pointer, return a simple malloc.
	if(p == nullptr) return kmalloc(size);

	// Unalign the pointer if required.
	ptr = p;
	UNALIGN(ptr);

	liballoc_lock();		// lockit

	min = (liballoc_minor*) ((uintptr_t) ptr - sizeof(liballoc_minor));

	// Ensure it is a valid structure.
	if(min->magic != LIBALLOC_MAGIC)
	{
		l_errorCount += 1;

		// Check for overrun errors. For all bytes of LIBALLOC_MAGIC
		if(((min->magic & 0xFFFFFF) == (LIBALLOC_MAGIC & 0xFFFFFF))
			|| ((min->magic & 0xFFFF) == (LIBALLOC_MAGIC & 0xFFFF))
			|| ((min->magic & 0xFF) == (LIBALLOC_MAGIC & 0xFF)))
		{
			l_possibleOverruns += 1;
			Kernel::Log("liballoc: ERROR: Possible 1-3 byte overrun for magic %x != %x\n", min->magic, LIBALLOC_MAGIC);
		}


		if(min->magic == LIBALLOC_DEAD)
		{
			Kernel::Log("liballoc: ERROR: multiple free() attempt on %x from %x.\n", ptr, __builtin_return_address(0));
		}
		else
		{
			Kernel::Log("liballoc: ERROR: free(%x) called from %x\n", ptr, __builtin_return_address(0));
		}

		// being lied to...
		liballoc_unlock();		// release the lock
		return NULL;
	}

	// Definitely a memory block.
	real_size = min->req_size;

	if(real_size >= size)
	{
		min->req_size = (uint32_t) size;
		liballoc_unlock();
		return p;
	}

	liballoc_unlock();

	// If we got here then we're reallocating to a block bigger than us.
	ptr = kmalloc(size);					// We need to allocate new memory
	liballoc_memcpy(ptr, p, real_size);
	kfree(p);

	return ptr;
}



















int liballoc_lock()
{
	return 0;
}

int liballoc_unlock()
{
	return 0;
}

void* liballoc_alloc(size_t size)
{
	return (void*) Kernel::HardwareAbstraction::MemoryManager::Virtual::AllocatePage(size);
}

int liballoc_free(void* page, size_t size)
{
	Kernel::HardwareAbstraction::MemoryManager::Virtual::FreePage((uint64_t) page, size);
	return 0;
}















