// TimerProcessing.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0


#include <Kernel.hpp>
#include <HardwareAbstraction/Devices/RTC.hpp>
#include <List.hpp>
#include <String.hpp>
#include <math.h>

extern "C" uint64_t SwitchProcess(uint64_t);
namespace Kernel {
namespace HardwareAbstraction {
namespace Multitasking
{
	// number of milliseconds elapsed.
	static uint64_t TimerCounter = 0;
	static uint16_t ThreadTime = 0;

	// in ms
	static const uint64_t SchedulerTickRate = 100;
	static const uint64_t ElapsedTicksPerCall = GlobalMilliseconds / GlobalTickRate;

	extern "C" uint64_t ProcessTimerInterrupt_C(uint64_t context)
	{
		TimerCounter += (GlobalMilliseconds / GlobalTickRate);
		ThreadTime += (GlobalMilliseconds / GlobalTickRate);

		for(uint64_t l = SleepList->Size(), g = 0; g < l; g++)
		{
			Thread* m = SleepList->RemoveFront();

			if(m->State == STATE_SUSPEND || m->State == STATE_BLOCKING)
			{
				SleepList->InsertBack(m);
				continue;
			}
			else if(m->State == STATE_AWAITDEATH)
			{
				// don't delete.
				m->State = STATE_DEAD;
				NumThreads--;
				continue;
			}



			if(m->Sleep <= ElapsedTicksPerCall)
				m->Sleep = 0;

			else
				m->Sleep -= ElapsedTicksPerCall;



			if(m->Sleep == 0)
			{
				GetThreadList(m)->push_front(m);
			}
			else
			{
				SleepList->InsertBack(m);
			}
		}






		// do the thing with the rtc thing
		if(Devices::RTC::DidInitialise())
			Time::UpdateTime();

		// increment the spent time in the current thread

		if(SchedulerEnabled && TimerCounter % SchedulerTickRate)
		{
			if(GetCurrentThread() != 0)
				GetCurrentThread()->ExecutionTime = ThreadTime;

			ThreadTime = 0;
			return SwitchProcess(context);
		}

		return 0;
	}
}
}
}





