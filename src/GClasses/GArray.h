/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GARRAY_H__
#define __GARRAY_H__

#include <stdio.h>
#include "GMacros.h"
#ifdef WIN32
#include <winsock2.h>
#endif // WIN32


class GSmallArray
{
protected:
	int m_nCellSize;
	int m_nCellCount;
	int m_nAllocCount;
	int m_nGrowBy;
	void* m_pData;

public:
	// nCellSize = size of a single element in bytes.
	// nGrowBy = number of elements to initially allocate space
	//           for when the first element is added to the array
	GSmallArray(int nCellSize, int nGrowBy);
	virtual ~GSmallArray();

	// Returns a reference to element n
	void* _GetCellRef(int n);

	// Adds an element (referenced by pData) to the end of the array.
	void _AddCellByRef(const void* pData);

	// Replaces element nCell with the element reference by pData
	void _SetCellByRef(int nCell, void* pData);

	// Deletes an element and moves all subsequent elements over
	// to fill the gap.  (If order is not important in your
	// array, it's a good idea to swap the cell you want to
	// delete with the last element before you delete
	// it so that you won't have to pay the performance
	// penalty for shifting the whole array over.)
	void DeleteCell(int n);

	// Inserts an element (referenced by pData) into the array at location n.
	// (It will shift all subsequent elements over to make room.)
	void _InsertCellByRef(int n, void* pData);

	// Returns the number of elements in the array
	int GetSize() { return m_nCellCount; }

	// Resizes the array (throwing out elements >= n)
	void SetSize(int n);

	// Resizes the array to zero elements
	void Clear() { SetSize(0); }

	void SetAllocSize(int n);
	int GetAllocSize() { return m_nAllocCount; }

	// Make the allocated buffer bigger
	void Grow();
};

class GHandleArray : public GSmallArray
{
public:
	GHandleArray(int nGrowBy) : GSmallArray(sizeof(HANDLE), nGrowBy) { }
	virtual ~GHandleArray() { }

	HANDLE GetHandle(int nIndex) { return *(HANDLE*)_GetCellRef(nIndex); }
	void AddHandle(HANDLE h) { _AddCellByRef(&h); }
	void SetHandle(int nCell, HANDLE h) { _SetCellByRef(nCell, &h); }
};


typedef int (*PointerComparer)(void* pThis, void* pA, void* pB);

// Dynamic array of pointers
class GPointerArray : public GSmallArray
{
public:
	GPointerArray(int nGrowBy) : GSmallArray(sizeof(void*), nGrowBy) { }
	virtual ~GPointerArray() { }

	void* GetPointer(int nIndex) { return *(void**)_GetCellRef(nIndex); }
	void AddPointer(const void* p) { _AddCellByRef(&p); }
	void InsertPointer(int nPos, void* p) { _InsertCellByRef(nPos, &p); }
	void SetPointer(int nCell, void* p) { _SetCellByRef(nCell, &p); }

	// Sorts the array with merge sort
	void Sort(PointerComparer pCompareFunc, void* pThis);
};

// Dynamic array of integers
class GIntArray : public GSmallArray
{
public:
	GIntArray(int nGrowBy) : GSmallArray(sizeof(int), nGrowBy) { }
	virtual ~GIntArray() { }

	int GetInt(int nIndex) { return *(int*)_GetCellRef(nIndex); }
	void AddInt(int n) { _AddCellByRef(&n); }
	void InsertInt(int nPos, int nVal) { _InsertCellByRef(nPos, &nVal); }
	void SetInt(int nCell, int n) { _SetCellByRef(nCell, &n); }
};

// Dynamic array of floats
class GFloatArray : public GSmallArray
{
public:
	GFloatArray(int nGrowBy) : GSmallArray(sizeof(float), nGrowBy) { }
	virtual ~GFloatArray() { }

	float GetFloat(int nIndex) { return *(float*)_GetCellRef(nIndex); }
	void AddFloat(float f) { _AddCellByRef(&f); }
	void SetFloat(int nCell, float f) { _SetCellByRef(nCell, &f); }
};

// Dynamic array of doubles
class GDoubleArray : public GSmallArray
{
public:
	GDoubleArray(int nGrowBy) : GSmallArray(sizeof(double), nGrowBy) { }
	virtual ~GDoubleArray() { }

	double GetDouble(int nIndex) { return *(double*)_GetCellRef(nIndex); }
	void AddDouble(double d) { _AddCellByRef(&d); }
	void SetDouble(int nCell, double d) { _SetCellByRef(nCell, &d); }
};

#endif // __GARRAY_H__
