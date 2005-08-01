/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GMacros.h"
#include "GTimer.h"
#ifdef WIN32
#include <windows.h>
#endif // WIN32

GTimer::GTimer(int nFramesPerSecond)
{
	m_nClock = 0;
	__int64 nTicksPerSecond;
	if(!QueryPerformanceFrequency((LARGE_INTEGER*)&nTicksPerSecond))
		GAssert(false, "Error querying timer");
	m_nTicksPerFrame = nTicksPerSecond / nFramesPerSecond;
}

bool GTimer::IsItTimeYet()
{
	__int64 nClock;
	QueryPerformanceCounter((LARGE_INTEGER*)&nClock);
	if(nClock - m_nClock < m_nTicksPerFrame)
		return(false);
	m_nClock = nClock;
	return(true);
}
