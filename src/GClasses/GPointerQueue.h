/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GPointerQueue_H__
#define __GPointerQueue_H__

#include "GMacros.h"

class GPointerQueueChunk
{
public:
	GPointerQueueChunk* m_pNext;
	void** m_pData;

	GPointerQueueChunk(int nChunkSize);
	~GPointerQueueChunk();
};

// Represents a FIFO queue of pointers
class GPointerQueue
{
protected:
	int m_nChunkSize;
	int m_nInPos;
	int m_nOutPos;
	int m_nDataSize;
	GPointerQueueChunk* m_pFirstChunk;
	GPointerQueueChunk* m_pLastChunk;
	GPointerQueueChunk* m_pExtra;

public:
	GPointerQueue(int nChunkSize = 1024);
	virtual ~GPointerQueue();

	void Flush();

	// Add a pointer to the queue
	inline void Push(void* pointer)
	{
		GAssert(pointer != (void*)0xfeeefeee, "This doesn't look right--todo: remove this check");
		if(m_nInPos >= m_nChunkSize)
			GetNewChunk();
		m_pLastChunk->m_pData[m_nInPos] = pointer;
		m_nInPos++;
		m_nDataSize++;
	}

	// Read the next pointer from the queue
	inline void* Pop()
	{
		GAssert(m_nDataSize > 0, "The queue is empty");
		void* pPointer = m_pFirstChunk->m_pData[m_nOutPos];
		m_nOutPos++;
		m_nDataSize--;
		if(m_nOutPos >= m_nChunkSize)
			ThrowOutChunk();
		return pPointer;
	}

	// Peek at the next pointer, but don't actually pop it
	inline void* Peek()
	{
		GAssert(m_nDataSize > 0, "The queue is empty");
		return m_pFirstChunk->m_pData[m_nOutPos];
	}

	// Returns how many pointers are in the queue
	inline int GetSize() { return m_nDataSize; }

protected:
	void GetNewChunk();
	void ThrowOutChunk();
	void Link(GPointerQueueChunk* pChunk);
	GPointerQueueChunk* UnlinkFirst();
};

#endif // __GPointerQueue_H__
