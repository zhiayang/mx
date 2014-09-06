// Boot.s
// Copyright (c) 2011-2013, zhiayang@gmail.com
// With reference to Sortix: (c) Jonas 'Sortie' Termansen 2011.
// Licensed under the Apache License Version 2.0.



// Sets up Long Mode, Paging and GDT.
// Calls KernelBootStrap (kbootstrap.s)

// Referenced Files:
// src/loader/kbootstrap.s



.global Boot

.section .text
.text 0x00100000
.type Boot, @function












.code32

Boot:
	jmp Prep64



// Multiboot 1 Header
.align 4
MultibootHeader:
	.long 0x1BADB002					// Magic Number
	.long 0x00000003					// Flags	(bits 0, 1)
	.long -(0x1BADB002 + 0x00000003)	// Checksum





Prep64:
	cli
	cld


	// Store magic value (0x2BADB002)
	mov %eax, 0x0500


	// Copy the multiboot structure to 0x40000.
	mov %ebx, %esi
	mov $0x40000, %edi
	mov $0x200, %ecx
	rep movsb

	// copy the memory map to 0x40200
	mov 48(%ebx), %esi
	mov $0x40200, %edi
	mov $0x500, %ecx
	rep movsb




	// This is confusing.
	// PML4T (256TB) -> 512x PDPT (512GB) -> 512x PD (1GB) -> 512x PT (2MB) -> 512x Pages (4KB)




	// Clear 0xC000 bytes after 0x3000
	movl $0x3000, %edi
	mov %edi, %cr3
	xorl %eax, %eax
	movl $0xC000, %ecx

	rep stosl
	movl %cr3, %edi




	// Set initial page tables.
	// Will re-create in long mode.


	// OR with 0x3 (R/W, Present)
	// Point the first entry in the PML4T to the first PDPT.

	movl $0x4007, (%edi)		// Make put the address + flags of the PDPT into 0x1000.
	addl $0x1000, %edi			// We only want 1 PDPT, so we add 0x1000 to make it point to the PDPT entries.


	// Point the first PDPT entry to the first PD.

	movl $0x5007, (%edi)		// Same deal, point PDPT entry number one to the PD we create below.
	addl $0x1000, %edi			// Point to the PD now.


	// PD

	movl $0x6007, (%edi)		// Exactly the same as above. Except:
	movl $0x7007, 8(%edi)		// There's a page table at 0x6000, 0x1000 long. There's another one at 0x5000. We address 4MB this way.
	movl $0x8007, 16(%edi)		// 6 MB
	movl $0x9007, 24(%edi)		// 8 MB
	movl $0xA007, 32(%edi)		// 10 MB
	movl $0xB007, 40(%edi)		// 12 MB
	movl $0xC007, 48(%edi)		// 14 MB
	movl $0xD007, 56(%edi)		// 16 MB
	addl $0x1000, %edi			// Reference the page table at 0x6000.


	// PT
	movl $0x07, %ebx			// Set our flags.
	movl $4096, %ecx			// Since the page tables are contiguous, we can simply loop 4096 times (512 x 8).


	// Memory Map 4MB
SetPTEntry:
	mov %ebx, (%edi)
	add $0x1000, %ebx
	add $8, %edi

	loop SetPTEntry





	// Enable PAE Paging
	mov %cr4, %eax
	orl $0x20, %eax						// Set PAE Bit
	mov %eax, %cr4


	// Enable Long Mode
	mov $0xC0000080, %ecx
	rdmsr

	// enable syscall/sysret
	// also enable the NX feature.
	orl $0x901, %eax
	wrmsr


	// Enable Paging, enter compatibility mode
	mov %cr0, %eax
	orl $0x80000000, %eax
	mov %eax, %cr0

	// Load Long Mode GDT
	mov GDT64Pointer, %eax
	lgdtl GDT64Pointer


	// Jump!
	ljmp $0x8, $Realm64


.code64
Realm64:
	cli

	// Set up segments
	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %gs

	mov $0x2B, %ax
	mov %ax, %fs


	mov $0x30, %ax
	ltr %ax

	// Jump to kernel bootstrap!
	jmp Main





Main:
	jmp KernelBootStrap					// SecondStage.s
	cli
	hlt












































