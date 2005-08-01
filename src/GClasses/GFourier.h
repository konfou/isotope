/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GFOURIER_H__
#define __GFOURIER_H__

struct ComplexNumber
{
	double dReal;
	double dImag;
};


class GFourier
{
public:
	// This will do a Fast Forier Transform.  nArraySize must be a power of 2.
	static bool FFT(struct ComplexNumber* pComplexNumberArray, int nArraySize, bool bInverse = false);

	// 2D Fast Forier Transform.  The 2D array must be square (width = height) and width must be a
	// power of 2.
	static bool FFT2D(struct ComplexNumber* p2DComplexNumberArray, int nArrayWidth, bool bInverse = false);

};


#endif // __GFOURIER_H__
