/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GVector.h"
#include <stdio.h>
#include <string.h>
#include "GMacros.h"

GVector::GVector()
{
	m_pData = NULL;
	m_nSize = 0;
	m_bDeleteData = false;
}

GVector::GVector(int nSize)
{
	m_pData = NULL;
	SetData(new double[nSize], nSize, true);
	int n;
	for(n = 0; n < nSize; n++)
		m_pData[n] = 0;
}

GVector::GVector(double* pData, int nSize, bool bTakeOwnership)
{
	SetData(pData, nSize, bTakeOwnership);
}

GVector::~GVector()
{
	SetData(NULL, 0, false);
}

void GVector::Resize(int nSize)
{
	if(m_nSize == nSize)
		return;
	SetData(new double[nSize], nSize, true);
	int n;
	for(n = 0; n < nSize; n++)
		m_pData[n] = 0;
}

void GVector::Copy(double* pData, int nSize)
{
	if(m_nSize != nSize)
	{
		SetData(new double[nSize], nSize, true);
		m_bDeleteData = true;
		m_nSize = nSize;
	}
	memcpy(m_pData, pData, sizeof(double) * nSize);
}

double GVector::ComputeDotProduct(GVector* pThat)
{
	GAssert(m_nSize == pThat->m_nSize, "mismatch sizes");
	int n;
	double dVal = 0;
	for(n = 0; n < m_nSize; n++)
		dVal += m_pData[n] * pThat->m_pData[n];
	return dVal;
}

/*static*/ double GVector::ComputeDotProduct(double* pA, double* pB, int nSize)
{
	int n;
	double dVal = 0;
	for(n = 0; n < nSize; n++)
		dVal += pA[n] * pB[n];
	return dVal;
}

double* GVector::DropData()
{
	GAssert(m_bDeleteData, "Can't drop data I don't own");
	m_bDeleteData = false;
	return m_pData;
}

void GVector::SetData(double* pData, int nSize, bool bTakeOwnership)
{
	if(m_bDeleteData)
		delete(m_pData);
	m_pData = pData;
	m_nSize = nSize;
	m_bDeleteData = bTakeOwnership;
}

void GVector::Add(GVector* pThat)
{
	GAssert(pThat->m_nSize = m_nSize, "size mismatch");
	int n;
	for(n = 0; n < m_nSize; n++)
		m_pData[n] += pThat->m_pData[n];
}

int GVector::GetIndexOfBiggestValue()
{
	int nIndex = 0;
	int n;
	for(n = 1; n < m_nSize; n++)
	{
		if(m_pData[n] > m_pData[nIndex])
			nIndex = n;
	}
	return nIndex;
}

