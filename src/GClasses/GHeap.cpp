/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GHeap.h"
#include "GMacros.h"
#include "GHashTable.h"

#define SMALLEST_LEFT_OVER 8
#define MIN_BLOCK_SIZE 4000
#define BLOCK_HEADER_SIZE_IN_UINTS (((sizeof(struct BlockHeader) + sizeof(unsigned int) - 1) & (~(sizeof(unsigned int) - 1))) / sizeof(unsigned int))


/*
	Blocks:
	-------
	The purpose of this Memory Manager is to improve the performance of object
	allocations by allocating them in blocks.  A block is a big chunk of memory
	that holds lots of objects.  If you allocate an object bigger than our hash
	table, then it just defaults to the regular C++ allocator.  My benchmarking
	indicates that this will make big allocations (bigger than SMALL_BUCKET_COUNT
	uints) 15% slower and small allocations 40% faster.  So if you do lots of
	small heap allocations, this is a good thing.

	Deallocated objects:
	--------------------
	When you deallocate an object, the object is flagged as reuseable and it's added
	to a hash table that holds linked lists of deallocated objects.  Each bucket
	points to the head of the list of objects all of the same size.  These objects
	will be reused later.

	When you deallocate an object, it checks the object's left and right neighbors
	in the block to see if it can merge with them (if they have been deallocated too).
	When you allocate an object, if there isn't an available object of the same size,
	it will try to pick a bigger one and break it up.  If a bigger one can't be found,
	it will allocate new space (adding a new block if necessary).

	Layout of an object:
	--------------------
	Sizes always refer to the number of unsigned ints, not the number of bytes.
	Every object is padded before and after the payload with a single unsigned int.
	The unsigned int on both sides should be equal.  If they are not, memory
	corruption has occurred.  So whenever you allocate an object, it actually
	allocates (size + 2) * sizeof(unsigned int) bytes.  Like this:

	[Value]
	[Payload]
	[Value]
	
	where "Value" is an unsigned int of this form:
		The first 8 bits are flags and the rest indicate the size (uint count) of the
		object.  So if you take this value and shift it right 8 bits, you'll get the
		uint count size of the entire object (including payload and both padded values.)
		Only the first 3 bits of the flags are used.  The other 5 bits are wasted space.
		
		Bit 0:	0 = deallocated (waiting to be reused), 1 = allocated (in use)
		Bit 1:	0 = this is the right-most object in the block, 1 = has a right neighbor  
		Bit 2:  0 = this is the left-most object in the block, 1 = has a left neighbor

		So in the code below, you'll see a lot of '&' and '|' logic with the values
		1, 2, and 4.  It's just twiddling with these bits.

	If the object is flagged as deallocated, the first two units (after the first "Value")
	are used as next/prev pointers (if it goes in a linked list of small objects), or as
	left-child/right-child pointers (if it goes in the AVL tree of big objects).
*/

#define FLAG_MASK 0xff
#define FLAG_BITS 8

#define FLAG_ALLOCATED 1
#define FLAG_RIGHT_NEIGHBOR 2
#define FLAG_LEFT_NEIGHBOR 4

GHeap::GHeap()
{
	m_nBiggestSmallFree = 0;
	m_pBlockHead = NULL;
	m_pCurrentBlock = NULL;
	memset(m_pSmallBuckets, '\0', SMALL_BUCKET_COUNT * sizeof(unsigned int*));
#ifdef _DEBUG
	m_nAllocs = 0;
	m_nDeallocs = 0;
#endif // _DEBUG
#ifdef FIND_MEMORY_LEAK
	m_pLeakTable = new GHashTable(137);
#endif // FIND_MEMORY_LEAK
}

GHeap::~GHeap()
{
#ifdef FIND_MEMORY_LEAK
	if(m_pLeakTable->GetCount() > 0)
	{
		GHashTableEnumerator e(m_pLeakTable);
		unsigned int* pObj = (unsigned int*)e.GetNextKey();
		unsigned int nAllocID;
		m_pLeakTable->Get(pObj, (void**)&nAllocID);
		GAssert(false, "Found leaking object!");
	}
#endif // FIND_MEMORY_LEAK
	GAssert(m_nAllocs == m_nDeallocs, "different number of allocations and deallocations");
	struct BlockHeader* pTmp;
	while(m_pBlockHead)
	{
		pTmp = m_pBlockHead;
		m_pBlockHead = m_pBlockHead->m_pNext;
		delete [] ((unsigned int*)pTmp);
	}
	delete [] (unsigned int*)m_pCurrentBlock;
#ifdef FIND_MEMORY_LEAK
	m_pLeakTable = new GHashTable(137);
#endif // FIND_MEMORY_LEAK
}

inline unsigned int* GHeap::ReuseSmallObject(unsigned int nSize)
{
	// Get the object
	unsigned int* pObject = UnlinkFirstSmallObject(nSize);

	// Flag it as allocated
	int nVal = (nSize << FLAG_BITS) | ((*pObject) & FLAG_MASK) | FLAG_ALLOCATED;
	*pObject = nVal;
	*(pObject + nSize - 1) = nVal;

	return pObject + 1;
}

inline void GHeap::LinkSmallObject(unsigned int* pObject, unsigned int nSize)
{
	GAssert(((*pObject) & FLAG_ALLOCATED) == 0, "this object has not been deleted yet");

	// Set next pointer
	unsigned int* pNextObject = m_pSmallBuckets[nSize];
	unsigned int* pTmp = pObject + 1;
	*pTmp = (unsigned int)pNextObject;

	// Set next object's previous pointer
	if(pNextObject)
		*(unsigned int**)(pNextObject + 2) = pObject;

	// Set previous pointer
	pTmp++;
	*pTmp = 0;	

	// Set the list head
	m_pSmallBuckets[nSize] = pObject;

	// Set m_nBiggestSmallFree
	if(nSize > m_nBiggestSmallFree)
		m_nBiggestSmallFree = nSize;
}

inline unsigned int* GHeap::UnlinkFirstSmallObject(unsigned int nSize)
{
	// Get the object
	unsigned int* pObject = m_pSmallBuckets[nSize];
	GAssert(pObject, "Expected an object to reuse");

	// Set new list head
	unsigned int* pNewHead = *(unsigned int**)(pObject + 1);
	m_pSmallBuckets[nSize] = pNewHead;

	if(pNewHead)
	{
		// Set the new head's previous pointer to NULL
		*(unsigned int**)(m_pSmallBuckets[nSize] + 2) = NULL;
	}
	else
	{
		// Clear m_nBiggestSmallFree
		if(m_nBiggestSmallFree == nSize)
		{
			// Try to find a new one
			if(m_pSmallBuckets[SMALL_BUCKET_COUNT - 1])
				m_nBiggestSmallFree = SMALL_BUCKET_COUNT - 1;
			else if(nSize < SMALL_BUCKET_COUNT - 1 && m_pSmallBuckets[nSize + 1])
				m_nBiggestSmallFree = nSize + 1;
			else if(m_pSmallBuckets[nSize - 1])
				m_nBiggestSmallFree = nSize - 1;
			else if(m_pSmallBuckets[((SMALL_BUCKET_COUNT - 1) / nSize) * nSize])
				m_nBiggestSmallFree = ((SMALL_BUCKET_COUNT - 1) / nSize) * nSize;
			else
				m_nBiggestSmallFree = 0;
		}
	}
	return pObject;
}

inline void GHeap::UnlinkSmallObject(unsigned int* pObject, unsigned int nSize)
{
	// See if it's the first one in the list
	unsigned int* pPrevObject = *(unsigned int**)(pObject + 2);
	if(!pPrevObject)
	{
		GAssert(m_pSmallBuckets[nSize] == pObject, "problem with small object buckets");
		UnlinkFirstSmallObject(nSize);
		return;
	}

	// Set next pointer of previous object
	unsigned int* pNextObject = *(unsigned int**)(pObject + 1);
	*(unsigned int**)(pPrevObject + 2) = pNextObject;

	// Set prev pointer of next object
	if(pNextObject)
		*(unsigned int**)(pNextObject + 1) = pPrevObject;
}

inline unsigned int* GHeap::SplitSmallObject(unsigned int nOldSize, unsigned int nNewSize)
{
	// Get the object
	unsigned int* pObject = UnlinkFirstSmallObject(nOldSize);
	unsigned int* pExtraObject = pObject + nNewSize;

	// Flag first half as allocated
	unsigned int nOldVal = *pObject;
	unsigned int nVal = (nNewSize << FLAG_BITS) | (nOldVal & FLAG_LEFT_NEIGHBOR) | FLAG_RIGHT_NEIGHBOR | FLAG_ALLOCATED;
	*pObject = nVal;
	*(pObject + nNewSize - 1) = nVal;

	// Flag second half as allocated
	nVal = ((nOldSize - nNewSize) << FLAG_BITS) | FLAG_LEFT_NEIGHBOR | (nOldVal & FLAG_RIGHT_NEIGHBOR) | FLAG_ALLOCATED;
	*pExtraObject = nVal;
	*(pObject + nOldSize - 1) = nVal;
#ifdef FIND_MEMORY_LEAK
	m_pLeakTable->Add(pExtraObject, (const void*)m_nAllocs);
#endif // FIND_MEMORY_LEAK
#ifdef _DEBUG
	m_nAllocs++;
#endif // _DEBUG

	// Deallocate the extra space
	Deallocate(pExtraObject + 1);

	return pObject + 1;
}

inline void GHeap::LinkCurrentBlock()
{
	GAssert(m_pCurrentBlock->m_pNext == NULL && m_pCurrentBlock->m_pPrev == NULL, "Already linked");
	if(m_pBlockHead)
	{
		GAssert(m_pBlockHead->m_pPrev == NULL, "Linked List error");
		m_pBlockHead->m_pPrev = m_pCurrentBlock;
	}
	m_pCurrentBlock->m_pNext = m_pBlockHead;
	m_pBlockHead = m_pCurrentBlock;
	m_pCurrentBlock = NULL;
	m_nCurrentBlockTop = 0;
}

inline unsigned int* GHeap::AllocateInCurrentBlock(unsigned int nSize, unsigned int nFreeSpace)
{
	// Find the object to use
	GAssert(m_pCurrentBlock, "No current block");
	unsigned int* pObject = ((unsigned int*)m_pCurrentBlock) + BLOCK_HEADER_SIZE_IN_UINTS + m_nCurrentBlockTop;
	unsigned int nVal = (nSize << FLAG_BITS) | FLAG_LEFT_NEIGHBOR | FLAG_RIGHT_NEIGHBOR | FLAG_ALLOCATED;

	// See if there's room for another object after this one
	if(nFreeSpace - nSize <= SMALLEST_LEFT_OVER)
	{
		// Finish up the block
		nSize = nFreeSpace;
		nVal &= (~FLAG_RIGHT_NEIGHBOR);
		nVal &= FLAG_MASK;
		nVal |= (nSize << FLAG_BITS);

		// Move the current block to the linked list of blocks
		LinkCurrentBlock();
	}
	else
		m_nCurrentBlockTop += nSize;

	// Flag the new object as allocated
	*pObject = nVal;
	*(pObject + nSize - 1) = nVal;
	return pObject + 1;
}

inline unsigned int* GHeap::AllocateNewObject(unsigned int nSize)
{
	if(m_pCurrentBlock)
	{
		// See if there's room in the current block
		GAssert(m_pCurrentBlock->m_nSize >= m_nCurrentBlockTop && m_pCurrentBlock->m_nSize < 4 * MIN_BLOCK_SIZE, "Corrupt block");
		unsigned int nFreeSpace = m_pCurrentBlock->m_nSize - m_nCurrentBlockTop;
		if(nSize <= nFreeSpace)
			return AllocateInCurrentBlock(nSize, nFreeSpace);
		else
		{
			DeallocateRemainderOfCurrentBlock();
			return AllocateInNewBlock(nSize);
		}
	}
	return AllocateInNewBlock(nSize);
}

void GHeap::DeallocateRemainderOfCurrentBlock()
{
	GAssert(m_pCurrentBlock, "No current block");
	unsigned int* pObject = ((unsigned int*)m_pCurrentBlock) + BLOCK_HEADER_SIZE_IN_UINTS + m_nCurrentBlockTop;
	unsigned int nSize = m_pCurrentBlock->m_nSize - m_nCurrentBlockTop;
	if(nSize > 0)
	{
		GAssert(nSize >= SMALLEST_LEFT_OVER, "Left over space too small");
		unsigned int nVal = (nSize << 8) | 4 | 1;
		*pObject = nVal;
		*(pObject + nSize - 1) = nVal;
#ifdef _DEBUG
		m_nAllocs++;
#endif // _DEBUG
#ifdef FIND_MEMORY_LEAK
		m_pLeakTable->Add(pObject + 1, (const void*)-1);
#endif // FIND_MEMORY_LEAK
		Deallocate(pObject + 1);
	}
	LinkCurrentBlock();
}

unsigned int* GHeap::AllocateInNewBlock(unsigned int nSize)
{
	// Pick a good size for the new block
	GAssert(!m_pCurrentBlock, "There's already a current block");
	int nBlockSize = MIN_BLOCK_SIZE;
/*	if(nSize > MIN_BLOCK_SIZE / 3)
	{
		if(nSize > MIN_BLOCK_SIZE)
			nBlockSize = nSize;
		else
		{
			if(nSize > MIN_BLOCK_SIZE / 2)
				nBlockSize = nSize * 2;
			else
				nBlockSize = nSize * 3;
		}
	}*/

	// Allocate the block
	m_pCurrentBlock = (struct BlockHeader*)new unsigned int[BLOCK_HEADER_SIZE_IN_UINTS + nBlockSize];
	m_pCurrentBlock->m_pNext = NULL;
	m_pCurrentBlock->m_pPrev = NULL;
	m_pCurrentBlock->m_nSize = nBlockSize;
	unsigned int* pObject = ((unsigned int*)m_pCurrentBlock) + BLOCK_HEADER_SIZE_IN_UINTS;
	unsigned int nVal = (nSize << 8) | 2 | 1;
	*pObject = nVal;
	*(pObject + nSize - 1) = nVal;
	m_nCurrentBlockTop = nSize;
	return pObject + 1;
}

unsigned int* GHeap::Allocate(unsigned int nSize)
{
	GAssert(nSize < 0x00ffffff, "That's more than 16 MB!  If you're sure you intended to allocate an object that big, then you'll need to remove this check");
	nSize += 2;
	unsigned int* pObj;
	if(nSize < SMALL_BUCKET_COUNT)
	{
		GAssert(nSize >= 4, "too small to hold deleted object info when deleted");
		if(m_pSmallBuckets[nSize])
			pObj = ReuseSmallObject(nSize);
		else if(m_nBiggestSmallFree > nSize)
		{
			// See if the extra space is too small to break up
			GAssert(m_pSmallBuckets[m_nBiggestSmallFree], "Bad value for m_nBiggestSmallFree");
			if(m_nBiggestSmallFree - nSize < SMALLEST_LEFT_OVER)
				pObj = ReuseSmallObject(m_nBiggestSmallFree);
			else
				pObj = SplitSmallObject(m_nBiggestSmallFree, nSize);
		}
		else
			pObj = AllocateNewObject(nSize);
	}
	else
	{
		unsigned int nVal = (nSize << 8) | 1;
		unsigned int* pObject = new unsigned int[nSize];
		*pObject = nVal;
		*(pObject + nSize - 1) = nVal;
		pObj = pObject + 1;
	}

#ifdef FIND_MEMORY_LEAK
	m_pLeakTable->Add(pObj, (const void*)m_nAllocs);
#endif // FIND_MEMORY_LEAK
#ifdef _DEBUG
	m_nAllocs++;
#endif // _DEBUG
	return pObj;
}

void GHeap::Deallocate(unsigned int* pObject)
{
#ifdef FIND_MEMORY_LEAK
	unsigned int nAllocID;
	if(!m_pLeakTable->Get(pObject, (void**)&nAllocID))
		GAssert(false, "This object is not tracked");
	m_pLeakTable->Remove(pObject);
#endif // FIND_MEMORY_LEAK
#ifdef _DEBUG
	m_nDeallocs++;
#endif // _DEBUG

	pObject--;
	unsigned int nVal = *pObject;
	unsigned int nSize = nVal >> 8;
	GAssert(*(pObject + nSize - 1) == nVal, "Memory corruption!  Buffer overrun!  Oh no!");
	GAssert(nVal & FLAG_ALLOCATED, "The object has already been deallocated");

	if(nSize >= SMALL_BUCKET_COUNT)
	{
		delete [] pObject;
		return;
	}

// todo: fix and uncomment this code
/*
	// try to merge with left neighbor
	if((nVal & FLAG_LEFT_NEIGHBOR) && (((*(pObject - 1)) & FLAG_ALLOCATED) == 0))
	{
		// Unlink left neighbor
		unsigned int nValTmp = *(pObject - 1);
		unsigned int nSizeTmp = nValTmp >> FLAG_BITS;
		if(nSize + nSizeTmp < SMALL_BUCKET_COUNT) // Make sure we're not going to get too big
		{
			UnlinkSmallObject(pObject - nSizeTmp, nSizeTmp);
			pObject -= nSizeTmp;

			// Recalculate new flags
			nSize += nSizeTmp;
			nVal &= (~FLAG_LEFT_NEIGHBOR);
			nVal |= (nValTmp & FLAG_LEFT_NEIGHBOR);
			nVal = (nSize << FLAG_BITS) | (nVal & FLAG_MASK);
		}
	}

	// try to merge with right neighbor
	if((nVal & FLAG_RIGHT_NEIGHBOR) && (((*(pObject + nSize)) & FLAG_ALLOCATED) == 0))
	{
		// Unlink right neighbor
		unsigned int nValTmp = *(pObject + nSize);
		unsigned int nSizeTmp = nValTmp >> 8;
		if(nSize + nSizeTmp < SMALL_BUCKET_COUNT) // Make sure we're not going to get too big
		{
			UnlinkSmallObject(pObject + nSize, nSizeTmp);

			// Recalculate new flags
			nSize += nSizeTmp;
			nVal &= (~FLAG_RIGHT_NEIGHBOR);
			nVal |= (nValTmp & FLAG_RIGHT_NEIGHBOR);
			nVal = (nSize << FLAG_BITS) | (nVal & FLAG_MASK);
		}
	}
*/
	// Mark it as deallocated
	nVal &= (~FLAG_ALLOCATED);
	*pObject = nVal;
	*(pObject + nSize - 1) = nVal;

	// Put it somewhere where we can find it when we need it
	if(nSize < SMALL_BUCKET_COUNT)
		LinkSmallObject(pObject, nSize);
	else
	{
		GAssert(false, "todo: link big object");
	}
}

bool GHeap::CheckObject(unsigned int* pObject)
{
	pObject--;
	unsigned int nVal = *pObject;
	unsigned int nSize = nVal >> FLAG_BITS;
	if(*(pObject + nSize - 1) == nVal)
		return true;
	else
		return false;
}

