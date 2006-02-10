/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GTHREAD_H__
#define __GTHREAD_H__

#include "GMacros.h"
#ifdef WIN32
#	include <windows.h>
#	include "GWindows.h"
#else
#	include <unistd.h>
#	include <sched.h>
#endif // WIN32

// A wrapper for PThreads on Linux and for some corresponding WIN32 api on Windows
class GThread
{
public:
	static HANDLE SpawnThread(unsigned int (*pFunc)(void*), void* pData);

	// don't sleep more than 975ms = 999ms on unix platforms. - simple math
	static inline void sleep(unsigned int nMiliseconds)
	{
#ifdef WIN32
		GWindows::YieldToWindows();
		Sleep(nMiliseconds);
#else // WIN32
		nMiliseconds ? usleep(nMiliseconds*1024) : sched_yield();		// it is an error to sleep for more than 1,000,000
#endif // else WIN32
	}
};

#endif // __GTHREAD_H__
