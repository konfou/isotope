/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include <stdio.h>
#include "GHashTable.h"
#include "GMacros.h"
#include "GQueue.h"

struct HashBucket
{
	HashBucket* pPrev;
	HashBucket* pNext;
	const char* pKey;
	const void* pValue;
};

GHashTableBase::GHashTableBase(int nInitialBucketCount)
{
	m_nBucketCount = 0;
	m_pBuckets = NULL;
	m_nCount = 0;
	m_nModCount = 0;
	_Resize(nInitialBucketCount);
}

GHashTableBase::~GHashTableBase()
{
	delete(m_pBuckets);
}

inline bool IsObviousNonPrime(int n)
{
	if((n % 3) == 0)
		return true;
	if((n % 5) == 0)
		return true;
	if((n % 7) == 0 && n != 7)
		return true;
	if((n % 11) == 0 && n != 11)
		return true;
	if((n % 13) == 0 && n != 13)
		return true;
	if((n % 17) == 0 && n != 17)
		return true;
	return false;
}

void GHashTableBase::_Resize(int nNewSize)
{
	// Find a good size
	if(nNewSize < m_nCount * 3)
		nNewSize = m_nCount * 3;
	if(nNewSize < 7)
		nNewSize = 7;
	if((nNewSize & 1) == 0)
		nNewSize++;
	while(IsObviousNonPrime(nNewSize))
		nNewSize += 2;

	// Allocate the new buckets
	struct HashBucket* pOldBuckets = m_pBuckets;
	m_pBuckets = new struct HashBucket[nNewSize];
	int nOldCount = m_nBucketCount;
	m_nBucketCount = nNewSize;
	m_nCount = 0;

	// Init the new buckets
	m_pBuckets[0].pPrev = NULL;
	m_pBuckets[0].pNext = &m_pBuckets[1];
	m_pBuckets[0].pKey = NULL;
	int n;
	int nNewSizeMinusOne = nNewSize - 1;
	for(n = 1; n < nNewSizeMinusOne; n++)
	{
		m_pBuckets[n].pPrev = &m_pBuckets[n - 1];
		m_pBuckets[n].pNext = &m_pBuckets[n + 1];
		m_pBuckets[n].pKey = NULL;
	}
	m_pBuckets[nNewSizeMinusOne].pPrev = &m_pBuckets[nNewSizeMinusOne - 1];
	m_pBuckets[nNewSizeMinusOne].pNext = NULL;
	m_pBuckets[nNewSizeMinusOne].pKey = NULL;
	m_pFirstEmpty = &m_pBuckets[0];

	// Copy the old data
	for(n = 0; n < nOldCount; n++)
	{
		if(pOldBuckets[n].pKey)
			_Add(pOldBuckets[n].pKey, pOldBuckets[n].pValue);
	}

	// delete the old buckets
	delete pOldBuckets;
	m_nModCount++;
}

void GHashTableBase::_Add(const char* pKey, const void* pValue)
{
	// Check inputs
	GAssert(pKey, "pKey can't be NULL");

	// Resize if necessary
	if(m_nCount * 2 > m_nBucketCount)
		_Resize(m_nBucketCount * 2);
	else
		m_nModCount++;

	// Determine which bucket
	unsigned int nPos = Hash(pKey, m_nBucketCount);
	GAssert(nPos < (unsigned int)m_nBucketCount, "Out of range");

	// Insert it
	m_nCount++;
	if(m_pBuckets[nPos].pKey)
	{
		// The bucket is occupied, so either boot them out or rent a place yourself
		if(m_pBuckets[nPos].pPrev)
		{
			// The bucket is being rented by someone else.  Boot them out to the next available empty spot.
			struct HashBucket* pRenter = m_pFirstEmpty;
			m_pFirstEmpty = pRenter->pNext;
			m_pFirstEmpty->pPrev = NULL;
			*pRenter = m_pBuckets[nPos];
			pRenter->pPrev->pNext = pRenter;
			if(pRenter->pNext)
				pRenter->pNext->pPrev = pRenter;
		
			// Move in
			m_pBuckets[nPos].pPrev = NULL;
			m_pBuckets[nPos].pNext = NULL;
			m_pBuckets[nPos].pKey = pKey;
			m_pBuckets[nPos].pValue = pValue;
		}
		else
		{
			// The bucket is already owned, so just rent the first available empty spot.
			struct HashBucket* pNewBucket = m_pFirstEmpty;
			m_pFirstEmpty = pNewBucket->pNext;
			m_pFirstEmpty->pPrev = NULL;
			pNewBucket->pKey = pKey;
			pNewBucket->pValue = pValue;
			pNewBucket->pNext = m_pBuckets[nPos].pNext;
			pNewBucket->pPrev = &m_pBuckets[nPos];
			m_pBuckets[nPos].pNext = pNewBucket;
			if(pNewBucket->pNext)
				pNewBucket->pNext->pPrev = pNewBucket;
		}
	}
	else
	{
		// The bucket is empty.  Move in.
		if(m_pBuckets[nPos].pPrev)
			m_pBuckets[nPos].pPrev->pNext = m_pBuckets[nPos].pNext;
		else
		{
			GAssert(m_pFirstEmpty == &m_pBuckets[nPos], "Orphaned empty bucket!  Only m_pFirstEmpty and non-empty buckets should have a NULL value for pPrev.");
			m_pFirstEmpty = m_pBuckets[nPos].pNext;
		}
		if(m_pBuckets[nPos].pNext)
			m_pBuckets[nPos].pNext->pPrev = m_pBuckets[nPos].pPrev;
		m_pBuckets[nPos].pPrev = NULL;
		m_pBuckets[nPos].pNext = NULL;
		m_pBuckets[nPos].pKey = pKey;
		m_pBuckets[nPos].pValue = pValue;
	}
	GAssert(m_pFirstEmpty && m_pFirstEmpty->pNext, "Less than two empty slots left!");
}

bool GHashTableBase::_Get(const char* pKey, void** pOutValue)
{
	GAssert(pKey != NULL, "pKey can't be NULL");
	unsigned int nPos = Hash(pKey, m_nBucketCount);
	GAssert(nPos < (unsigned int)m_nBucketCount, "Out of range");
	if(!m_pBuckets[nPos].pKey || m_pBuckets[nPos].pPrev)
		return false;
	struct HashBucket* pBucket;
	for(pBucket = &m_pBuckets[nPos]; pBucket; pBucket = pBucket->pNext)
	{
		if(AreKeysEqual(pBucket->pKey, pKey))
		{
			*pOutValue = (void*)pBucket->pValue;
			return true;
		}
	}
	return false;
}

int GHashTableBase::_Count(const char* pKey)
{
	GAssert(pKey != NULL, "pKey can't be NULL");
	unsigned int nPos = Hash(pKey, m_nBucketCount);
	GAssert(nPos < (unsigned int)m_nBucketCount, "Out of range");
	if(!m_pBuckets[nPos].pKey || m_pBuckets[nPos].pPrev)
		return 0;
	int nCount = 0;
	struct HashBucket* pBucket;
	for(pBucket = &m_pBuckets[nPos]; pBucket; pBucket = pBucket->pNext)
	{
		if(AreKeysEqual(pBucket->pKey, pKey))
			nCount++;
	}
	return nCount;
}

void GHashTableBase::_Remove(const char* pKey)
{
	GAssert(pKey != NULL, "pKey can't be NULL");
	unsigned int nPos = Hash(pKey, m_nBucketCount);
	GAssert(nPos < (unsigned int)m_nBucketCount, "Out of range");
	if(!m_pBuckets[nPos].pKey || m_pBuckets[nPos].pPrev)
		return;
	struct HashBucket* pBucket;
	for(pBucket = &m_pBuckets[nPos]; pBucket; pBucket = pBucket->pNext)
	{
		GAssert(pBucket->pKey, "empty bucket should not be in a chain!");
		if(AreKeysEqual(pBucket->pKey, pKey))
		{
			if(pBucket->pPrev)
			{
				// It's just a renter, so unlink it and delete the bucket
				GAssert(pBucket != &m_pBuckets[nPos], "The landlord bucket shouldn't have a prev");
				pBucket->pPrev->pNext = pBucket->pNext;
				if(pBucket->pNext)
					pBucket->pNext->pPrev = pBucket->pPrev;
				pBucket->pPrev = NULL;
				pBucket->pNext = m_pFirstEmpty;
				pBucket->pKey = NULL;
				m_pFirstEmpty->pPrev = pBucket;
				m_pFirstEmpty = pBucket;
			}
			else
			{
				// It's a landlord
				GAssert(pBucket == &m_pBuckets[nPos], "Renters should have a prev");
				if(pBucket->pNext)
				{
					// Move the next renter into the landlord bucket
					struct HashBucket* pOldBucket = pBucket->pNext;
					pBucket->pNext = pOldBucket->pNext;
					pBucket->pKey = pOldBucket->pKey;
					pBucket->pValue = pOldBucket->pValue;
					if(pBucket->pNext)
						pBucket->pNext->pPrev = pBucket;

					// Delete the former-renter's old bucket
					pOldBucket->pNext = m_pFirstEmpty;
					pOldBucket->pPrev = NULL;
					pOldBucket->pKey = NULL;
					m_pFirstEmpty->pPrev = pOldBucket;
					m_pFirstEmpty = pOldBucket;
				}
				else
				{
					// Just delete the landlord bucket
					pBucket->pNext = m_pFirstEmpty;
					pBucket->pKey = NULL;
					m_pFirstEmpty->pPrev = pBucket;
					m_pFirstEmpty = pBucket;
				}
			}
			m_nCount--;
			m_nModCount++;
			return;
		}
	}
}

// ------------------------------------------------------------------------------

const char* GHashTableEnumerator::GetNext(void** ppValue)
{
	GAssert(m_pHashTable->GetModCount() == m_nModCount, "The HashTable was modified since this enumerator was constructed!");
	const void* pValue;
	while(m_nPos < m_pHashTable->m_nBucketCount)
	{
		const char* pKey = m_pHashTable->m_pBuckets[m_nPos].pKey;
		pValue = m_pHashTable->m_pBuckets[m_nPos].pValue;
		m_nPos++;
		if(pKey)
		{
			*ppValue = (void*)pValue;
			return pKey;
		}
	}
	return NULL;
}

void* GHashTableEnumerator::GetCurrentValue()
{
	if(m_nPos <= 0)
		return NULL;
	return (void*)m_pHashTable->m_pBuckets[m_nPos - 1].pValue;
}


// ------------------------------------------------------------------------------

char* GStringHeap::Add(GQueue* pQ)
{
	int nLen = pQ->GetSize();
	char* pNewString = Allocate(nLen + 1);
	pQ->DumpToExistingBuffer(pNewString);
	pNewString[nLen] = '\0';
	return pNewString;
}
