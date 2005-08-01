/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GHEAP_H__
#define __GHEAP_H__

#include <stdio.h>
#include <string.h>

class GHashTable;

//#define FIND_MEMORY_LEAK // Uncomment this to find where your memory leak is coming from


#define SMALL_BUCKET_COUNT 64

struct BlockHeader
{
	unsigned int m_nSize;
	struct BlockHeader* m_pNext;
	struct BlockHeader* m_pPrev;
};

class GHeap
{
protected:
	unsigned int m_nCurrentBlockTop;
	unsigned int m_nBiggestSmallFree;
	struct BlockHeader* m_pBlockHead;
	struct BlockHeader* m_pCurrentBlock;
	unsigned int* m_pSmallBuckets[SMALL_BUCKET_COUNT];
#ifdef FIND_MEMORY_LEAK
	GHashTable* m_pLeakTable;
#endif // FIND_MEMORY_LEAK

public:
#ifdef _DEBUG
	unsigned int m_nAllocs;
	unsigned int m_nDeallocs;
#endif // _DEBUG

	GHeap();
	virtual ~GHeap();

	// nSize is in uints (not bytes) and must be >= 4 uints
	unsigned int* Allocate(unsigned int nSize);
	void Deallocate(unsigned int* pObject);

	// Check to see if the values that wrap the object match
	// each other.  (Good for hunting down buffer overruns.)
	bool CheckObject(unsigned int* pObject);

protected:
	inline void LinkSmallObject(unsigned int* pObject, unsigned int nSize);
	inline void UnlinkSmallObject(unsigned int* pObject, unsigned int nSize);
	inline void LinkCurrentBlock();
	inline unsigned int* ReuseSmallObject(unsigned int nSize);
	inline unsigned int* UnlinkFirstSmallObject(unsigned int nSize);
	inline unsigned int* SplitSmallObject(unsigned int nOldSize, unsigned int nNewSize);
	inline unsigned int* AllocateInCurrentBlock(unsigned int nSize, unsigned int nFreeSpace);
	inline unsigned int* AllocateNewObject(unsigned int nSize);

	void DeallocateRemainderOfCurrentBlock();
	unsigned int* AllocateInNewBlock(unsigned int nSize);
};

#endif // __GHEAP_H__
