/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#include <stdio.h>
#include "MCollisionMap.h"
#include "../GClasses/GArray.h"
#include <stdlib.h>

class MCollisionMapNode
{
friend class MCollisionMap;
protected:
	bool m_bX;
	float m_fValue;
	MCollisionMapNode* m_pLesser;
	MCollisionMapNode* m_pGreater;

public:
	MCollisionMapNode(bool bX, float fValue)
	{
		m_bX = bX;
		m_fValue = fValue;
		m_pLesser = NULL;
		m_pGreater = NULL;
	}

	~MCollisionMapNode()
	{
		delete(m_pLesser);
		delete(m_pGreater);
	}
};

// ---------------------------------------------------------------------------------

MCollisionMap::MCollisionMap()
{
	m_pSolidRects = NULL;
	m_pRoot = NULL;
}

MCollisionMap::~MCollisionMap()
{
	if(m_pSolidRects)
	{
		int nCount = m_pSolidRects->GetSize();
		int n;
		for(n = 0; n < nCount; n++)
			delete((FRect*)m_pSolidRects->GetPointer(n));
		delete(m_pSolidRects);
	}
	delete(m_pRoot);
}

void MCollisionMap::AddSolidRect(FRect* pRect)
{
	if(!m_pSolidRects)
		m_pSolidRects = new GPointerArray(256);
	FRect* pNewRect = new FRect();
	memcpy(pNewRect, pRect, sizeof(FRect));
	m_pSolidRects->AddPointer(pNewRect);
}

void MCollisionMap::Compile()
{
	delete(m_pRoot);
	FRect bounds;
	bounds.x = -1e10;
	bounds.w = 2e10;
	bounds.y = -1e10;
	bounds.h = 2e10;
	if(m_pSolidRects)
	{
		m_pRoot = CompileArea(m_pSolidRects, &bounds);
		int nCount = m_pSolidRects->GetSize();
		int n;
		for(n = 0; n < nCount; n++)
			delete((FRect*)m_pSolidRects->GetPointer(n));
		delete(m_pSolidRects);
		m_pSolidRects = NULL;
	}
	else
		m_pRoot = new MCollisionMapNode(false, 0);
}

int CompareByLeftEdge(void* pThis, void* pA, void* pB)
{
	FRect* pC = (FRect*)pA;
	FRect* pD = (FRect*)pB;
	if(pC->x < pD->x)
		return -1;
	if(pC->x > pD->x)
		return 1;
	return 0;
}

int CompareByRightEdge(void* pThis, void* pA, void* pB)
{
	FRect* pC = (FRect*)pA;
	FRect* pD = (FRect*)pB;
	if(pC->x + pC->w < pD->x + pD->w)
		return -1;
	if(pC->x + pC->w > pD->x + pD->w)
		return 1;
	return 0;
}

int CompareByTopEdge(void* pThis, void* pA, void* pB)
{
	FRect* pC = (FRect*)pA;
	FRect* pD = (FRect*)pB;
	if(pC->y < pD->y)
		return -1;
	if(pC->y > pD->y)
		return 1;
	return 0;
}

int CompareByBottomEdge(void* pThis, void* pA, void* pB)
{
	FRect* pC = (FRect*)pA;
	FRect* pD = (FRect*)pB;
	if(pC->y + pC->h < pD->y + pD->h)
		return -1;
	if(pC->y + pC->h > pD->y + pD->h)
		return 1;
	return 0;
}

void MCollisionMap::EstimateBestDivision(GPointerArray* pSolidRects, bool* pbX, float* pfValue, FRect* pBounds)
{
	int nCount = pSolidRects->GetSize();
	FRect* pRect;
	while(true)
	{
		switch(rand() % 4)
		{
		case 0:
			pSolidRects->Sort(CompareByLeftEdge, NULL);
			*pbX = true;
			pRect = (FRect*)pSolidRects->GetPointer(nCount / 2);
			if(pRect->x <= pBounds->x)
				continue;
			*pfValue = pRect->x;
			break;

		case 1:
			pSolidRects->Sort(CompareByRightEdge, NULL);
			*pbX = true;
			pRect = (FRect*)pSolidRects->GetPointer(nCount / 2);
			if(pRect->x + pRect->w >= pBounds->x + pBounds->w)
				continue;
			*pfValue = pRect->x + pRect->w;
			break;

		case 2:
			pSolidRects->Sort(CompareByTopEdge, NULL);
			*pbX = false;
			pRect = (FRect*)pSolidRects->GetPointer(nCount / 2);
			if(pRect->y <= pBounds->y)
				continue;
			*pfValue = pRect->y;
			break;

		case 3:
			pSolidRects->Sort(CompareByBottomEdge, NULL);
			*pbX = false;
			pRect = (FRect*)pSolidRects->GetPointer(nCount / 2);
			if(pRect->y + pRect->h >= pBounds->y + pBounds->h)
				continue;
			*pfValue = pRect->y + pRect->h;
			break;

		default:
			GAssert(false, "math error");
		}
		return;
	}
}

int MCollisionMap::EvaluateDivision(GPointerArray* pSolidRects, bool bX, float fValue, FRect* pBounds)
{
	int nLesser = 0;
	int nGreater = 0;
	int nSplit = 0;
	int nCount = pSolidRects->GetSize();
	FRect* pRect;
	int n;
	if(bX)
	{
		if(fValue <= pBounds->x)
			return nCount * 10;
		if(fValue >= pBounds->x + pBounds->w)
			return nCount * 10;
		for(n = 0; n < nCount; n++)
		{
			pRect = (FRect*)pSolidRects->GetPointer(n);
			if(pRect->x >= fValue)
				nGreater++;
			else if(pRect->x + pRect->w <= fValue)
				nLesser++;
			else
				nSplit++;
		}
	}
	else
	{
		if(fValue <= pBounds->y)
			return nCount * 10;
		if(fValue >= pBounds->y + pBounds->h)
			return nCount * 10;
		for(n = 0; n < nCount; n++)
		{
			pRect = (FRect*)pSolidRects->GetPointer(n);
			if(pRect->y >= fValue)
				nGreater++;
			else if(pRect->y + pRect->y <= fValue)
				nLesser++;
			else
				nSplit++;
		}
	}
	int nError;
	if(nGreater > nLesser)
		nError = nGreater - nLesser + 3 * nSplit;
	else
		nError = nLesser - nGreater + 3 * nSplit;
	return nError;
}

bool MCollisionMap::FindBestDivision(GPointerArray* pSolidRects, bool* pbX, float* pfValue, FRect* pBounds)
{
	int nCount = pSolidRects->GetSize();
	FRect* pRect;
	bool bBestDimension = false;
	float fBestValue = 0;
	int nBestError = 10 * nCount;
	int nError;
	int n;
	for(n = 0; n < nCount; n++)
	{
		pRect = (FRect*)pSolidRects->GetPointer(n);
		nError = EvaluateDivision(pSolidRects, true, pRect->x, pBounds);
		if(nError < nBestError)
		{
			nBestError = nError;
			bBestDimension = true;
			fBestValue = pRect->x;
		}
		nError = EvaluateDivision(pSolidRects, true, pRect->x + pRect->w, pBounds);
		if(nError < nBestError)
		{
			nBestError = nError;
			bBestDimension = true;
			fBestValue = pRect->x + pRect->w;
		}
		nError = EvaluateDivision(pSolidRects, false, pRect->y, pBounds);
		if(nError < nBestError)
		{
			nBestError = nError;
			bBestDimension = false;
			fBestValue = pRect->y;
		}
		nError = EvaluateDivision(pSolidRects, false, pRect->y + pRect->h, pBounds);
		if(nError < nBestError)
		{
			nBestError = nError;
			bBestDimension = false;
			fBestValue = pRect->y + pRect->h;
		}
	}
	*pbX = bBestDimension;
	*pfValue = fBestValue;
	return nBestError < 10 * nCount;
}

MCollisionMapNode* MCollisionMap::CompileArea(GPointerArray* pSolidRects, FRect* pBounds)
{
	// Find the place to divide on
	bool bX;
	float fValue;
	int nCount = pSolidRects->GetSize();
	if(nCount > 128)
		EstimateBestDivision(pSolidRects, &bX, &fValue, pBounds);
	else
	{
		if(!FindBestDivision(pSolidRects, &bX, &fValue, pBounds))
		{
			bool bSolid = nCount > 0;
			MCollisionMapNode* pLeafNode = new MCollisionMapNode(bSolid, 0);
			return pLeafNode;
		}
	}

	// Divide into two lists
	GPointerArray arrLesser(MAX(8, nCount));
	GPointerArray arrGreater(MAX(8, nCount));
	GPointerArray arrSplits(MAX(8, nCount / 10));
	FRect* pRect;
	int n;
	for(n = 0; n < nCount; n++)
	{
		pRect = (FRect*)pSolidRects->GetPointer(n);
		if(bX)
		{
			if(pRect->x >= fValue)
				arrGreater.AddPointer(pRect);
			else if(pRect->x + pRect->w <= fValue)
				arrLesser.AddPointer(pRect);
			else
			{
				// Split the rect
				FRect* pLesserHalf = new FRect();
				FRect* pGreaterHalf = new FRect();
				memcpy(pLesserHalf, pRect, sizeof(FRect));
				pLesserHalf->w = fValue - pLesserHalf->x;
				memcpy(pGreaterHalf, pRect, sizeof(FRect));
				pGreaterHalf->w = pGreaterHalf->x + pGreaterHalf->w - fValue;
				pGreaterHalf->x = fValue;
				arrSplits.AddPointer(pLesserHalf);
				arrLesser.AddPointer(pLesserHalf);
				arrSplits.AddPointer(pGreaterHalf);
				arrGreater.AddPointer(pGreaterHalf);
			}
		}
		else
		{
			if(pRect->y >= fValue)
				arrGreater.AddPointer(pRect);
			else if(pRect->y + pRect->h <= fValue)
				arrLesser.AddPointer(pRect);
			else
			{
				// Split the rect
				FRect* pLesserHalf = new FRect();
				FRect* pGreaterHalf = new FRect();
				memcpy(pLesserHalf, pRect, sizeof(FRect));
				pLesserHalf->h = fValue - pLesserHalf->y;
				memcpy(pGreaterHalf, pRect, sizeof(FRect));
				pGreaterHalf->h = pGreaterHalf->y + pGreaterHalf->h - fValue;
				pGreaterHalf->y = fValue;
				arrSplits.AddPointer(pLesserHalf);
				arrLesser.AddPointer(pLesserHalf);
				arrSplits.AddPointer(pGreaterHalf);
				arrGreater.AddPointer(pGreaterHalf);
			}
		}
	}	

	// Split the bounds and recurse
	FRect rLesser;
	FRect rGreater;
	memcpy(&rLesser, pBounds, sizeof(FRect));
	memcpy(&rGreater, pBounds, sizeof(FRect));
	if(bX)
	{
		rLesser.w = fValue - rLesser.x;
		rGreater.w = rGreater.x + rGreater.w - fValue;
		rGreater.x = fValue;
	}
	else
	{
		rLesser.h = fValue - rLesser.y;
		rGreater.h = rGreater.y + rGreater.h - fValue;
		rGreater.y = fValue;
	}
	MCollisionMapNode* pNewNode = new MCollisionMapNode(bX, fValue);
	pNewNode->m_pLesser = CompileArea(&arrLesser, &rLesser);
	pNewNode->m_pGreater = CompileArea(&arrGreater, &rGreater);

	// Clean up the split rects
	int nSplitCount = arrSplits.GetSize();
	for(n = 0; n < nSplitCount; n++)
	{
		FRect* pRect = (FRect*)arrSplits.GetPointer(n);
		delete(pRect);
	}

	return pNewNode;
}

bool MCollisionMap::Check(float x, float y)
{
	MCollisionMapNode* pNode = m_pRoot;
	GAssert(pNode, "not compiled yet");
	while(pNode->m_pLesser)
	{
		if(pNode->m_bX)
		{
			if(x >= pNode->m_fValue)
				pNode = pNode->m_pGreater;
			else
				pNode = pNode->m_pLesser;
		}
		else
		{
			if(y >= pNode->m_fValue)
				pNode = pNode->m_pGreater;
			else
				pNode = pNode->m_pLesser;
		}
	}
	return pNode->m_bX;
}
