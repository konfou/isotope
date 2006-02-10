/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GSpinLock.h"
#include <time.h>
#include "GMacros.h"
#include "GThread.h"
#ifdef DARWIN
#include <libkern/OSAtomic.h>
#endif

GSpinLock::GSpinLock()
{
	m_dwLocked = 0;
#ifdef _DEBUG
	m_szUniqueStaticStringToIdentifyWhoLockedIt = "<Never Been Locked>";
#endif
}

GSpinLock::~GSpinLock()
{
}

static inline unsigned int testAndSet(unsigned int* pDWord)
{
#ifdef WIN32
	unsigned int dwRetVal;
	__asm
    {
        mov edx, pDWord
        mov eax, 1
        lock xchg eax, [edx] // This is the line that does the job
		mov dwRetVal, eax
	}
	return dwRetVal;
#else // WIN32
#ifdef DARWIN
	return (int)!OSSpinLockTry((OSSpinLock *)pDWord);
#else // DARWIN
	unsigned int dwRetVal;
	__asm__ __volatile__
		("xchgl %0, %1"
		: "=r"(dwRetVal), "=m"(*pDWord)
		: "0"(1), "m"(*pDWord)
		: "memory");
	return dwRetVal;
#endif // !DARWIN
#endif // !WIN32
}

void GSpinLock::Lock(const char* szUniqueStaticStringToIdentifyWhoLockedIt)
{
#ifdef _DEBUG
	time_t t;
	time_t tStartTime = time(&t);
	time_t tCurrentTime;
#endif // _DEBUG

	while(testAndSet(&m_dwLocked))
	{
#ifdef _DEBUG
		tCurrentTime = time(&t);
		GAssert(tCurrentTime - tStartTime < 10, "Blocked for 10 seconds!");
#endif // _DEBUG
		GThread::sleep(0);
	}
#ifdef _DEBUG
	m_szUniqueStaticStringToIdentifyWhoLockedIt = szUniqueStaticStringToIdentifyWhoLockedIt;
#endif // _DEBUG
}

void GSpinLock::Unlock()
{
#ifdef _DEBUG
	m_szUniqueStaticStringToIdentifyWhoLockedIt = "<Not Locked>";
#endif // _DEBUG
#ifdef DARWIN
	OSSpinLockUnlock((OSSpinLock *)&m_dwLocked);
#else
	m_dwLocked = 0;
#endif
}

