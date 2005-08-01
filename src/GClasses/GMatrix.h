/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GMATRIX_H__
#define __GMATRIX_H__

class GMatrix
{
protected:
	double* m_pData;
	int m_nRows;
	int m_nColumns;

public:
	GMatrix(int nRows, int nColumns);
	virtual ~GMatrix();

	void SetToIdentity();
	double Get(int nRow, int nColumn);
	void Set(int nRow, int nColumn, double fValue);
	void Redim(int nRows, int nColumns);
	void Copy(const GMatrix* pMatrix);
	void Multiply(GMatrix* pA, GMatrix* pB);
	double GetDeterminant();
};

#endif // __GMATRIX_H__
