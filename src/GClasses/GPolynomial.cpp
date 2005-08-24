/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GMacros.h"
#include "GPolynomial.h"
#ifdef DARWIN
#include <sys/malloc.h>
#else // DARWIN
#include <malloc.h>
#endif // !DARWIN
#include "GArff.h"
#include <math.h>

GPolynomial::GPolynomial(int nDimensions, int nControlPoints)
{
	GAssert(nDimensions > 0 && nControlPoints > 0, "Bad values");
	m_nDimensions = nDimensions;
	m_nControlPoints = nControlPoints;
	m_nCoefficients = 1;
	while(nDimensions > 0)
	{
		m_nCoefficients *= nControlPoints;
		nDimensions--;
	}
	m_pCoefficients = new double[m_nCoefficients];
	int n;
	for(n = 0; n < m_nCoefficients; n++)
		m_pCoefficients[n] = 0;
}

GPolynomial::~GPolynomial()
{
	delete(m_pCoefficients);
}

int GPolynomial::CalcIndex(int* pDegrees)
{
	int nIndex = 0;
	int n;
	for(n = m_nDimensions - 1; n >= 0; n--)
	{
		nIndex *= m_nControlPoints;
		GAssert(pDegrees[n] >= 0 && pDegrees[n] < m_nControlPoints, "out of range");
		nIndex += pDegrees[n];
	}
	return nIndex;
}

double GPolynomial::GetCoefficient(int* pDegrees)
{
	return m_pCoefficients[CalcIndex(pDegrees)];
}

void GPolynomial::SetCoefficient(int* pDegrees, double dVal)
{
	m_pCoefficients[CalcIndex(pDegrees)] = dVal;
}

double GPolynomial::Eval(double* pVariables)
{
	int* pDegrees = (int*)alloca(m_nDimensions);
	int n, i;
	for(n = 0; n < m_nDimensions; n++)
		pDegrees[n] = m_nControlPoints - 1;
	double dSum = 0;
	double dVar;
	int nCoeff;
	for(nCoeff = m_nCoefficients - 1; nCoeff >= 0; nCoeff--)
	{
		dVar = 1;
		for(n = 0; n < m_nDimensions; n++)
		{
			for(i = pDegrees[n]; i > 0; i--)
				dVar *= pVariables[n];
		}
		dVar *= m_pCoefficients[nCoeff];
		dSum += dVar;
		i = 0;
		while(true)
		{
			if(pDegrees[i]-- == 0)
			{
				pDegrees[i] = m_nControlPoints - 1;
				if(++i < m_nControlPoints)
					continue;
			}
			break;
		}
	}
	return dSum;
}

double GPolynomial::MeasureMeanSquareError(GArffRelation* pRelation, GArffData* pData, int nOutputAttr)
{
	GArffAttribute* pAttr = pRelation->GetAttribute(nOutputAttr);
	GAssert(!pAttr->IsInput(), "expected an output attribute");
	int nInputs = pRelation->GetInputCount();
	double dSum = 0;
	double* pRow;
	double* pVariables = (double*)alloca(sizeof(double) * nInputs);
	double dError;
	int n, i;
	int nCount = pData->GetRowCount();
	double dTargetVal;
	double dEstimatedVal;
	for(n = 0; n < nCount; n++)
	{
		pRow = pData->GetRow(n);
		for(i = 0; i < nInputs; i++)
			pVariables[i] = pRow[pRelation->GetInputIndex(i)];
		dTargetVal = pRow[nOutputAttr];
		dEstimatedVal = Eval(pVariables);
		dError = dTargetVal - dEstimatedVal;
		dError *= dError;
		dSum += dError;
	}
	return dSum / nCount;
}

double GPolynomial::DivideAndMeasureError(GArffRelation* pRelation, GArffData* pData, int nOutputAttr)
{
	int nCount = pData->GetRowCount();
	GArffData d1(nCount);
	GArffData d2(nCount);
	int nInputs = pRelation->GetInputCount();
	int nLastInput = pRelation->GetInputIndex(nInputs - 1);
	GAssert(m_nDimensions == nInputs - 1, "unexpected number of inputs");
	double* pInputs = (double*)alloca(sizeof(double) * (nInputs - 1));
	int n, i;
	double dThresh;
	for(n = 0; n < nCount; n++)
	{
		double* pRow = pData->GetRow(n);
		for(i = 0; i < nInputs - 1; i++)
			pInputs[i] = pRow[pRelation->GetInputIndex(i)];
		dThresh = Eval(pInputs);
		if(dThresh >= pRow[nLastInput])
			d1.AddRow(pRow);
		else
			d2.AddRow(pRow);
	}
	double dError1 = 1e100;
	double dError2 = 1e100;
	if(d1.GetRowCount() * 5 >= d2.GetRowCount() && d2.GetRowCount() * 5 >= d1.GetRowCount())
	{
		Holder<GPolynomial*> hPoly1(FitData(pRelation, &d1, nOutputAttr, m_nControlPoints));
		dError1 = hPoly1.Get()->MeasureMeanSquareError(pRelation, &d1, nOutputAttr);
		Holder<GPolynomial*> hPoly2(FitData(pRelation, &d2, nOutputAttr, m_nControlPoints));
		dError2 = hPoly2.Get()->MeasureMeanSquareError(pRelation, &d2, nOutputAttr);
	}
	dError1 *= dError1;
	dError2 *= dError2;
	d1.DropAllRows();
	d2.DropAllRows();
	return dError1 + dError2;
}

inline double RandomDouble()
{
	return (double)rand() / RAND_MAX;
}

/*static*/ GPolynomial* GPolynomial::FitData(GArffRelation* pRelation, GArffData* pData, int nOutputAttr, int nControlPoints)
{
	GArffAttribute* pAttr = pRelation->GetAttribute(nOutputAttr);
	GAssert(!pAttr->IsInput(), "expected an output attribute");
	int nInputs = pRelation->GetInputCount();
	GPolynomial* pPolynomial = new GPolynomial(nInputs, nControlPoints);
	double dBestError = pPolynomial->MeasureMeanSquareError(pRelation, pData, nOutputAttr);
	double dError;
	double dStep;
	double d;
	int nCoefficients = pPolynomial->m_nCoefficients;
	int n;
	bool bGotOne;
	for(dStep = 100; dStep > .001; dStep *= .95)
	{
		d = RandomDouble() * dStep;
		bGotOne = false;
		for(n = 0; n < nCoefficients; n++)
		{
			pPolynomial->m_pCoefficients[n] += d;
			dError = pPolynomial->MeasureMeanSquareError(pRelation, pData, nOutputAttr);
			if(dError >= dBestError)
			{
				pPolynomial->m_pCoefficients[n] -= d;
				pPolynomial->m_pCoefficients[n] -= d;
				dError = pPolynomial->MeasureMeanSquareError(pRelation, pData, nOutputAttr);
			}
			if(dError >= dBestError)
				pPolynomial->m_pCoefficients[n] += d;
			else
			{
				dBestError = dError;
				bGotOne = true;
			}
		}
		if(!bGotOne)
			dStep *= .5;
	}
	return pPolynomial;
}

/*static*/ GPolynomial* GPolynomial::DivideData(GArffRelation* pRelation, GArffData* pData, int nOutputAttr, int nControlPoints)
{
	GArffAttribute* pAttr = pRelation->GetAttribute(nOutputAttr);
	GAssert(!pAttr->IsInput(), "expected an output attribute");
	int nInputs = pRelation->GetInputCount();
	GAssert(nInputs > 1, "Not enough inputs for this technique");
	GPolynomial* pPolynomial = new GPolynomial(nInputs - 1, nControlPoints);
	double dBestError = pPolynomial->DivideAndMeasureError(pRelation, pData, nOutputAttr);
	double dError;
	double dStep;
	double d;
	int nCoefficients = pPolynomial->m_nCoefficients;
	int n;
	bool bGotOne;
	for(dStep = 100; dStep > .01; dStep *= .95)
	{
		d = RandomDouble() * dStep;
		bGotOne = false;
		for(n = 0; n < nCoefficients; n++)
		{
			pPolynomial->m_pCoefficients[n] += d;
			dError = pPolynomial->DivideAndMeasureError(pRelation, pData, nOutputAttr);
			if(dError >= dBestError)
			{
				pPolynomial->m_pCoefficients[n] -= d;
				pPolynomial->m_pCoefficients[n] -= d;
				dError = pPolynomial->DivideAndMeasureError(pRelation, pData, nOutputAttr);
			}
			if(dError >= dBestError)
				pPolynomial->m_pCoefficients[n] += d;
			else
			{
				dBestError = dError;
				bGotOne = true;
			}
		}
		if(!bGotOne)
			dStep *= .5;
	}
	return pPolynomial;
}

// todo: merge with CalcIndex
inline int GetValueIndex(int nDimensions, int nControlPoints, int* pCoords)
{
	int nIndex = 0;
	int n;
	for(n = nDimensions - 1; n >= 0; n--)
	{
		nIndex *= nControlPoints;
		nIndex += pCoords[n];
	}
	return nIndex;
}

/*static*/ /*GPolynomial* GPolynomial::Newton(int nDimensions, int nControlPoints, double* pControlPoints, double* pValues)
{
	int* pCoords = (int*)alloca(nDimensions);
	int n, i;
	int nCoords = 1;
	for(n = nDimensions; n > 0; n--)
		nCoords *= nControlPoints;
	int nDim, nPoint;
	for(nPoint = 1; nPoint < nControlPoints; n++)
	{
		// Do a differencing pass in each dimension
		for(nDim = nDimensions - 1; nDim >= 0; nDim--)
		{
			// Initialize coord
			for(n = 0; n < nDimensions; n++)
				pCoords[n] = nControlPoints - 1;

			// Iterate through all coords
			for(n = nCoords - 1; n >= 0; n--)
			{
				// Difference on this dimension
				if(pCoords[nDim] >= nPoint)
				{
					xxx
				}

				// Move to next coord
				i = 0;
				while(true)
				{
					if(pCoords[i]-- == 0)
					{
						pCoords[i] = nControlPoints - 1;
						if(++i < nControlPoints)
							continue;
					}
					break;
				}
			}
		}
	}


xxx

	// Calculate the coefficients to Newton's blending functions
	double* pNC = (double*)alloca(nPoints * sizeof(double));
	memcpy(pNC, pFuncValues, nPoints * sizeof(double));
	int n, i;
	for(n = 1; n < nPoints; n++)
	{
		for(i = nPoints - n - 1; i >= 0; i--)
		{
			pNC[n + i] -= pNC[n + i - 1];
			pNC[n + i] /= (pTValues[n + i] - pTValues[i]);
		}
	}

	// Accumulate into polynomial coefficients
	double* pBlending = (double*)alloca(nPoints * sizeof(double));
	for(n = 1; n < nPoints; n++)
	{
		pBlending[n] = 0;
		pFuncValues[n] = 0;
	}
	pBlending[0] = 1;
	pFuncValues[0] = pNC[0];
	for(n = 1; n < nPoints; n++)
	{
		for(i = n; i > 0; i--)
			pBlending[i] -= pTValues[n - 1] * pBlending[i - 1];
		for(i = 0; i <= n; i++)
			pFuncValues[n - i] += pNC[n] * pBlending[i];
	}

}

*/
