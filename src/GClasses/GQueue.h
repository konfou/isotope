/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GQUEUE_H__
#define __GQUEUE_H__

#include <string.h>
#include "GMacros.h"

class GQueueChunk
{
public:
	GQueueChunk* m_pNext;
	int m_nSize;
	char* m_pData;
	bool m_bDelete;

	GQueueChunk(int nSize, const char* pStaticData)
	{
		m_pNext = NULL;
		m_nSize = nSize;
		if(pStaticData)
		{
			m_pData = (char*)pStaticData;
			m_bDelete = false;
		}
		else
		{
			m_pData = new char[nSize];
			m_bDelete = true;
		}
	}

	~GQueueChunk()
	{
		GAssert(!m_pNext, "Still linked into the list!");
		if(m_bDelete)
			delete(m_pData);
	}
};



class GQueue
{
protected:
	int m_nDefaultChunkSize;
	int m_nInPos;
	int m_nOutPos;
	int m_nDataSize;
	GQueueChunk* m_pFirstChunk;
	GQueueChunk* m_pLastChunk;
	GQueueChunk* m_pExtra;
	GQueueChunk* m_pExtraStatic;

public:
	GQueue(int nChunkSize = 1024);
	virtual ~GQueue();

	void Flush();

	void Push(wchar_t wc);
	void Push(char c);
	void Push(int n);
	void Push(const char* szString);
	void Push(const unsigned char* pBuf, int nBufSize);
	void PushStatic(const unsigned char* pBuf, int nBufSize);
	inline void Push(void* pointer) { Push((int)pointer); }
	inline void Push(unsigned int n) { Push((int)n); }
	inline void Push(unsigned char uc) { Push((char)uc); }
	void Push(float f);
	void Push(double d);

	bool Pop(wchar_t* pwc);
	bool Pop(char* pc);
	bool Pop(int* pn);
	inline bool Pop(void** pointer) { return Pop((int*)pointer); }
	inline bool Pop(unsigned int* pui) { return Pop((int*)pui); }
	inline bool Pop(unsigned char* puc) { return Pop((char*)puc); }
	bool Pop(float* pf);
	bool Pop(double* pd);
	
	int GetSize() { return m_nDataSize; }
	char* DumpToString(); // note: you must delete the string this returns
	void DumpToExistingBuffer(char* pBuf);

	// This is a hack to set the first int in the queue
	void SetFirstInt(int n);

protected:
	void Link(GQueueChunk* pChunk);
	GQueueChunk* UnlinkFirst();
	void DropFirstChunk();
};

#endif // __GQUEUE_H__
