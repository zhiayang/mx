// CPUID.cpp
// Copyright (c) 2014 - 2016, zhiayang@gmail.com
// Licensed under Creative Commons Attribution ShareAlike 3.0 Unported.


#include <stdint.h>
#include <StandardIO.hpp>
#include <Kernel.hpp>
#include <HardwareAbstraction/CPUID.hpp>

namespace Kernel {
namespace HardwareAbstraction {
namespace CPUID
{
	static inline void ExecuteCPUID(uint32_t Function, uint32_t* EAX, uint32_t* EBX, uint32_t* ECX, uint32_t* EDX)
	{
		asm volatile("mov %[func], %%eax; cpuid; mov %%eax, %[a]; mov %%ebx, %[b]; mov %%ecx, %[c]; mov %%edx, %[d]" : [a]"=r"(*EAX), [b]"=r"(*EBX), [c]"=r"(*ECX), [d]"=r"(*EDX) : [func]"r"(Function) : "%rax", "%rbx", "%rcx", "%rdx");
	}

	CPUIDData* Initialise(CPUIDData* CPUIDInfo)
	{
		uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;
		ExecuteCPUID(0, &eax, &ebx, &ecx, &edx);

		uint32_t maxf = eax;

		CPUIDInfo = new CPUIDData();

		// fetch vendor ID.
		CPUIDInfo->_VendorID[0] = (char)((ebx >> 0) & 0xFF);
		CPUIDInfo->_VendorID[1] = (char)((ebx >> 8) & 0xFF);
		CPUIDInfo->_VendorID[2] = (char)((ebx >> 16) & 0xFF);
		CPUIDInfo->_VendorID[3] = (char)((ebx >> 24) & 0xFF);

		CPUIDInfo->_VendorID[4] = (char)((edx >> 0) & 0xFF);
		CPUIDInfo->_VendorID[5] = (char)((edx >> 8) & 0xFF);
		CPUIDInfo->_VendorID[6] = (char)((edx >> 16) & 0xFF);
		CPUIDInfo->_VendorID[7] = (char)((edx >> 24) & 0xFF);

		CPUIDInfo->_VendorID[8] = (char)((ecx >> 0) & 0xFF);
		CPUIDInfo->_VendorID[9] = (char)((ecx >> 8) & 0xFF);
		CPUIDInfo->_VendorID[10] = (char)((ecx >> 16) & 0xFF);
		CPUIDInfo->_VendorID[11] = (char)((ecx >> 24) & 0xFF);
		CPUIDInfo->_VendorID[12] = 0;

		ExecuteCPUID(1, &eax, &ebx, &ecx, &edx);
		CPUIDInfo->_Features_EDX = edx;
		CPUIDInfo->_Features_ECX = ecx;

		CPUIDInfo->_Stepping = eax & 0xF;
		CPUIDInfo->_ProcessorType = eax & 0x3000;

		uint32_t m1 = eax & 0xF0;
		uint32_t f1 = eax & 0xF00;

		// check if we're an intel chip
		if(CPUIDInfo->_VendorID[0] == 'G' || f1 == 0xF00)
		{
			CPUIDInfo->_Model = m1 + ((eax & 0xF0000) << 4);
			CPUIDInfo->_Family = (f1 >> 8) + ((eax & 0x0FF00000) >> 20);
		}
		else
		{
			CPUIDInfo->_Family = (f1 >> 8);
			CPUIDInfo->_Model = m1 >> 8;
		}

		// see if we can use function seven.
		if(maxf >= 7)
		{
			ExecuteCPUID(7, &eax, &ebx, &ecx, &edx);
			CPUIDInfo->_Features_Extended = ebx;
		}

		uint32_t maxe = 0;
		ExecuteCPUID(0x80000000, &eax, &ebx, &ecx, &edx);
		maxe = eax;
		if(maxe >= 0x80000001)
		{
			// extended features.
			ExecuteCPUID(0x80000001, &eax, &ebx, &ecx, &edx);
			CPUIDInfo->_Extended_EDX = edx;
			CPUIDInfo->_Extended_ECX = ecx;
		}
		else
		{
			CPUIDInfo->_Extended_ECX = 0;
			CPUIDInfo->_Extended_EDX = 0;
		}
		if(maxe >= 0x80000004)
		{
			// brand string.
			ExecuteCPUID(0x80000002, &eax, &ebx, &ecx, &edx);
			CPUIDInfo->_BrandString[0] = (char)((eax >> 0) & 0xFF);
			CPUIDInfo->_BrandString[1] = (char)((eax >> 8) & 0xFF);
			CPUIDInfo->_BrandString[2] = (char)((eax >> 16) & 0xFF);
			CPUIDInfo->_BrandString[3] = (char)((eax >> 24) & 0xFF);

			CPUIDInfo->_BrandString[4] = (char)((ebx >> 0) & 0xFF);
			CPUIDInfo->_BrandString[5] = (char)((ebx >> 8) & 0xFF);
			CPUIDInfo->_BrandString[6] = (char)((ebx >> 16) & 0xFF);
			CPUIDInfo->_BrandString[7] = (char)((ebx >> 24) & 0xFF);

			CPUIDInfo->_BrandString[8] = (char)((ecx >> 0) & 0xFF);
			CPUIDInfo->_BrandString[9] = (char)((ecx >> 8) & 0xFF);
			CPUIDInfo->_BrandString[10] = (char)((ecx >> 16) & 0xFF);
			CPUIDInfo->_BrandString[11] = (char)((ecx >> 24) & 0xFF);

			CPUIDInfo->_BrandString[12] = (char)((edx >> 0) & 0xFF);
			CPUIDInfo->_BrandString[13] = (char)((edx >> 8) & 0xFF);
			CPUIDInfo->_BrandString[14] = (char)((edx >> 16) & 0xFF);
			CPUIDInfo->_BrandString[15] = (char)((edx >> 24) & 0xFF);


			ExecuteCPUID(0x80000003, &eax, &ebx, &ecx, &edx);
			CPUIDInfo->_BrandString[16] = (char)((eax >> 0) & 0xFF);
			CPUIDInfo->_BrandString[17] = (char)((eax >> 8) & 0xFF);
			CPUIDInfo->_BrandString[18] = (char)((eax >> 16) & 0xFF);
			CPUIDInfo->_BrandString[19] = (char)((eax >> 24) & 0xFF);

			CPUIDInfo->_BrandString[20] = (char)((ebx >> 0) & 0xFF);
			CPUIDInfo->_BrandString[21] = (char)((ebx >> 8) & 0xFF);
			CPUIDInfo->_BrandString[22] = (char)((ebx >> 16) & 0xFF);
			CPUIDInfo->_BrandString[23] = (char)((ebx >> 24) & 0xFF);

			CPUIDInfo->_BrandString[24] = (char)((ecx >> 0) & 0xFF);
			CPUIDInfo->_BrandString[25] = (char)((ecx >> 8) & 0xFF);
			CPUIDInfo->_BrandString[26] = (char)((ecx >> 16) & 0xFF);
			CPUIDInfo->_BrandString[27] = (char)((ecx >> 24) & 0xFF);

			CPUIDInfo->_BrandString[28] = (char)((edx >> 0) & 0xFF);
			CPUIDInfo->_BrandString[29] = (char)((edx >> 8) & 0xFF);
			CPUIDInfo->_BrandString[30] = (char)((edx >> 16) & 0xFF);
			CPUIDInfo->_BrandString[31] = (char)((edx >> 24) & 0xFF);


			ExecuteCPUID(0x80000004, &eax, &ebx, &ecx, &edx);
			CPUIDInfo->_BrandString[32] = (char)((eax >> 0) & 0xFF);
			CPUIDInfo->_BrandString[33] = (char)((eax >> 8) & 0xFF);
			CPUIDInfo->_BrandString[34] = (char)((eax >> 16) & 0xFF);
			CPUIDInfo->_BrandString[35] = (char)((eax >> 24) & 0xFF);

			CPUIDInfo->_BrandString[36] = (char)((ebx >> 0) & 0xFF);
			CPUIDInfo->_BrandString[37] = (char)((ebx >> 8) & 0xFF);
			CPUIDInfo->_BrandString[38] = (char)((ebx >> 16) & 0xFF);
			CPUIDInfo->_BrandString[39] = (char)((ebx >> 24) & 0xFF);

			CPUIDInfo->_BrandString[40] = (char)((ecx >> 0) & 0xFF);
			CPUIDInfo->_BrandString[41] = (char)((ecx >> 8) & 0xFF);
			CPUIDInfo->_BrandString[42] = (char)((ecx >> 16) & 0xFF);
			CPUIDInfo->_BrandString[43] = (char)((ecx >> 24) & 0xFF);

			CPUIDInfo->_BrandString[44] = (char)((edx >> 0) & 0xFF);
			CPUIDInfo->_BrandString[45] = (char)((edx >> 8) & 0xFF);
			CPUIDInfo->_BrandString[46] = (char)((edx >> 16) & 0xFF);
			CPUIDInfo->_BrandString[47] = (char)((edx >> 24) & 0xFF);
			CPUIDInfo->_BrandString[48] = 0;
		}

		// print the info to log.
		Log("CPU Vendor: %s", CPUIDInfo->VendorID());
		Log("Family: %x, Model: %x, Stepping: %x", CPUIDInfo->Family(), CPUIDInfo->Model(), CPUIDInfo->Stepping());
		Log("Brand Identifier: %s", CPUIDInfo->BrandString()[0] == 0 ? "Not supported" : CPUIDInfo->BrandString());

		return CPUIDInfo;
	}
}
}
}






