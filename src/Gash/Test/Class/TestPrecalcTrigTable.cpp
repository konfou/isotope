/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "TestMatrix.h"
#include "../../../GClasses/GTrigTable.h"
#include "../ClassTests.h"
#include <math.h>
#ifdef WIN32
#include <windows.h>
#endif // WIN32

bool TestGPrecalculatedTrigTableForAccuracy(ClassTests* pThis)
{
	GPrecalculatedTrigTable trig(524288);

	// Test critical Sine values
	if(ABS(trig.Sin(0) - sin((double)0)) > .000001)
		return false;
	if(ABS(trig.Sin(PI / 2) - sin(PI / 2)) > .000001)
		return false;
	if(ABS(trig.Sin(PI) - sin(PI)) > .000001)
		return false;
	if(ABS(trig.Sin(2 * PI) - sin(2 * PI)) > .000001)
		return false;
	if(ABS(trig.Sin(-PI / 2) - sin(-PI / 2)) > .000001)
		return false;
	if(ABS(trig.Sin(-PI) - sin(-PI)) > .000001)
		return false;
	if(ABS(trig.Sin(-2 * PI) - sin(-2 * PI)) > .000001)
		return false;

	// Test critical Cosine values
	if(ABS(trig.Cos(0) - cos((double)0)) > .000001)
		return false;
	if(ABS(trig.Cos(PI / 2) - cos(PI / 2)) > .000001)
		return false;
	if(ABS(trig.Cos(PI) - cos(PI)) > .000001)
		return false;
	if(ABS(trig.Cos(2 * PI) - cos(2 * PI)) > .000001)
		return false;
	if(ABS(trig.Cos(-PI / 2) - cos(-PI / 2)) > .000001)
		return false;
	if(ABS(trig.Cos(-PI) - cos(-PI)) > .000001)
		return false;
	if(ABS(trig.Cos(-2 * PI) - cos(-2 * PI)) > .000001)
		return false;

	// Test random Sine and Cosine values
	double d;
	double d1;
	double d2;
	int n;
	for(n = 0; n < 10000; n++)
	{
		d = (double)(rand() - (RAND_MAX / 2)) / (((double)rand()) / 1000);
		d1 = trig.Sin(d);
		d2 = sin(d);
		if(ABS(d1 - d2) > .00001)
			return false;
		d1 = trig.Cos(d);
		d2 = cos(d);
		if(ABS(d1 - d2) > .00001)
			return false;
	}
	return true;
}

bool TestGPrecalculatedTrigTableForSpeed(ClassTests* pThis)
{
	GPrecalculatedTrigTable trig(360);
#ifdef WIN32
// todo: rewrite this code for Linux

	// Make sure precalculated table is faster than math functions
	__int64 nClockBefore;
	__int64 nClockAfter;
	__int64 nDiff1;
	__int64 nDiff2;
	int n;
	double d;

	srand(12345);
	QueryPerformanceCounter((LARGE_INTEGER*)&nClockBefore);
	for(n = 0; n < 100000; n++)
	{
		d = trig.Sin(rand());
		d = trig.Cos(rand());
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&nClockAfter);
	nDiff1 = nClockAfter - nClockBefore;

	srand(12345);
	QueryPerformanceCounter((LARGE_INTEGER*)&nClockBefore);
	for(n = 0; n < 100000; n++)
	{
		d = sin((double)rand());
		d = cos((double)rand());
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&nClockAfter);
	nDiff2 = nClockAfter - nClockBefore;

	if(nDiff1 > nDiff2)
		return false;
	else
#endif // WIN32
		return true;
}

bool TestGPrecalculatedTrigTable(ClassTests* pThis)
{
	if(!TestGPrecalculatedTrigTableForAccuracy(pThis))
		return false;
//	if(!TestGPrecalculatedTrigTableForSpeed(pThis))
//		return false;
	return true;
}
