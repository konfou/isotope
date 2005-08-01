/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GThread.h"
#include "GMacros.h"
#ifndef WIN32
#include <pthread.h>
#endif // !WIN32

HANDLE GThread::SpawnThread(unsigned int (*pFunc)(void*), void* pData)
{
#ifdef WIN32
	unsigned int nID;
	HANDLE hThread = (void*)CreateThread/*_beginthreadex*/(
							NULL,
							0,
							(LPTHREAD_START_ROUTINE)pFunc,
							pData,
							0,
							(unsigned long*)&nID
							);
	GAssert(hThread != BAD_HANDLE, "Failed to create thread");
	return hThread;
#else // WIN32
	pthread_t thread;
	if(pthread_create(&thread, NULL, (void*(*)(void*))pFunc, pData) != 0)
	{
		GAssert(false, "Failed to create thread");
	}
	return (HANDLE)NULL;
#endif // else WIN32
}

