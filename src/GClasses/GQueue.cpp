/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GQueue.h"
#include "GMacros.h"
#include <cassert>

GQueue::GQueue(int nChunkSize)
{
	GAssert(nChunkSize > 4, "too small");
#ifdef WIN32
	GAssert(sizeof(wchar_t) == 2, "wrong size");
#else
	GAssert(sizeof(wchar_t) == 4, "wrong size");
#endif // !WIN32
	m_nDefaultChunkSize = nChunkSize;
	m_nInPos = nChunkSize;
	m_nOutPos = 0;
	m_pFirstChunk = NULL;
	m_pLastChunk = NULL;
	m_nDataSize = 0;
	m_pExtra = NULL;
}

GQueue::~GQueue()
{
	while(m_pFirstChunk)
		delete(UnlinkFirst());
	delete(m_pExtra);
}

void GQueue::Link(GQueueChunk* pChunk)
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

GQueueChunk* GQueue::UnlinkFirst()
{
	GQueueChunk* pChunk = m_pFirstChunk;
	m_pFirstChunk = pChunk->m_pNext;
	if(!m_pFirstChunk)
		m_pLastChunk = NULL;
	pChunk->m_pNext = NULL;
	return pChunk;
}

void GQueue::DropFirstChunk()
{
	if(m_pExtra)
		delete(UnlinkFirst());
	else
		m_pExtra = UnlinkFirst();
}

void GQueue::Flush()
{
	while(m_pFirstChunk)
		DropFirstChunk();
	m_nInPos = 0;
	m_nOutPos = 0;
	m_nDataSize = 0;
}

void GQueue::Push(char c)
{
	GQueueChunk* pCurrentChunk = m_pLastChunk;
	if(!pCurrentChunk || m_nInPos >= pCurrentChunk->m_nSize)
	{
		if(m_pExtra && m_pExtra->m_bDelete)
		{
			pCurrentChunk = m_pExtra;
			m_pExtra = NULL;
		}
		else
			pCurrentChunk = new GQueueChunk(m_nDefaultChunkSize, NULL);
		Link(pCurrentChunk);
		m_nInPos = 0;
	}
	pCurrentChunk->m_pData[m_nInPos] = c;
	m_nInPos++;
	m_nDataSize++;
}

void GQueue::PushStatic(const unsigned char* pBuf, int nBufSize)
{
	GQueueChunk* pCurrentChunk = m_pLastChunk;
	if(pCurrentChunk)
		pCurrentChunk->m_nSize = m_nInPos;
	if(m_pExtra && !m_pExtra->m_bDelete)
	{
		pCurrentChunk = m_pExtra;
		m_pExtra = NULL;
		pCurrentChunk->m_pData = (char*)pBuf;
		pCurrentChunk->m_nSize = nBufSize;
	}
	else
		pCurrentChunk = new GQueueChunk(nBufSize, (const char*)pBuf);
	Link(pCurrentChunk);
	m_nInPos = nBufSize;
	m_nDataSize += nBufSize;
}

void GQueue::Push(int n)
{
	Push(((char*)&n)[0]);
	Push(((char*)&n)[1]);
	Push(((char*)&n)[2]);
	Push(((char*)&n)[3]);
}

void GQueue::Push(wchar_t wc)
{
#ifdef WIN32	
	Push(((char*)&wc)[0]);
	Push(((char*)&wc)[1]);
#else
	Push(((char*)&wc)[0]);
	Push(((char*)&wc)[1]);
	Push(((char*)&wc)[2]);
	Push(((char*)&wc)[3]);
#endif // !WIN32
}

void GQueue::Push(const char* szString)
{
	while(*szString != '\0')
		Push(*szString++);
}

void GQueue::Push(const unsigned char* pBuf, int nBufSize)
{
	int n;
	for(n = 0; n < nBufSize; n++)
		Push((char)pBuf[n]);
}

void GQueue::Push(float f)
{
	int n;
	for(n = 0; n < sizeof(float); n++)
		Push(((char*)&f)[n]);
}

bool GQueue::Pop(float* pf)
{
	int n;
	for(n = 0; n < sizeof(float); n++)
	{
		if(!Pop(&((char*)pf)[n]))
			return false;
	}
	return true;
}

void GQueue::Push(double d)
{
	int n;
	for(n = 0; n < sizeof(double); n++)
		Push(((char*)&d)[n]);
}

bool GQueue::Pop(double* pd)
{
	int n;
	for(n = 0; n < sizeof(double); n++)
	{
		if(!Pop(&((char*)pd)[n]))
			return false;
	}
	return true;
}

bool GQueue::Pop(char* pc)
{
	GQueueChunk* pCurrentChunk = m_pFirstChunk;
	if(!pCurrentChunk)
	{
		GAssert(m_nDataSize == 0, "inconsistent size");
		return false;
	}
	*pc = pCurrentChunk->m_pData[m_nOutPos];
	m_nOutPos++;
	m_nDataSize--;
	if(m_nOutPos >= pCurrentChunk->m_nSize)
	{
		DropFirstChunk();
		m_nOutPos = 0;
	}
	return true;
}

bool GQueue::Pop(int* pn)
{
	Pop((char*)pn);
	Pop(((char*)pn) + 1);
	Pop(((char*)pn) + 2);
	return Pop(((char*)pn) + 3);
}

bool GQueue::Pop(wchar_t* pwc)
{
#ifdef WIN32
	Pop((char*)pwc);
	return Pop(((char*)pwc) + 1);
#else
	Pop((char*)pwc);
	Pop(((char*)pwc) + 1);
	Pop(((char*)pwc) + 2);
	return Pop(((char*)pwc) + 3);
#endif // !WIN32
}

char* GQueue::DumpToString()
{
	int nSize = m_nDataSize;
	char* szStr = new char[nSize + 1];
	DumpToExistingBuffer(szStr);
	szStr[nSize] = '\0';
	return szStr;
}

void GQueue::SetFirstInt(int n)
{
	char* pData = m_pLastChunk->m_pData;
	pData[0] = ((char*)&n)[0];
	pData[1] = ((char*)&n)[1];
	pData[2] = ((char*)&n)[2];
	pData[3] = ((char*)&n)[3];
}

void GQueue::DumpToExistingBuffer(char* pBuf)
{
	while(m_nDataSize > 0)
	{
		Pop(pBuf);
		pBuf++;
	}
}
