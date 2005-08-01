/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GMatrix.h"
#include "GMacros.h"

GMatrix::GMatrix(int nRows, int nColumns)
{
	m_nRows = nRows;
	m_nColumns = nColumns;
	m_pData = new double[nRows * nColumns];
	int n;
	int nSize = nRows * nColumns;
	for(n = 0; n < nSize; n++)
		m_pData[n] = 0.0f;
}

GMatrix::~GMatrix()
{
	delete(m_pData);
}

void GMatrix::SetToIdentity()
{
	int nRow, nColumn;
	for(nRow = 0; nRow < m_nRows; nRow++)
	{
		for(nColumn = 0; nColumn < m_nColumns; nColumn++)
			Set(nRow + 1, nColumn + 1, nRow == nColumn ? 1.0f : 0.0f);
	}
}

double GMatrix::Get(int nRow, int nColumn)
{
	return m_pData[(nRow - 1) * m_nColumns + nColumn - 1];
}

void GMatrix::Set(int nRow, int nColumn, double fValue)
{
	m_pData[(nRow - 1) * m_nColumns + nColumn - 1] = fValue;
}

void GMatrix::Redim(int nRows, int nColumns)
{
	if(m_nRows == nRows && m_nColumns == nColumns)
		return;
	delete(m_pData);
	m_pData = new double[nRows * nColumns];
	m_nRows = nRows;
	m_nColumns = nColumns;
}

void GMatrix::Copy(const GMatrix* pMatrix)
{
	Redim(pMatrix->m_nRows, pMatrix->m_nColumns);
	int nRow, nColumn;
	for(nRow = 0; nRow < m_nRows; nRow++)
	{
		for(nColumn = 0; nColumn < m_nColumns; nColumn++)
			m_pData[nRow * m_nColumns + nColumn] = pMatrix->m_pData[nRow * m_nColumns + nColumn];
	}
}

void GMatrix::Multiply(GMatrix* pA, GMatrix* pB)
{
	if(pA->m_nColumns != pB->m_nRows)
	{
		GAssert(false, "Bad dimensions");
		return;
	}
	Redim(pA->m_nRows, pB->m_nColumns);
	double fSum;
	int nPos;
	int nRow;
	int nColumn;
	for(nRow = 0; nRow < pA->m_nRows; nRow++)
	{
		for(nColumn = 0; nColumn < pB->m_nColumns; nColumn++)
		{
			fSum = 0;
			for(nPos = 0; nPos < pA->m_nColumns; nPos++)
				fSum += pA->Get(nRow + 1, nPos + 1) * pB->Get(nPos + 1, nColumn + 1);
			Set(nRow + 1, nColumn + 1, fSum);
		}
	}
}

double GMatrix::GetDeterminant()
{
	if(m_nRows != m_nColumns)
	{
		GAssert(false, "Rows not equal to Columns");
		return 0;
	}
	if(m_nRows < 2)
		return Get(1, 1);
	if(m_nRows == 2)
		return Get(1, 1) * Get(2, 2) - Get(1, 2) * Get(2, 1);
	else
	{
		GMatrix TmpMatrix(m_nRows - 1, m_nRows - 1);
		double fSum = 0;
		int nRow, nColumn;
		int n;
		int nC;
		for(n = 1; n <= m_nColumns; n++)
		{
			for(nColumn = 1; nColumn < m_nColumns; nColumn++)
			{
				nC = n + nColumn;
				if(nC > m_nColumns)
					nC -= m_nColumns;
				for(nRow = 1; nRow < m_nRows; nRow++)
					TmpMatrix.Set(nRow, nColumn, Get(nRow + 1, nC));
			}
			fSum += Get(1, n) * TmpMatrix.GetDeterminant();
		}
		return fSum;
	}
}
