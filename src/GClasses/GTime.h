/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GTIME_H__
#define __GTIME_H__

class GTime
{
public:
	// Returns the number of seconds since midnight with at least milisecond precision
	static double GetTime();
};

#endif // __GTIME_H__
