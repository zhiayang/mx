// MachO.hpp
// Copyright (c) 2011-2013, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#pragma once
#include <stdint.h>

typedef int	cpu_type_t;
typedef int	cpu_subtype_t;
typedef int	cpu_threadtype_t;




// CPU TYPES
#define	CPU_ARCH_MASK				0xFF000000	// mask for architecture bits
#define CPU_ARCH_ABI64				0x01000000	// 64 bit ABI


// MACHINE TYPES
#define CPU_TYPE_ANY				((cpu_type_t) -1)
#define CPU_TYPE_X86				((cpu_type_t) 7)
#define CPU_TYPE_I386				CPU_TYPE_X86
#define	CPU_TYPE_X86_64				(CPU_TYPE_X86 | CPU_ARCH_ABI64)
#define CPU_TYPE_ARM				((cpu_type_t) 12)
#define CPU_TYPE_ARM64				(CPU_TYPE_ARM | CPU_ARCH_ABI64)


// SUBTYPES
#define	CPU_SUBTYPE_MULTIPLE		((cpu_subtype_t) -1)
#define CPU_SUBTYPE_LITTLE_ENDIAN	((cpu_subtype_t) 0)
#define CPU_SUBTYPE_BIG_ENDIAN		((cpu_subtype_t) 1)






struct mach_header_64
{
	uint32_t magic;					// mach magic number identifier
	cpu_type_t cputype;				// cpu specifier
	cpu_subtype_t cpusubtype;		// machine specifier
	uint32_t filetype;				// type of file
	uint32_t ncmds;					// number of load commands
	uint32_t sizeofcmds;			// the size of all the load commands
	uint32_t flags;					// flags
	uint32_t reserved;				// reserved
};

#define MH_MAGIC_64					0xFEEDFACF	// the 64-bit mach magic number
#define MH_CIGAM_64					0xCFFAEDFE	// NXSwapInt(MH_MAGIC_64)

// FILETYPES
#define	MH_OBJECT					0x1		// relocatable object file
#define	MH_EXECUTE					0x2		// demand paged executable file
#define	MH_FVMLIB					0x3		// fixed VM shared library file
#define	MH_CORE						0x4		// core file
#define	MH_PRELOAD					0x5		// preloaded executable file
#define	MH_DYLIB					0x6		// dynamically bound shared library
#define	MH_DYLINKER					0x7		// dynamic link editor
#define	MH_BUNDLE					0x8		// dynamically bound bundle file
#define	MH_DYLIB_STUB				0x9		// shared library stub for static linking only, no section contents
#define	MH_DSYM						0xA		// companion file with only debug sections
#define	MH_KEXT_BUNDLE				0xB		// x86_64 kexts


// FLAGS
#define MH_NOUNDEFS					0x1
#define MH_INCRLINK					0x2
#define MH_DYLDLINK					0x4
#define MH_BINDATLOAD				0x8
#define MH_PREBOUND					0x10
#define MH_SPLIT_SEGS				0x20
#define MH_TWOLEVEL					0x80
#define MH_FORCE_FLAT				0x100
#define MH_NOMULTIDEFS				0x200
#define MH_NOFIXPREBINDING			0x400
#define MH_PREBINDABLE				0x800
#define MH_ALLMODSBOUND				0x1000
#define MH_SUBSECTIONS_VIA_SYMBOLS	0x2000
#define MH_CANONICAL				0x4000
#define MH_WEAK_DEFINES				0x8000
#define MH_BINDS_TO_WEAK			0x10000
#define MH_ALLOW_STACK_EXECUTION	0x20000
#define MH_ROOT_SAFE				0x40000
#define MH_SETUID_SAFE				0x80000
#define MH_NO_REEXPORTED_DYLIBS		0x100000
#define	MH_PIE						0x200000
#define	MH_DEAD_STRIPPABLE_DYLIB	0x400000
#define MH_HAS_TLV_DESCRIPTORS		0x800000
#define MH_NO_HEAP_EXECUTION		0x1000000
#define MH_APP_EXTENSION_SAFE		0x02000000









struct load_command
{
	uint32_t cmd;		// type of load command
	uint32_t cmdsize;	// total size of command in bytes
};

#define LC_REQ_DYLD					0x80000000

// CMD TYPES
#define	LC_SEGMENT					0x1						// segment of this file to be mapped
#define	LC_SYMTAB					0x2						// link-edit stab symbol table info
#define	LC_SYMSEG					0x3						// link-edit gdb symbol table info (obsolete)
#define	LC_THREAD					0x4						// thread
#define	LC_UNIXTHREAD				0x5						// unix thread (includes a stack)
#define	LC_LOADFVMLIB				0x6						// load a specified fixed VM shared library
#define	LC_IDFVMLIB					0x7						// fixed VM shared library identification
#define	LC_IDENT					0x8						// object identification info (obsolete)
#define LC_FVMFILE					0x9						// fixed VM file inclusion (internal use)
#define LC_PREPAGE					0xA						// prepage command (internal use)
#define	LC_DYSYMTAB					0xB						// dynamic link-edit symbol table info
#define	LC_LOAD_DYLIB				0xC						// load a dynamically linked shared library
#define	LC_ID_DYLIB					0xD						// dynamically linked shared lib ident
#define LC_LOAD_DYLINKER			0xE						// load a dynamic linker
#define LC_ID_DYLINKER				0xF						// dynamic linker identification
#define	LC_PREBOUND_DYLIB			0x10					// modules prebound for a dynamically linked shared library
#define	LC_ROUTINES					0x11					// image routines
#define	LC_SUB_FRAMEWORK			0x12					// sub framework
#define	LC_SUB_UMBRELLA				0x13					// sub umbrella
#define	LC_SUB_CLIENT				0x14					// sub client
#define	LC_SUB_LIBRARY 				0x15					// sub library
#define	LC_TWOLEVEL_HINTS			0x16					// two-level namespace lookup hints
#define	LC_PREBIND_CKSUM 			0x17					// prebind checksum
#define	LC_LOAD_WEAK_DYLIB			(0x18 | LC_REQ_DYLD)	// load a dynamically linked shared library that is allowed to be missing
#define	LC_SEGMENT_64				0x19					// 64-bit segment of this file to be mapped
#define	LC_ROUTINES_64				0x1A					// 64-bit image routines
#define LC_UUID						0x1B					// the uuid
#define LC_RPATH					(0x1C | LC_REQ_DYLD)	// runpath additions
#define LC_CODE_SIGNATURE			0x1D					// local of code signature
#define LC_SEGMENT_SPLIT_INFO		0x1e					// local of info to split segments
#define LC_REEXPORT_DYLIB			(0x1f | LC_REQ_DYLD)	// load and re-export dylib
#define	LC_LAZY_LOAD_DYLIB			0x20					// delay load of dylib until first use
#define	LC_ENCRYPTION_INFO			0x21					// encrypted segment information
#define	LC_DYLD_INFO				0x22					// compressed dyld information
#define	LC_DYLD_INFO_ONLY			(0x22 | LC_REQ_DYLD)	// compressed dyld information only
#define	LC_LOAD_UPWARD_DYLIB		(0x23 | LC_REQ_DYLD)	// load upward dylib
#define LC_VERSION_MIN_MACOSX		0x24					// build for MacOSX min OS version
#define LC_VERSION_MIN_IPHONEOS		0x25					// build for iPhoneOS min OS version
#define LC_FUNCTION_STARTS			0x26					// compressed table of function start addresses
#define LC_DYLD_ENVIRONMENT			0x27					// string for dyld to treat like environment variable
#define LC_MAIN						(0x28 | LC_REQ_DYLD)	// replacement for LC_UNIXTHREAD
#define LC_DATA_IN_CODE				0x29					// table of non-instructions in __text
#define LC_SOURCE_VERSION			0x2A					// source version used to build binary
#define LC_DYLIB_CODE_SIGN_DRS		0x2B					// Code signing DRs copied from linked dylibs
#define	LC_ENCRYPTION_INFO_64		0x2C					// 64-bit encrypted segment information
#define LC_LINKER_OPTION			0x2D					// linker options in MH_OBJECT files
#define LC_LINKER_OPTIMIZATION_HINT	0x2E					// optimization hints in MH_OBJECT files



// for Mach.
typedef int vm_prot_t;
#define	VM_PROT_NONE		((vm_prot_t) 0x00)
#define VM_PROT_READ		((vm_prot_t) 0x01)
#define VM_PROT_WRITE		((vm_prot_t) 0x02)
#define VM_PROT_EXECUTE		((vm_prot_t) 0x04)
#define VM_PROT_DEFAULT		(VM_PROT_READ | VM_PROT_WRITE)
#define VM_PROT_ALL			(VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE)
#define VM_PROT_NO_CHANGE	((vm_prot_t) 0x08)
#define VM_PROT_COPY		((vm_prot_t) 0x10)
#define VM_PROT_WANTS_COPY	((vm_prot_t) 0x10)
#define VM_PROT_IS_MASK		((vm_prot_t) 0x40)

struct segment_command_64
{
	uint32_t cmd;				// LC_SEGMENT_64
	uint32_t cmdsize;			// includes sizeof section_64 structs
	char segname[16];			// segment name
	uint64_t vmaddr;			// memory address of this segment
	uint64_t vmsize;			// memory size of this segment
	uint64_t fileoff;			// file offset of this segment
	uint64_t filesize;			// amount to map from the file
	vm_prot_t maxprot;			// maximum VM protection
	vm_prot_t initprot;			// initial VM protection
	uint32_t nsects;			// number of sections in segment
	uint32_t flags;				// flags
};

// SEGMENT FLAGS
#define	SG_HIGHVM				0x1
#define	SG_FVMLIB				0x2
#define	SG_NORELOC				0x4
#define SG_PROTECTED_VERSION_1	0x8





struct section_64
{
	char sectname[16];			// name of this section
	char segname[16];			// segment this section goes in
	uint64_t addr;				// memory address of this section
	uint64_t size;				// size in bytes of this section
	uint32_t offset;			// file offset of this section
	uint32_t align;				// section alignment (power of 2)
	uint32_t reloff;			// file offset of relocation entries
	uint32_t nreloc;			// number of relocation entries
	uint32_t flags;				// flags (section type and attributes)
	uint32_t reserved1;			// reserved (for offset or index)
	uint32_t reserved2;			// reserved (for count or sizeof)
	uint32_t reserved3;			// reserved
};

// SECTION TYPES AND ATTRIBS
#define SECTION_TYPE					0x000000FF	// 256 section types
#define SECTION_ATTRIBUTES				0xFFFFFF00	// 24 section attributes

// SECTION TYPES
#define	S_REGULAR						0x0		// regular section
#define	S_ZEROFILL						0x1		// zero fill on demand section
#define	S_CSTRING_LITERALS				0x2		// section with only literal C string
#define	S_4BYTE_LITERALS				0x3		// section with only 4 byte litera
#define	S_8BYTE_LITERALS				0x4		// section with only 8 byte litera
#define	S_LITERAL_POINTERS				0x5		// section with only pointers to litera
#define	S_NON_LAZY_SYMBOL_POINTERS		0x6		// section with only non-lazy symbol pointers
#define	S_LAZY_SYMBOL_POINTERS			0x7		// section with only lazy symbol pointers
#define	S_SYMBOL_STUBS					0x8		// section with only symbol stubs, byte size of stub in the reserved2 field
#define	S_MOD_INIT_FUNC_POINTERS		0x9		// section with only function pointers for initialization
#define	S_MOD_TERM_FUNC_POINTERS		0xA		// section with only function pointers for termination
#define	S_COALESCED						0xB		// section contains symbols that are to be coalesced
#define	S_GB_ZEROFILL					0xC		// zero fill on demand section (that can be larger than 4 gigabytes)
#define	S_INTERPOSING					0xD		// section with only pairs of function pointers for interposing
#define	S_16BYTE_LITERALS				0xE		// section with only 16 byte literals
#define	S_DTRACE_DOF					0xF		// section contains DTrace Object Format
#define	S_LAZY_DYLIB_SYMBOL_POINTERS	0x10	// section with only lazy symbol pointers to lazy loaded dylibs

// THREAD LOCAL VARS TYPES
#define S_THREAD_LOCAL_REGULAR					0x11	// template of initial values for TLVs
#define S_THREAD_LOCAL_ZEROFILL					0x12	// template of initial values for TLVs
#define S_THREAD_LOCAL_VARIABLES				0x13	// TLV descriptors
#define S_THREAD_LOCAL_VARIABLE_POINTERS		0x14	// pointers to TLV descriptors
#define S_THREAD_LOCAL_INIT_FUNCTION_POINTERS	0x15	// functions to call to initialize TLV values


// SECTION ATTRIBUTES
#define SECTION_ATTRIBUTES_USR			0xFF000000	// User setable attributes
#define S_ATTR_PURE_INSTRUCTIONS		0x80000000	// section contains only true machine instructions
#define S_ATTR_NO_TOC					0x40000000	// section contains coalesced symbols that are not to be in a ranlib table of contents
#define S_ATTR_STRIP_STATIC_SYMS		0x20000000	// ok to strip static symbols in this section in files with the MH_DYLDLINK flag
#define S_ATTR_NO_DEAD_STRIP			0x10000000	// no dead stripping
#define S_ATTR_LIVE_SUPPORT				0x08000000	// blocks are live if they reference live blocks
#define S_ATTR_SELF_MODIFYING_CODE		0x04000000	// Used with i386 code stubs written on by dyld

#define	S_ATTR_DEBUG					0x02000000	// a debug section
#define SECTION_ATTRIBUTES_SYS			0x00FFFF00	// system setable attributes
#define S_ATTR_SOME_INSTRUCTIONS		0x00000400	// section contains some machine instructions
#define S_ATTR_EXT_RELOC				0x00000200	// section has external relocation entries
#define S_ATTR_LOC_RELOC				0x00000100	// section has local relocation entries


// SECTION AND SEGMENT NAMES
#define	SEG_PAGEZERO		"__PAGEZERO"		// the pagezero segment catches null derefs
#define	SEG_TEXT			"__TEXT"			// the tradition UNIX text segment
#define	SECT_TEXT			"__text"			// the real text part of the text section; no headers, and no padding
#define SECT_FVMLIB_INIT0	"__fvmlib_init0"	// the fvmlib initialisation section
#define SECT_FVMLIB_INIT1	"__fvmlib_init1"	// the section following the fvmlib initialisation section

#define	SEG_DATA			"__DATA"			// the tradition UNIX data segment
#define	SECT_DATA			"__data"			// the real initialised data section; no padding, no bss overlap
#define	SECT_BSS			"__bss"				// the real uninitialised data section; no padding
#define SECT_COMMON			"__common"			// the section common symbols are allocated in by the link editor
#define	SEG_OBJC			"__OBJC"			// objective-C runtime segment
#define SECT_OBJC_SYMBOLS	"__symbol_table"	// symbol table
#define SECT_OBJC_MODULES	"__module_info"		// module information
#define SECT_OBJC_STRINGS	"__selector_strs"	// string table
#define SECT_OBJC_REFS		"__selector_refs"	// string table

#define	SEG_ICON			"__ICON"			// the icon segment
#define	SECT_ICON_HEADER	"__header"			// the icon headers
#define	SECT_ICON_TIFF		"__tiff"			// the icons in tiff format
#define	SEG_LINKEDIT		"__LINKEDIT"		// the segment containing all structs created and maintained by the link editor.
#define SEG_UNIXSTACK		"__UNIXSTACK"		// the unix stack segment

#define SEG_IMPORT			"__IMPORT"			// the segment for the self (dyld) modifing code stubs that has RWX perms

/*
 * Thread commands contain machine-specific data structures suitable for
 * use in the thread state primitives. The machine specific data structures
 * follow the struct thread_command as follows.
 * Each flavor of machine specific data structure is preceded by an unsigned
 * long constant for the flavor of that data structure, an uint32_t
 * that is the count of longs of the size of the state data structure and then
 * the state data structure follows. This triple may be repeated for many
 * flavors. The constants for the flavors, counts and state data structure
 * definitions are expected to be in the header file <machine/thread_status.h>.
 * These machine specific data structures sizes must be multiples of
 * 4 bytes The cmdsize reflects the total size of the thread_command
 * and all of the sizes of the constants for the flavors, counts and state
 * data structures.
 *
 * For executable objects that are unix processes there will be one
 * thread_command (cmd == LC_UNIXTHREAD) created for it by the link-editor.
 * This is the same as a LC_THREAD, except that a stack is automatically
 * created (based on the shell's limit for the stack size). Command arguments
 * and environment variables are copied onto that stack.
 */

struct thread_command
{
	uint32_t	cmd;		// LC_THREAD or LC_UNIXTHREAD
	uint32_t	cmdsize;	// total size of this command
	/* uint32_t flavour		flavour of thread state */
	/* uint32_t count		  count of longs in thread state */
	/* struct XXX_thread_state state  thread state for this flavour */
	/* ... */
};









