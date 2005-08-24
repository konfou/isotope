/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "stdio.h"
#include "GDaftLearner.h"
#include "GPolynomial.h"
#include "GMacros.h"
#include "GImage.h"

class GDaftNode
{
public:
	GPolynomial* m_pPolyFit;
	GPolynomial* m_pPolyDivide;
	GDaftNode* m_pNegative;
	GDaftNode* m_pPositive;

	GDaftNode(GPolynomial* pPolyFit)
	{
		m_pPolyFit = pPolyFit;
		m_pPolyDivide = NULL;
		m_pNegative = NULL;
		m_pPositive = NULL;
	}

	~GDaftNode()
	{
		delete(m_pNegative);
		delete(m_pPositive);
		delete(m_pPolyDivide);
		delete(m_pPolyFit);
	}
};



GDaftLearner::GDaftLearner()
{
	m_pRed = NULL;
	m_pGreen = NULL;
	m_pBlue = NULL;
	m_pAlpha = NULL;
	m_nControlPoints = 3;
	m_dAcceptableError = .01;
}

GDaftLearner::~GDaftLearner()
{
	delete(m_pRed);
	delete(m_pGreen);
	delete(m_pBlue);
	delete(m_pAlpha);
}

void GDaftLearner::Train(GImage* pImage)
{
	int w = pImage->GetWidth();
	int h = pImage->GetHeight();
	GAssert(w <= 0xffff && h <= 0xffff, "image too big");
	int x, y;
	int n = 0;
	int* pPixels = new int[w * h];
	for(y = 0; y < h; y++)
	{
		for(x = 0; x < w; x++)
			pPixels[n++] = (x | (y << 16));
	}
	m_pRed = BuildBranch(pImage, pPixels, w * h, 0);
	m_pGreen = BuildBranch(pImage, pPixels, w * h, 1);
	m_pBlue = BuildBranch(pImage, pPixels, w * h, 2);
	m_pAlpha = BuildBranch(pImage, pPixels, w * h, 3);
	delete(pPixels);
}

GDaftNode* GDaftLearner::BuildBranch(GImage* pImage, int* pPixels, int nPixelCount, int nChannel)
{
/*
printf("Fitting...\n");
	// Fit the data
	int nOutputIndex = pRelation->GetOutputIndex(0);
	GPolynomial* pPolyFit = GPolynomial::FitData(pRelation, pData, nOutputIndex, m_nControlPoints);
	double dError = pPolyFit->MeasureMeanSquareError(pRelation, pData, nOutputIndex);
	GDaftNode* pNode = new GDaftNode(pPolyFit);
	int nCount = pData->GetRowCount();
	if(nCount < 8 || dError <= m_dAcceptableError)
		return pNode;
printf("Dividing %d...", nCount);
	// Subtract the polygon from the data
	int nInputs = pRelation->GetInputCount();
	double* pInputs = (double*)alloca(sizeof(double) * nInputs);
	double dEstimatedVal;
	int n, i;
	for(n = 0; n < nCount; n++)
	{
		double* pRow = pData->GetRow(n);
		for(i = 0; i < nInputs; i++)
			pInputs[i] = pRow[pRelation->GetInputIndex(i)];
		dEstimatedVal = pPolyFit->Eval(pInputs);
		pRow[nOutputIndex] -= dEstimatedVal;
	}

	// Divide the data
	pNode->m_pPolyDivide = GPolynomial::DivideData(pRelation, pData, nOutputIndex, m_nControlPoints);
	GArffData d1(nCount);
	GArffData d2(nCount);
	double dThresh;
	int nLastInput = pRelation->GetInputIndex(nInputs - 1);
	for(n = 0; n < nCount; n++)
	{
		double* pRow = pData->GetRow(n);
		for(i = 0; i < nInputs - 1; i++)
			pInputs[i] = pRow[pRelation->GetInputIndex(i)];
		dThresh = pNode->m_pPolyDivide->Eval(pInputs);
		if(dThresh >= pRow[nLastInput])
			d1.AddRow(pRow);
		else
			d2.AddRow(pRow);
	}
	if(d1.GetRowCount() * 5 < d2.GetRowCount() || d2.GetRowCount() * 5 < d1.GetRowCount())
	{
		// Couldn't divide it, so bail out
		delete(pNode->m_pPolyDivide);
		pNode->m_pPolyDivide = NULL;
		return pNode;
	}

printf("(%d, %d)\n", d1.GetRowCount(), d2.GetRowCount());
	// Recurse
	pNode->m_pNegative = BuildBranch(pRelation, &d1);
	pNode->m_pPositive = BuildBranch(pRelation, &d2);
	d1.DropAllRows();
	d2.DropAllRows();
	return pNode;
*/
	return NULL;
}
/*
void GDaftLearner::Eval(GArffRelation* pRelation, double* pRow)
{
	int nOutputIndex = pRelation->GetOutputIndex(0);
	int nInputs = pRelation->GetInputCount();
	int nLastInput = pRelation->GetInputIndex(nInputs - 1);
	double* pInputs = (double*)alloca(sizeof(double) * nInputs);
	GDaftNode* pNode = m_pRoot;
	double dThresh;
	int i;
	double dVal = 0;
	while(pNode)
	{
		for(i = 0; i < nInputs; i++)
			pInputs[i] = pRow[pRelation->GetInputIndex(i)];
		dVal += pNode->m_pPolyFit->Eval(pInputs);
		if(pNode->m_pPolyDivide)
		{
			dThresh = pNode->m_pPolyDivide->Eval(pInputs);
			if(dThresh >= pRow[nLastInput])
				pNode = pNode->m_pNegative;
			else
				pNode = pNode->m_pPositive;
		}
		else
			pNode = NULL;
	}
	pRow[nOutputIndex] = dVal;
}
*/