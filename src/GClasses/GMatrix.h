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

class GVector;

// A two-dimensional matrix
class GMatrix
{
protected:
	double* m_pData;
	int m_nRows;
	int m_nColumns;

public:
	GMatrix(int nRows = 0, int nColumns = 0);
	virtual ~GMatrix();

	// Get an element
	inline double Get(int nRow, int nColumn)
	{
		return m_pData[nRow * m_nColumns + nColumn];
	}

	// Set an element
	inline void Set(int nRow, int nColumn, double dValue)
	{
		m_pData[nRow * m_nColumns + nColumn] = dValue;
	}

	int GetColumnCount() { return m_nColumns; }
	int GetRowCount() { return m_nRows; }
	void SetToIdentity();
	void Transpose();
	void Resize(int nRows, int nColumns);
	void Copy(const GMatrix* pMatrix);
	void Multiply(GMatrix* pA, GMatrix* pB);
	double GetDeterminant();

	// Dumps a representation of the matrix to stdout
	void Print();

	// Dumps a partial representation of the matrix to stdout.
	// typically you will select a small number for n, like 2 or 3
	void PrintCorners(int n);
	void Invert();

	// Returns the sum of the diagonal values in the matrix
	double ComputeTrace();

	void Solve(double* pVector);

	void ComputeEigenVectors(GVector* pOutEigenValues, GMatrix* pOutEigenVectors);

	int CountNonZeroElements();
};

#endif // __GMATRIX_H__
