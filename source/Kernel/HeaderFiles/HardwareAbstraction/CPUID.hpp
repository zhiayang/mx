// CPUID.hpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under Creative Commons Attribution ShareAlike 3.0 Unported.


#include <stdint.h>
#pragma once

namespace Kernel {
namespace HardwareAbstraction {
namespace CPUID
{
	class CPUIDData
	{
		public:
			const char* VendorID()				{ return this->_VendorID; }
			const char* BrandString()			{ return this->_BrandString; }
			uint32_t Family()					{ return this->_Family; }
			uint32_t Model()					{ return this->_Model; }
			uint32_t Stepping()					{ return this->_Stepping; }

			// behold: functions.

			// EDX flags, EAX = 1
			bool Onboardx87FPU()				{ return this->_Features_EDX & (1 <<  0); }
			bool Virtual8086()					{ return this->_Features_EDX & (1 <<  1); }
			bool DebugExtensions()				{ return this->_Features_EDX & (1 <<  2); }
			bool PageSizeExtension()			{ return this->_Features_EDX & (1 <<  3); }
			bool TimeStampCounter()				{ return this->_Features_EDX & (1 <<  4); }
			bool ModelSpecificRegisters()		{ return this->_Features_EDX & (1 <<  5); }
			bool PhysicalAddressExtensions()	{ return this->_Features_EDX & (1 <<  6); }
			bool MachineCheckException()		{ return this->_Features_EDX & (1 <<  7); }
			bool CompareAndSwap()				{ return this->_Features_EDX & (1 <<  8); }
			bool OnboardAPIC()					{ return this->_Features_EDX & (1 <<  9); }
			// 10 is reserved.
			bool SysenterAndSysexit()			{ return this->_Features_EDX & (1 << 11); }
			bool MemoryTypeRange()				{ return this->_Features_EDX & (1 << 12); }
			bool PageGlobalBit()				{ return this->_Features_EDX & (1 << 13); }
			bool MachineCheckArchitecture()		{ return this->_Features_EDX & (1 << 14); }
			bool ConditionalMove()				{ return this->_Features_EDX & (1 << 15); }
			bool PageAttributeTable()			{ return this->_Features_EDX & (1 << 16); }
			bool PSE36Bit()						{ return this->_Features_EDX & (1 << 17); }
			bool ProcessorSerialNumber()		{ return this->_Features_EDX & (1 << 18); }
			bool CLFlush()						{ return this->_Features_EDX & (1 << 19); }
			// 20 is reserved.
			bool DebugStore()					{ return this->_Features_EDX & (1 << 21); }
			bool OnboardThermalControl()		{ return this->_Features_EDX & (1 << 22); }
			bool MMXInstructions()				{ return this->_Features_EDX & (1 << 23); }
			bool FXStateSave()					{ return this->_Features_EDX & (1 << 24); }
			bool SSEInstructions()				{ return this->_Features_EDX & (1 << 25); }
			bool SSE2Instructions()				{ return this->_Features_EDX & (1 << 26); }
			bool CacheSelfSnoop()				{ return this->_Features_EDX & (1 << 27); }
			bool Hyperthreading()				{ return this->_Features_EDX & (1 << 28); }
			bool ThermalRegulation()			{ return this->_Features_EDX & (1 << 29); }
			bool IntelItanium64()				{ return this->_Features_EDX & (1 << 30); }
			bool PendingBreakEnable()			{ return this->_Features_EDX & 0x80000000; } // stop clang complaining about overflow



			// ECX flags, EAX = 1
			bool SSE3Instructions()				{ return this->_Features_ECX & (1 <<  0); }
			bool PCLMULQDQ()					{ return this->_Features_ECX & (1 <<  1); }
			bool DebugStore64Bit()				{ return this->_Features_ECX & (1 <<  2); }
			bool MonitorMwaitInstructions()		{ return this->_Features_ECX & (1 <<  3); }
			bool CPLDebugStore()				{ return this->_Features_ECX & (1 <<  4); }
			bool VirtualMachineExtensions()		{ return this->_Features_ECX & (1 <<  5); }
			bool SaferMode()					{ return this->_Features_ECX & (1 <<  6); }
			bool EnhancedSpeedStep()			{ return this->_Features_ECX & (1 <<  7); }
			bool ThermalMonitor2()				{ return this->_Features_ECX & (1 <<  8); }
			bool SupplementalSSE3Instructions()	{ return this->_Features_ECX & (1 <<  9); }
			bool L1ContextID()					{ return this->_Features_ECX & (1 << 10); }
			// 11 is reserved
			bool FusedMultiplyAdd()				{ return this->_Features_ECX & (1 << 12); }
			bool CompareExchange16Bit()			{ return this->_Features_ECX & (1 << 13); }
			bool DisableTaskPriorityMessages()	{ return this->_Features_ECX & (1 << 14); }
			bool PerformanceMonitorAndDebug()	{ return this->_Features_ECX & (1 << 15); }
			// 16 is reserved
			bool ProcessContextID()				{ return this->_Features_ECX & (1 << 17); }
			bool DMADirectCache()				{ return this->_Features_ECX & (1 << 18); }
			bool SSE4Point1Instructions()		{ return this->_Features_ECX & (1 << 19); }
			bool SSE4Point2Instructions()		{ return this->_Features_ECX & (1 << 20); }
			bool x2APIC()						{ return this->_Features_ECX & (1 << 21); }
			bool BigEndianMove()				{ return this->_Features_ECX & (1 << 22); }
			bool PopCountInstruction()			{ return this->_Features_ECX & (1 << 23); }
			bool APICOneShot()					{ return this->_Features_ECX & (1 << 24); }
			bool AESInstructions()				{ return this->_Features_ECX & (1 << 25); }
			bool XStateSave()					{ return this->_Features_ECX & (1 << 26); }
			bool XStateSaveEnabled()			{ return this->_Features_ECX & (1 << 27); }
			bool AVXInstructions()				{ return this->_Features_ECX & (1 << 28); }
			bool HalfPrecisionFloatingPoint()	{ return this->_Features_ECX & (1 << 29); }
			bool HardwareRandomGenerator()		{ return this->_Features_ECX & (1 << 30); }
			bool IsInsideHypervisor()			{ return this->_Features_ECX & 0x80000000; } // same.



			// EDX Flags, EAX = 0x80000001
			// Onboardx87FPU
			// Virtual8086Extensions
			// DebugExtensions
			// PageSizeExtension
			// TimeStampCounter
			// ModelSpecificRegisters()
			// PhysicalAddressExtensions
			// MachineCheckException
			// CompareAndSwap
			// OnboardAPIC
			// 10 is reserved.
			bool SyscallAndSysret()				{ return this->_Extended_EDX & (1 << 11); }
			// MemoryTypeRange
			// PageGlobalBit
			// MachineCheckArchitecture
			// ConditionalMove
			// PageAttributeTable
			// PSE36Bit
			// 18 is reserved
			bool MultiprocessorCabable()		{ return this->_Extended_EDX & (1 << 19); }
			bool NoExecute()					{ return this->_Extended_EDX & (1 << 20); }
			// 21 is reserved.
			bool ExtendedMMXInstructions()		{ return this->_Extended_EDX & (1 << 22); }
			// MMXInstructions
			// FXStateSave
			bool FXStateSaveOptimisations()		{ return this->_Extended_EDX & (1 << 25); }
			bool GigabytePageSize()				{ return this->_Extended_EDX & (1 << 26); }
			bool RDTSCP()						{ return this->_Extended_EDX & (1 << 27); }
			// 28 is reserved
			bool LongMode()						{ return this->_Extended_EDX & (1 << 29); }
			bool Extended3DNow()				{ return this->_Extended_EDX & (1 << 30); }
			bool ThreeDeeNow()					{ return this->_Extended_EDX & 0x80000000; } // same



			// ECX Flags, EAX = 0x80000001
			bool LAHFAndSAHF()					{ return this->_Extended_ECX & (1 <<  0); }
			bool NoHyperThreading()				{ return this->_Extended_ECX & (1 <<  1); }
			bool SecureVirtualMachine()			{ return this->_Extended_ECX & (1 <<  2); }
			bool ExtendedAPIC()					{ return this->_Extended_ECX & (1 <<  3); }
			bool CR8In32BitMode()				{ return this->_Extended_ECX & (1 <<  4); }
			bool AdvancedBitManipulation()		{ return this->_Extended_ECX & (1 <<  5); }
			bool SSE4aInstructions()			{ return this->_Extended_ECX & (1 <<  6); }
			bool MisalignedSSEMode()			{ return this->_Extended_ECX & (1 <<  7); }
			bool Prefetch3DNow()				{ return this->_Extended_ECX & (1 <<  8); }
			bool OSVisibleWorkaround()			{ return this->_Extended_ECX & (1 <<  9); }
			bool InstructionBasedSampling()		{ return this->_Extended_ECX & (1 << 10); }
			bool XOPInstructions()				{ return this->_Extended_ECX & (1 << 11); }
			bool SKInitAndSTGI()				{ return this->_Extended_ECX & (1 << 12); }
			bool WatchdogTimer()				{ return this->_Extended_ECX & (1 << 13); }
			// 14 is reserved
			bool LightWeightProfiling()			{ return this->_Extended_ECX & (1 << 15); }
			bool FourOpsFusedMulAdd()			{ return this->_Extended_ECX & (1 << 16); }
			bool TranslationCacheExtension()	{ return this->_Extended_ECX & (1 << 17); }
			// 18 is reserved
			bool NodeIDModelSpecifcRegister()	{ return this->_Extended_ECX & (1 << 19); }
			// 20 is reserved
			bool TrailingBitManipulation()		{ return this->_Extended_ECX & (1 << 21); }
			bool TopologyExtensions()			{ return this->_Extended_ECX & (1 << 22); }
			bool CorePerformanceExtensions()	{ return this->_Extended_ECX & (1 << 23); }
			bool NBPerformanceExtensions()		{ return this->_Extended_ECX & (1 << 24); }
			// 25 is reserved
			bool DataBreakpointExtensions()		{ return this->_Extended_ECX & (1 << 26); }
			bool PerformanceTimeStampCounter()	{ return this->_Extended_ECX & (1 << 27); }
			bool L2iPerformanceExtensions()		{ return this->_Extended_ECX & (1 << 28); }
			// 29 - 31 are reserved.



			char _VendorID[13];
			char _BrandString[49];
			uint32_t _Features_ECX;
			uint32_t _Features_EDX;
			uint32_t _Features_Extended;
			uint32_t _Highest_Extended_Function;
			uint32_t _Extended_ECX;
			uint32_t _Extended_EDX;

			uint32_t _Stepping;
			uint32_t _Model;
			uint32_t _Family;
			uint32_t _ProcessorType;
	};

	CPUIDData* Initialise(CPUIDData* CPUIDInfo);

}
}
}
