/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GLLIST_H__
#define __GLLIST_H__

// GBuckets are used by linked lists.
class GBucket
{
protected:
	GBucket* m_pNext;

public:
	GBucket();

	// This deletes the entire bucket-chain.  (It doesn't recurse, so any chain-length is safe.)
	virtual ~GBucket();

	inline GBucket* GetNext()	{ return m_pNext; }

	void SetNext(GBucket* pNext);

	// To use a Linked list, make your data class inherrit from
	// GBucket and override the Compare method.
	// -1 = this is Less than pBucket
	// 0  = this is Equal to pBucket
	// 1  = this is Greater than pBucket
	virtual int Compare(GBucket* pBucket) = 0;
};

// This does linked lists
class GLList
{
protected:
	GBucket* m_pFirstBucket;
	GBucket* m_pLastBucket;
	int m_nBucketCount;

public:
	GLList();
	virtual ~GLList();

	// Deletes all the buckets in the list
	void Clear();

	// Add a bucket to the end of the list
	void Link(GBucket* pBucket);

	// Insert the bucket into the list in order
	void LinkSorted(GBucket* pBucket);

	// Merge Sort
	void Sort();

	// In case you want it to take a long time to sort
	void SlowSort();

	// Returns the first bucket in the list
	inline GBucket* GetFirst()	{	return(m_pFirstBucket);	}

	// Returns the last bucket in the list
	inline GBucket* GetLast()	{	return(m_pLastBucket);	}

	// Returns the bucket that follows pCurrentBucket
	inline GBucket* GetNext(GBucket* pCurrentBucket)
	{
		return pCurrentBucket ? pCurrentBucket->GetNext() : m_pFirstBucket;
	}

	// This finds the bucket that compares to 0 with pLikeMe (or NULL if not found)
	GBucket* GetBucket(GBucket* pLikeMe);

	// Get the nth (zero-based) bucket 
	GBucket* GetBucket(int n);

	// How many buckets are in this list?
	int GetCount() { return m_nBucketCount; }

	// This cuts out the bucket following pPrev from the list.  Use NULL to get the first one
	// It is your job to delete this bucket
	GBucket* Unlink(GBucket* pPrev);

	// This calls Unlink, and deletes the bucket pointer after it is done
	void DeleteBucket(GBucket* pPrev);

	// Insert pThis after pPrev in the list.  (If pPrev is NULL, it inserts at the fromt of the list)
	void Insert(GBucket* pPrev, GBucket* pThis);

protected:
	void Split(GLList* pList2);
	void Merge(GLList* pList2);
};

#endif // __GLLIST_H__
