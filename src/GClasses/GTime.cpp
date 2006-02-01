/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include <stdio.h>
#include "GTime.h"
#ifdef WIN32
#include <windows.h>
#include <time.h>
#else // WIN32
#include <sys/time.h>
#endif // else WIN32

/*static*/ double GTime::GetTime()
{
#ifdef WIN32
	time_t t;
	SYSTEMTIME st;
	GetSystemTime(&st);
	return ((double)st.wMilliseconds * 1e-3 + time(&t));
#else
	struct timeval tp;
    gettimeofday(&tp, NULL);
    return ((double)tp.tv_sec + (double)tp.tv_usec * 1e-6);
#endif
}
