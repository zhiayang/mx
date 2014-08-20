// RealTimeClock.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.



#include <Kernel.hpp>
#include <HardwareAbstraction/Devices/IOPort.hpp>


using namespace Kernel::HardwareAbstraction::Devices;

namespace Kernel {
namespace HardwareAbstraction {
namespace Devices {
namespace RTC
{
	static bool DidInit = false;

	const uint8_t CMOSAddrReg	= 0x70;
	const uint8_t CMOSDataReg	= 0x71;

	uint8_t ReadRegister(uint8_t reg)
	{
		IOPort::WriteByte(CMOSAddrReg, reg);
		return IOPort::ReadByte(CMOSDataReg);
	}

	bool IsUpdateInProgress()
	{
		IOPort::WriteByte(CMOSAddrReg, 0x0A);
		return IOPort::ReadByte(CMOSDataReg) & 0x80;
	}

	void CalculateSecondsSinceEpoch(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second)
	{
		// if we call this function, reset the time.
		Kernel::Time::SecondsSinceEpoch = 0;
		uint16_t actualyear = 2000 + year;
		// loop through each year.

		for(uint16_t i = 0; i < (actualyear - 2000) + (2000 - Kernel::Time::EpochYear); i++)
		{
			// add each complete year, checking for leap years.
			Kernel::Time::SecondsSinceEpoch += (60 * 60 * 24 * (Kernel::Time::IsLeapYear(Kernel::Time::EpochYear + i) ? 366 : 365));
		}

		// now check for each complete month.
		// because we do a switch-case here, i needs to start at one.
		// in actual fact, there is no zero value (month 0?!), so we should minus one.
		// offset with i = 1.
		for(uint8_t i = 1; i < month; i++)
		{
			switch(i)
			{
				case 1:
				case 3:
				case 5:
				case 7:
				case 8:
				case 10:
				case 12:
					Kernel::Time::SecondsSinceEpoch += (60 * 60 * 24 * 31);
					break;

				case 4:
				case 6:
				case 9:
				case 11:
					Kernel::Time::SecondsSinceEpoch += (60 * 60 * 24 * 30);
					break;

				case 2:
					if(Kernel::Time::IsLeapYear(actualyear))
					{
						Kernel::Time::SecondsSinceEpoch += (60 * 60 * 24 * 29);
					}
					else
					{
						Kernel::Time::SecondsSinceEpoch += (60 * 60 * 24 * 28);
					}
					break;
			}
		}

		// now check for each day.
		// cannot have zero value (january 0?!), so we minus one.
		for(uint8_t i = 0; i < (day - 1); i++)
			Kernel::Time::SecondsSinceEpoch += (60 * 60 * 24);

		// and each hour.
		// zero value permitted, iterate normally.
		for(uint8_t i = 0; i < hour; i++)
			Kernel::Time::SecondsSinceEpoch += (60 * 60);

		// and each minute.
		// similarly, zero value allowed.
		for(uint8_t i = 0; i < minute; i++)
			Kernel::Time::SecondsSinceEpoch += 60;


		// and finally the seconds.
		Kernel::Time::SecondsSinceEpoch += second;
	}

	bool DidInitialise()
	{
		return DidInit;
	}

	void ReadTime()
	{
		uint8_t registerB;

		uint8_t second;
		uint8_t minute;
		uint8_t hour;
		uint8_t day;
		uint8_t month;
		uint8_t year;


		// if we already initialised, we're allowed to skip this one if the RTC is still updating.
		if(DidInit && !IsUpdateInProgress())
			return;

		const uint64_t tt = 50;
		uint64_t timeout = tt;
		while(!IsUpdateInProgress() && timeout > 0){ timeout--; }		// wait until it's updating
		while(IsUpdateInProgress() && timeout < tt){ timeout++; }	// wait until it's done.


		second	= ReadRegister(0x00);
		minute	= ReadRegister(0x02);
		hour	= ReadRegister(0x04);
		day	= ReadRegister(0x07);
		month	= ReadRegister(0x08);
		year	= ReadRegister(0x09);





		if(second == 60)
			second = 0;

		if(minute == 60)
			minute = 0;

		if(hour == 24)
			hour = 0;



		registerB = ReadRegister(0x0B);

		// Convert BCD to binary values if necessary
		if(!(registerB & 0x04))
		{
			second	= (second & 0x0F) + ((second / 16) * 10);
			minute	= (minute & 0x0F) + ((minute / 16) * 10);
			hour	= ((hour & 0x0F) + (((hour & 0x70) / 16) * 10)) | (hour & 0x80);
			day	= (day & 0x0F) + ((day / 16) * 10);
			month	= (month & 0x0F) + ((month / 16) * 10);
			year	= (year & 0x0F) + ((year / 16) * 10);
		}

		// Convert 12 hour clock to 24 hour clock if necessary
		if (!(registerB & 0x02) && (hour & 0x80))
		{
			hour = ((hour & 0x7F) + 12) % 24;
		}

		if(second == 60)
			second = 0;

		DidInit = true;

		// now convert this time to a format in epochs.
		CalculateSecondsSinceEpoch(year, month, day, hour, minute, second);
		Time::GetTime();

	}

	void Initialise(int8_t UTCOffset)
	{
		Kernel::SystemTime = new Kernel::Time::TimeStruct();
		Kernel::SystemTime->UTCOffset = UTCOffset;
		ReadTime();

		Kernel::SystemTime->MillisecondsSinceEpoch = Kernel::Time::SecondsSinceEpoch * 1000;
	}

	void InitialiseTimer()
	{
		// Kernel::HardwareAbstraction::Interrupts::SetGate(32 + 8, (uint64_t) Time::RTCHandler, 0x08, 0xEE);

		// IOPort::WriteByte(0x70, 0x8B);				// select register B, and disable NMI
		// uint8_t prev = IOPort::ReadByte(0x71);			// read the current value of register B

		// IOPort::WriteByte(0x70, 0x8B);				// set the index again (a read will reset the index to register D)
		// IOPort::WriteByte(0x71, prev | 0x40);			// write the previous value ORed with 0x40. This turns on bit 6 of register B

		// uint8_t rate = RTCTImerShiftAmt;			// rate must be above 2 and not over 15
		// IOPort::WriteByte(0x70, 0x8A);				// set index to register A, disable NMI
		// prev = IOPort::ReadByte(0x71);				// get initial value of register A
		// IOPort::WriteByte(0x70, 0x8A);				// reset index to A
		// IOPort::WriteByte(0x71, (prev & 0xF0) | rate);		//write only our rate to A. Note, rate is the bottom 4 bits.

		// IOPort::WriteByte(0x70, 0x0C);
		// IOPort::ReadByte(0x71);
	}
}
}
}
}
