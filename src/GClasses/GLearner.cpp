/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GLearner.h"
#include "GArff.h"
#include <stdlib.h>
#include <string.h>
#include "GMacros.h"

GSupervisedLearner::GSupervisedLearner(GArffRelation* pRelation)
{
	m_pRelation = pRelation;
}

GSupervisedLearner::~GSupervisedLearner()
{
}

double GSupervisedLearner::MeasurePredictiveAccuracy(GArffData* pData)
{
	int nInputCount = m_pRelation->GetInputCount();
	int nOutputCount = m_pRelation->GetOutputCount();
	int nRowBytes = sizeof(double) * m_pRelation->GetAttributeCount();
	int nIndex;
	double* pSample = (double*)alloca(nRowBytes);
	double* pRow;
	int nRowCount = pData->GetRowCount();
	double dCorrectCount = 0;
	double d;
	int nTotalCount = 0;
	int n, i;
	for(n = 0; n < nRowCount; n++)
	{
		pRow = pData->GetRow(n);

		// Copy the input values into the sample
		for(i = 0; i < nInputCount; i++)
		{
			nIndex = m_pRelation->GetInputIndex(i);
			pSample[nIndex] = pRow[nIndex];
		}

		// Mess up the output values just to be safe
		for(i = 0; i < nOutputCount; i++)
		{
			nIndex = m_pRelation->GetOutputIndex(i);
			pSample[nIndex] = 1e100;
		}

		// Evaluate
		Eval(pSample);

		// Check the answer
		for(i = 0; i < nOutputCount; i++)
		{
			nIndex = m_pRelation->GetOutputIndex(i);
			if(m_pRelation->GetAttribute(nIndex)->IsContinuous())
			{
				// Predictive accuracy doesn't really make sense for real values,
				// so we'll just use a squashed squared error for an estimate
				d = pRow[nIndex] - pSample[nIndex];
				dCorrectCount += (1.0 - (1.0 / (1.0 + (d * d))));
			}
			else
			{
				if((int)pSample[nIndex] == (int)pRow[nIndex])
					dCorrectCount++;
			}
			nTotalCount++;
		}
	}
	return dCorrectCount / nTotalCount;
}

double GSupervisedLearner::MeasureMeanSquaredError(GArffData* pData)
{
	int nInputCount = m_pRelation->GetInputCount();
	int nOutputCount = m_pRelation->GetOutputCount();
	int nRowBytes = sizeof(double) * m_pRelation->GetAttributeCount();
	int nIndex;
	double* pSample = (double*)alloca(nRowBytes);
	double* pRow;
	int nRowCount = pData->GetRowCount();
	double dError = 0;
	double d;
	int n, i;
	for(n = 0; n < nRowCount; n++)
	{
		pRow = pData->GetRow(n);

		// Copy the input values into the sample
		for(i = 0; i < nInputCount; i++)
		{
			nIndex = m_pRelation->GetInputIndex(i);
			pSample[nIndex] = pRow[nIndex];
		}

		// Mess up the output values just to be safe
		for(i = 0; i < nOutputCount; i++)
		{
			nIndex = m_pRelation->GetOutputIndex(i);
			pSample[nIndex] = 1e100;
		}

		// Evaluate
		Eval(pSample);

		// Check the answer
		for(i = 0; i < nOutputCount; i++)
		{
			nIndex = m_pRelation->GetOutputIndex(i);
			if(m_pRelation->GetAttribute(nIndex)->IsContinuous())
			{
				d = pRow[nIndex] - pSample[nIndex];
				dError += (d * d);
			}
			else
			{
				// Squared error doesn't really make sense for discreet
				// values, so we'll just say an incorrect classification
				// corresponds to an error of 1.
				if((int)pSample[nIndex] != (int)pRow[nIndex])
					dError += 1;
			}
		}
	}
	return dError / (nRowCount * nOutputCount);
}
