// JobDispatcher.hpp
// Copyright (c) 2014 - 2016, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#pragma once
#include <stdint.h>

namespace Kernel {
namespace JobDispatch
{
	#define JOBPRIO_HIGH	2
	#define JOBPRIO_NORM	1
	#define JOBPRIO_LOW		0

	struct Job
	{
		Job() : handle(0), param(0), prio(0) { }
		Job(void (*_handle)(void*), void* _param, int _prio) : handle(_handle), param(_param), prio(_prio) { }

		void (*handle)(void*);
		void* param;
		int prio;
	};

	void AddJob(Job job);
	void RemoveJob(Job job);
	void Initialise();
}
}
