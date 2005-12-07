/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GVECTOR_H__
#define __GVECTOR_H__

class GVector
{
protected:
	double* m_pData;
	int m_nSize;
	bool m_bDeleteData;

public:
	GVector();
	GVector(int nSize);
	GVector(double* pData, int nSize, bool bTakeOwnership);
	~GVector();

	double ComputeDotProduct(GVector* pThat);
	static double ComputeDotProduct(double* pA, double* pB, int nSize);
	double* GetData() { return m_pData; }
	double* DropData();
	void Copy(double* pData, int nSize);
	void SetData(double* pData, int nSize, bool bTakeOwnership);
	double Get(int n) { return m_pData[n]; }
	void Set(int index, double val) { m_pData[index] = val; }
	void Add(int index, double val) { m_pData[index] += val; }
	void Add(GVector* pThat);
	int GetSize() { return m_nSize; }
	void Resize(int nSize);
	int GetIndexOfBiggestValue();
};

#endif // __GVECTOR_H__

