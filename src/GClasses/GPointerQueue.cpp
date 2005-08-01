/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GPointerQueue.h"
#include "GMacros.h"

GPointerQueueChunk::GPointerQueueChunk(int nChunkSize)
{
	GAssert(nChunkSize >= 0 && nChunkSize < 33554432, "looks like a bad chunk size");
	m_pNext = NULL;
	m_pData = new void*[nChunkSize];
	if(!m_pData)
		throw "allocation failed";
}

GPointerQueueChunk::~GPointerQueueChunk()
{
	GAssert(!m_pNext, "still linked into the list!");
	delete(m_pData);
}

// ------------------------------------------------------------------------------

GPointerQueue::GPointerQueue(int nChunkSize)
{
	GAssert(nChunkSize > 0, "must be > 0");
	m_nChunkSize = nChunkSize;
	m_nInPos = nChunkSize;
	m_nOutPos = 0;
	m_nDataSize = 0;
	m_pFirstChunk = NULL;
	m_pLastChunk = NULL;
	m_pExtra = NULL;
}

GPointerQueue::~GPointerQueue()
{
	while(m_pFirstChunk)
		delete(UnlinkFirst());
	delete(m_pExtra);
}

void GPointerQueue::Link(GPointerQueueChunk* pChunk)
{
	GAssert(!pChunk->m_pNext, "Already linked");
	if(m_pLastChunk)
	{
		GAssert(!m_pLastChunk->m_pNext, "not really the last one");
		m_pLastChunk->m_pNext = pChunk;
		m_pLastChunk = pChunk;
	}
	else
	{
		GAssert(!m_pFirstChunk, "Linked list messed up");
		m_pFirstChunk = pChunk;
		m_pLastChunk = pChunk;
	}
}

GPointerQueueChunk* GPointerQueue::UnlinkFirst()
{
	GPointerQueueChunk* pChunk = m_pFirstChunk;
	m_pFirstChunk = pChunk->m_pNext;
	if(!m_pFirstChunk)
		m_pLastChunk = NULL;
	pChunk->m_pNext = NULL;
	return pChunk;
}

void GPointerQueue::ThrowOutChunk()
{
	if(m_pExtra)
		delete(UnlinkFirst());
	else
		m_pExtra = UnlinkFirst();
	m_nOutPos = 0;
}

void GPointerQueue::Flush()
{
	while(m_pFirstChunk)
		ThrowOutChunk();
	m_nInPos = m_nChunkSize;
	m_nOutPos = 0;
	m_nDataSize = 0;
}

void GPointerQueue::GetNewChunk()
{
	GPointerQueueChunk* pNewChunk = m_pExtra;
	if(!pNewChunk)
		pNewChunk = new GPointerQueueChunk(m_nChunkSize);
	Link(pNewChunk);
	m_pExtra = NULL;
	m_nInPos = 0;
}


