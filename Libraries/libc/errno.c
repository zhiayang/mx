/*

   Copyright 2009 Pierre KRIEGER

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

*/

#include "include/errno.h"
#include "include/stdlib.h"
#include "machine/_target.h"

// 	How does it work ?

// The _get_errno function gives the location of errno
// depending on the current stack location

// The goal is to make errno as reentrant as possible
// (though it's not yet guaranteed)

// The _get_errno function determines the current stack, then
// tries to find a similar value in the _errno_entries table.
// If it doesn't find an entry, it will create one


int errno = 0;


// #define	_ERRNO_ENTRIES_COUNT	16

// static struct _errno_entry
// {
// 	void*	stack;
// 	int	value;
// } _errno_entries[_ERRNO_ENTRIES_COUNT] = { { 0, 0 } };
// static int _errno_entries_oldest = 0;

int _get_errno()
{
	return errno;
}


// int* _get_errno()
// {
// 	// first we determine the current stack
// 	void* currentStack = 0;
// 	asm("mov %%rsp, %0" : "=r"(currentStack));

// 	// now we search the table for a similar stack
// 	int i;

// 	for (i = 0; i < _ERRNO_ENTRIES_COUNT; i++)
// 	{
// 		// two stacks are similar <=> diff less than 0x500
// 		if(abs((int) ((unsigned long long) currentStack - (unsigned long long) _errno_entries[i].stack)) < 0x500)
// 			return &_errno_entries[i].value;
// 	}

// 	// there was no table entry, so we create one
// 	int* returnValue = &_errno_entries[_errno_entries_oldest].value;
// 	_errno_entries[_errno_entries_oldest].stack = currentStack;
// 	_errno_entries[_errno_entries_oldest].value = 0;
// 	_errno_entries_oldest++;
// 	_errno_entries_oldest %= _ERRNO_ENTRIES_COUNT;
// 	return returnValue;
// }
