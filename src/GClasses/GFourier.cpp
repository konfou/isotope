/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GFourier.h"
#include <math.h>
#include "GMacros.h"

inline int ReverseBits(int nValue, int nBits)
{
    int n;
	int nReversed = 0;
    for(n = 0; n < nBits; n++)
    {
        nReversed = (nReversed << 1) | (nValue & 1);
        nValue >>= 1;
    }
    return nReversed;
}

bool GFourier::FFT(struct ComplexNumber* pComplexNumberArray, int nArraySize, bool bInverse)
{
	double* pData = (double*)pComplexNumberArray;

	// Make sure nArraySize is a power of 2
	if(nArraySize & (nArraySize - 1))
	{
		GAssert(false, "Error, nArraySize must be a power of 2");
		return false;
	}
	
	// Calculate the Log2 of nArraySize and put it in nBits
	int n = 1;
	int nBits = 0;
	while(n < nArraySize)
	{
		n <<= 1;
		nBits++;
	}

	// Move the data to it's reversed-bit position
	int nTotalSize = nArraySize << 1;
	double* pTmp = new double[nArraySize << 1];
	int nReversed;
	for(n = 0; n < nArraySize; n++)
	{
		nReversed = ReverseBits(n, nBits);
		pTmp[nReversed << 1] = pData[n << 1];
		pTmp[(nReversed << 1) + 1] = pData[(n << 1) + 1];
	}
	for(n = 0; n < nTotalSize; n++)
		pData[n] = pTmp[n];
	delete(pTmp);

	// Calculate the angle numerator
	double dAngleNumerator;
	if(bInverse)
		dAngleNumerator = 2.0 * PI;
	else
		dAngleNumerator = -2.0 * PI;

	// Do the Fast Forier Transform
	double dR0, dR1, dR2, dR3, dI0, dI1, dI2, dI3;
	int n2;
	int nStart;
	int nHalfBlockSize;
	for(nHalfBlockSize = 1; nHalfBlockSize < nArraySize; nHalfBlockSize <<= 1)
	{
		// Calculate angles, sines, and cosines
		double dAngleDelta = dAngleNumerator / ((double)(nHalfBlockSize << 1));
		double dCos1 = cos(-dAngleDelta);
		double d2Cos1 = 2 * dCos1; // So we don't have to calculate this a bunch of times
		double dCos2 = cos(-2 * dAngleDelta);
		double dSin1 = sin(-dAngleDelta);
		double dSin2 = sin(-2 * dAngleDelta);

		// Do each block
		for(nStart = 0; nStart < nArraySize; nStart += (nHalfBlockSize << 1))
		{
			dR1 = dCos1;
			dR2 = dCos2;
			dI1 = dSin1;
			dI2 = dSin2;
			int nEnd = nStart + nHalfBlockSize;
			for(n = nStart; n < nEnd; n++)
			{
				dR0 = d2Cos1 * dR1 - dR2;
				dR2 = dR1;
				dR1 = dR0;
				dI0 = d2Cos1 * dI1 - dI2;
				dI2 = dI1;
				dI1 = dI0;
				n2 = n + nHalfBlockSize;
				dR3 = dR0 * pData[n2 << 1] - dI0 * pData[(n2 << 1) + 1];
				dI3 = dR0 * pData[(n2 << 1) + 1] + dI0 * pData[n2 << 1];
				pData[n2 << 1] = pData[n << 1] - dR3;
				pData[(n2 << 1) + 1] = pData[(n << 1) + 1] - dI3;
				pData[n << 1] += dR3;
				pData[(n << 1) + 1] += dI3;
			}
		}
	}

	// Normalize output if we're doing the inverse forier transform
	if(bInverse)
	{
		for(n = 0; n < nTotalSize; n++)
			pData[n] /= (double)nArraySize;
	}

	return true;
}

bool GFourier::FFT2D(struct ComplexNumber* p2DComplexNumberArray, int nArrayWidth, bool bInverse)
{
	double* pData = (double*)p2DComplexNumberArray;

	double* pTmpArray = new double[nArrayWidth << 1];
	int x, y;
	for(y = 0; y < nArrayWidth; y++)
	{
		for(x = 0; x < nArrayWidth; x++)
		{
			pTmpArray[x << 1] = pData[(nArrayWidth * y + x) << 1];
			pTmpArray[(x << 1) + 1] = pData[((nArrayWidth * y + x) << 1) + 1];
		}
		if(!FFT((struct ComplexNumber*)pTmpArray, nArrayWidth, bInverse))
		{
			delete(pTmpArray);
			return false;
		}
		for(x = 0; x < nArrayWidth; x++)
		{
			pData[(nArrayWidth * y + x) << 1] = pTmpArray[x << 1];
			pData[((nArrayWidth * y + x) << 1) + 1] = pTmpArray[(x << 1) + 1];
		}
	}
	for(x = 0; x < nArrayWidth; x++)
	{
		for(y = 0; y < nArrayWidth; y++)
		{
			pTmpArray[y << 1] = pData[(nArrayWidth * y + x) << 1];
			pTmpArray[(y << 1) + 1] = pData[((nArrayWidth * y + x) << 1) + 1];
		}
		if(!FFT((struct ComplexNumber*)pTmpArray, nArrayWidth, bInverse))
		{
			delete(pTmpArray);
			return false;
		}
		for(y = 0; y < nArrayWidth; y++)
		{
			pData[(nArrayWidth * y + x) << 1] = pTmpArray[y << 1];
			pData[((nArrayWidth * y + x) << 1) + 1] = pTmpArray[(y << 1) + 1];
		}
	}
	delete(pTmpArray);
	return true;
}
