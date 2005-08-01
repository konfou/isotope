/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GLList.h"
#include "GMacros.h"
#include <stdio.h>

GBucket::GBucket()
{
	m_pNext = NULL;
}

GBucket::~GBucket()
{
	GBucket* pCurrent;
	GBucket* pNext;
	for(pCurrent = m_pNext; pCurrent; pCurrent = pNext)
	{
		pNext = pCurrent->m_pNext;
		pCurrent->m_pNext = NULL;
		delete(pCurrent);
	}
}

void GBucket::SetNext(GBucket* pNext)
{
	m_pNext = pNext;
}

// -------------------------------------------------------------------

GLList::GLList()
{
	m_pFirstBucket = NULL;
	m_pLastBucket = NULL;
	m_nBucketCount = 0;
}

GLList::~GLList()
{
	Clear();
}

void GLList::Clear()
{
	// delete the buckets (not recursively cuz it may be a huge list)
	GBucket* pBucket;
	while(m_pFirstBucket)
	{
		pBucket = m_pFirstBucket;
		m_pFirstBucket = pBucket->GetNext();
		pBucket->SetNext(NULL);
		delete(pBucket);
	}
	m_pFirstBucket = NULL;
	m_pLastBucket = NULL;
	m_nBucketCount = 0;
}

GBucket* GLList::GetBucket(GBucket* pLikeMe)
{
	GBucket* pBucket;
	for(pBucket = m_pFirstBucket; pBucket; pBucket = pBucket->GetNext())
	{
		if(pBucket->Compare(pLikeMe) == 0)
			return(pBucket);
	}
	return(NULL);
}

GBucket* GLList::GetBucket(int n)
{
	GBucket* pBucket = m_pFirstBucket;
	while(pBucket && n > 0)
	{
		pBucket = pBucket->GetNext();
		n--;
	}
	return pBucket;
}

void GLList::Link(GBucket* pNewBucket)
{
	GAssert(!pNewBucket->GetNext(), "That bucket is already in a list");
	GAssert(pNewBucket != m_pFirstBucket && pNewBucket != m_pLastBucket, "That bucket is already in the list");
	if(m_pLastBucket)
	{
		GAssert(m_pFirstBucket, "last but no first");
		GAssert(m_pLastBucket->GetNext() == NULL, "the end is not the end");
		m_pLastBucket->SetNext(pNewBucket);
	}
	else
	{
		GAssert(!m_pFirstBucket, "list out of whack");
		m_pFirstBucket = pNewBucket;
	}
	m_pLastBucket = pNewBucket;
	m_nBucketCount++;
}

void GLList::LinkSorted(GBucket* pBucket)
{
	GAssert(pBucket, "bad parameter");
	GBucket* pPrev = NULL;
	GBucket* pTmp;
	for(pTmp = m_pFirstBucket; pTmp != NULL; pTmp = pTmp->GetNext())
	{
		if(pBucket->Compare(pTmp) < 0)
			break;
		pPrev = pTmp;
	}
	if(pPrev == NULL)
	{
		if(m_pFirstBucket == NULL)
		{
			GAssert(pBucket->GetNext() == NULL, "tsnh");
			GAssert(!m_pLastBucket, "tsnh");
			m_pLastBucket = pBucket;
		}
		else
			pBucket->SetNext(m_pFirstBucket);
		m_pFirstBucket = pBucket;
	}
	else
	{
		if(pPrev == m_pLastBucket)
			m_pLastBucket = pBucket;
		else
			GAssert(pTmp, "tsnh");
		pBucket->SetNext(pTmp);
		pPrev->SetNext(pBucket);
	}
	m_nBucketCount++;
}

void GLList::SlowSort()
{
	GLList tmp;
	GBucket* pBucket;
	while(m_pFirstBucket)
	{
		pBucket = m_pFirstBucket;
		m_pFirstBucket = pBucket->GetNext();
		pBucket->SetNext(NULL);
		tmp.LinkSorted(pBucket);
	}
	m_pFirstBucket = tmp.m_pFirstBucket;
	m_pLastBucket = tmp.m_pLastBucket;
	tmp.m_pFirstBucket = NULL;
	tmp.m_pLastBucket = NULL;
}

void GLList::Split(GLList* pList2)
{
	int nHalf = m_nBucketCount / 2;
	int n = 0;
	GBucket* pBucket;
	for(pBucket = m_pFirstBucket; pBucket != NULL; pBucket = pBucket->GetNext())
	{
		n++;
		if(n >= nHalf)
			break;
	}
	GAssert(pBucket, "tsnh");
	pList2->m_pFirstBucket = pBucket->GetNext();
	pList2->m_pLastBucket = m_pLastBucket;
	pList2->m_nBucketCount = m_nBucketCount - nHalf;
	pBucket->SetNext(NULL);
	m_pLastBucket = pBucket;
	m_nBucketCount = nHalf;
}

// If pPrev == NULL then it inserts at the front of the list
void GLList::Insert(GBucket* pPrev, GBucket* pThis)
{
	if(pPrev)
	{
		pThis->SetNext(pPrev->GetNext());
		pPrev->SetNext(pThis);
	}
	else
	{
		pThis->SetNext(m_pFirstBucket);
		m_pFirstBucket = pThis;
	}
	if(!pThis->GetNext())
		m_pLastBucket = pThis;
	m_nBucketCount++;
}

// It is your job to delete it after this cuts it out of the list
GBucket* GLList::Unlink(GBucket* pPrev)
{
	GBucket* pTmp;
	if(pPrev)
	{
		pTmp = pPrev->GetNext();
		if(pTmp)
		{
			pPrev->SetNext(pTmp->GetNext());
			pTmp->SetNext(NULL);
			if(pTmp == m_pLastBucket)
				m_pLastBucket = pPrev;
			m_nBucketCount--;
		}
	}
	else
	{
		pTmp = m_pFirstBucket;
		if(pTmp)
		{
			m_pFirstBucket = pTmp->GetNext();
			pTmp->SetNext(NULL);
			if(pTmp == m_pLastBucket)
				m_pLastBucket = pPrev;
			m_nBucketCount--;
		}
	}
	return(pTmp);
}

void GLList::DeleteBucket(GBucket* pPrev)
{
	delete(Unlink(pPrev));
}

void GLList::Merge(GLList* pList2)
{
	GBucket* pPrev = NULL;
	GBucket* pThis;
	GBucket* pNext = GetFirst();
	while(pList2->m_nBucketCount)
	{
		pThis = pList2->Unlink(NULL);
		while(true)
		{
			if(!pNext)
				break;
			if(pNext->Compare(pThis) > 0)
				break;
			pPrev = pNext;
			pNext = pPrev->GetNext();
		}
		Insert(pPrev, pThis);
		pPrev = pThis;
	}
}

// This does merge-sort
void GLList::Sort()
{
	if(m_nBucketCount < 2)
		return;
	GLList* pTmpList = new GLList();
	Split(pTmpList);
	Sort();
	pTmpList->Sort();
	Merge(pTmpList);
	delete(pTmpList);
}

