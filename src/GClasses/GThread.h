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
#include <windows.h>
#endif // WIN32

// A wrapper for PThreads on Linux and for some corresponding WIN32 api on Windows
class GThread
{
public:
	static HANDLE SpawnThread(unsigned int (*pFunc)(void*), void* pData);
};

#endif // __GTHREAD_H__
