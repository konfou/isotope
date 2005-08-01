/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GStack.h"
#include "GLList.h"
#include <stdio.h>

class GStackChunk : public GBucket
{
public:
	char* m_pData;

	GStackChunk(int nChunkSize) : GBucket() { m_pData = new char[nChunkSize]; }
	virtual ~GStackChunk() { delete(m_pData); }

	virtual int Compare(GBucket* pBucket) { return -1; }
};

// --------------------------------------------------------------------

GStack::GStack(int nChunkSize)
{
	m_nChunkSize = nChunkSize;
	m_nPos = nChunkSize;
	m_pChunks = new GLList();
	m_nDataSize = 0;
	m_pExtra = NULL;
}

GStack::~GStack()
{
	delete(m_pChunks);
	delete(m_pExtra);
}

void GStack::Flush()
{
	m_pChunks->Clear();
	m_nPos = m_nChunkSize;
	m_nDataSize = 0;
}

void GStack::Push(char c)
{
	if(m_nPos >= m_nChunkSize)
	{
		GStackChunk* pNewChunk = m_pExtra;
		if(!pNewChunk)
			pNewChunk = new GStackChunk(m_nChunkSize);
		m_pChunks->Insert(NULL, pNewChunk);
		m_pExtra = NULL;
		m_nPos = 0;
	}
	((GStackChunk*)m_pChunks->GetFirst())->m_pData[m_nPos] = c;
	m_nPos++;
	m_nDataSize++;
}

void GStack::Push(int n)
{
	Push(((char*)&n)[0]);
	Push(((char*)&n)[1]);
	Push(((char*)&n)[2]);
	Push(((char*)&n)[3]);
}

bool GStack::Pop(char* pc)
{
	if(m_nDataSize < 1)
		return false;
	m_nPos--;
	m_nDataSize--;
	*pc = ((GStackChunk*)m_pChunks->GetFirst())->m_pData[m_nPos];
	if(m_nPos <= 0)
	{
		delete(m_pExtra);
		m_pExtra = (GStackChunk*)m_pChunks->Unlink(NULL);
		m_nPos = m_nChunkSize;
	}
	return true;
}

bool GStack::Pop(int* pn)
{
	Pop(((char*)pn) + 3);
	Pop(((char*)pn) + 2);
	Pop(((char*)pn) + 1);
	return Pop((char*)pn);
}
