/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GArray.h"
#include "GMacros.h"

GSmallArray::GSmallArray(int nCellSize, int nGrowBy)
{
	GAssert(nCellSize > 0, "invalid cell size");
	GAssert(nGrowBy > 0, "invalid grow-by");
	m_nCellSize = nCellSize;
	m_nGrowBy = nGrowBy;
	m_nCellCount = 0;
	m_nAllocCount = 0;
	m_pData = NULL;
}

GSmallArray::~GSmallArray()
{
	delete((unsigned char*)m_pData);
}

void* GSmallArray::_GetCellRef(int n)
{
	GAssert(n >= 0 && n < m_nCellCount, "out of range");
	return((char*)m_pData + (m_nCellSize * n));
}

void GSmallArray::SetAllocSize(int n)
{
	if(m_nAllocCount == n)
		return;
	unsigned char* pOld = (unsigned char*)m_pData;
	m_pData = new unsigned char[n * m_nCellSize];
	GAssert(m_pData, "Out of Memory");
	if(m_nCellCount > n)
		m_nCellCount = n;
	memcpy(m_pData, pOld, m_nCellCount * m_nCellSize);
	m_nAllocCount = n;
	delete(pOld);
}

void GSmallArray::Grow()
{
	if(m_nAllocCount == 0)
		SetAllocSize(m_nGrowBy);
	else
		SetAllocSize(m_nAllocCount * 2);
}

void GSmallArray::_AddCellByRef(const void* pData)
{
	if(m_nCellCount >= m_nAllocCount)
		Grow();
	memcpy((char*)m_pData + (m_nCellCount * m_nCellSize), (char*)pData, m_nCellSize);
	m_nCellCount++;
}

void GSmallArray::_SetCellByRef(int nCell, void* pData)
{
	GAssert(nCell >= 0 && nCell < m_nCellCount, "Out of range");
	memcpy((char*)m_pData + (nCell * m_nCellSize), (char*)pData, m_nCellSize);
}

inline void SmartMemCpy(void* pDest, void* pSrc, int nSize)
{
	int i;
	if(pDest < pSrc)
	{
		for(i = 0; i < nSize; i++)
			((unsigned char*)pDest)[i] = ((unsigned char*)pSrc)[i];
	}
	else
	{
		for(i = nSize - 1; i >= 0; i--)
			((unsigned char*)pDest)[i] = ((unsigned char*)pSrc)[i];
	}
}

void GSmallArray::DeleteCell(int n)
{
	GAssert(n <= m_nCellCount, "Out of range");

	SmartMemCpy(
					(char*)m_pData + (n * m_nCellSize),
					(char*)m_pData + ((n + 1) * m_nCellSize),
					(m_nCellCount - n - 1) * m_nCellSize
					);
	m_nCellCount--;
}

void GSmallArray::_InsertCellByRef(int n, void* pData)
{
	if(m_nCellCount >= m_nAllocCount)
		Grow();
	SmartMemCpy(
					(char*)m_pData + ((n + 1) * m_nCellSize),
					(char*)m_pData + (n * m_nCellSize),
					(m_nCellCount - n) * m_nCellSize
					);
	memcpy((char*)m_pData + (n * m_nCellSize), (char*)pData, m_nCellSize);
	m_nCellCount++;
}

void GSmallArray::SetSize(int n)
{
	if(m_nAllocCount < m_nCellCount)
		SetAllocSize(n);
	m_nCellCount = n;
}

// -------------------------------------------------------------------------

// This uses merge sort.  todo: rewrite it so that we don't copy the
// entire buffer each iteration.  Just swap buffers instead.
void GPointerArray::Sort(PointerComparer pCompareFunc)
{
	int nCount = GetSize();
	void** pBuf = new void*[m_nAllocCount];
	void** pTmp;
	int nStep = 1;
	int n, i, j, k, end, cmp;
	while(nStep < nCount)
	{
		for(n = 0; n < nCount; n += (nStep + nStep))
		{
			i = nStep;
			if(n + nStep > nCount)
				i = nCount - n;
			j = nStep;
			if(n + nStep + j > nCount)
				j = nCount - (n + nStep);
			if(j > 0)
			{
				end = i + j - 1;
				for(k = end; k >= 0; k--)
				{
					if(!i)
						cmp = -1;
					else if(!j)
						cmp = 1;
					else
						cmp = pCompareFunc(GetPointer(n + i - 1), GetPointer(n + nStep + j - 1));
					if(cmp <= 0)
					{
						pBuf[n + k] = GetPointer(n + nStep + j - 1);
						j--;
					}
					else
					{
						pBuf[n + k] = GetPointer(n + i - 1);
						i--;
					}
				}
			}
			else
			{
				for(k = i - 1; k >= 0; k--)
					pBuf[n + k] = GetPointer(n + i - 1);
			}
		}
		pTmp = pBuf;
		pBuf = (void**)m_pData;
		m_pData = (void*)pTmp;
		nStep *= 2;
	}
	delete(pBuf);
}
