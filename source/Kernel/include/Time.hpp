// Time.hpp
// Copyright (c) 2013 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#pragma once
#include <stdint.h>

namespace Kernel
{
	namespace Time
	{
		class TimeStruct
		{
			public:
				uint64_t SecondsSinceEpoch;
				uint64_t MillisecondsSinceEpoch;
				uint16_t Year;
				uint8_t Month;
				uint8_t Day;
				uint16_t YearDay;

				uint8_t Weekday;

				bool AM;
				uint8_t Hour12F;
				uint8_t Hour;
				uint8_t HourActual;
				uint8_t HourActual12F;
				uint8_t Minute;
				uint8_t Second;

				int8_t UTCOffset;
		};

		extern const uint16_t EpochYear;
		extern uint64_t SecondsSinceEpoch;
		extern bool TimerOn;
		extern bool PrintSeconds;


		void GetTime();
		void ReSyncTime();
		void IncrementAndRollover();
		bool IsLeapYear(uint16_t year);
		void UpdateTime();


		void TimeSyncService();
		uint8_t DaysInMonth(uint8_t month);
		void PrintTime();

		uint64_t Now();
		// void GetHumanReadableTime(std::string& output);

		void AdjustForTimezone();

		extern "C" void RTCHandler();
	}
}
