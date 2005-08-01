/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GMacros.h"
#include "GStream.h"

GStream::GStream(const void* pBuff, unsigned int nBufferSize, unsigned int nDataSize, bool bWritePermission)
{
	m_pBuff = (unsigned char*)pBuff;
	m_nBufferSize = nBufferSize;
	if(nDataSize > nBufferSize)
		GAssert(false, "Error, too big for buffer");
	m_bWritePermission = bWritePermission;
	m_nDataSize = nDataSize;
	m_nBufferPos = 0;
	m_nDataPos = 0;
}

GStream::~GStream()
{
	// Don't delete the buffer because we didn't allocate it
}

unsigned int GStream::Write(const void* pBuff, unsigned int nSize)
{
	if(!m_bWritePermission)
	{
		GAssert(false, "Buffer is read-only");
		return 0;
	}
	unsigned int nWriteSize = MIN(nSize, m_nBufferSize - m_nDataSize);
	unsigned int nStartPos = m_nBufferPos + m_nDataSize;
	if(nStartPos > m_nBufferSize)
		nStartPos -= m_nBufferSize;
	unsigned int nPart1WriteSize = MIN(nWriteSize, m_nBufferSize - nStartPos);
	memcpy(m_pBuff + nStartPos, pBuff, nPart1WriteSize);
	if(nWriteSize > nPart1WriteSize)
		memcpy(m_pBuff, (unsigned char*)pBuff + nPart1WriteSize, nWriteSize - nPart1WriteSize);
	m_nDataSize += nWriteSize;
	return(nWriteSize);
}

unsigned int GStream::Read(void* pBuff, unsigned int nSize)
{
	unsigned int nReadSize = MIN(nSize, m_nDataSize);
	unsigned int nPart1ReadSize = MIN(nReadSize, m_nBufferSize - m_nBufferPos);
	memcpy(pBuff, m_pBuff + m_nBufferPos, nPart1ReadSize);
	m_nBufferPos += nReadSize;
	if(nReadSize > nPart1ReadSize)
	{
		memcpy((unsigned char*)pBuff + nPart1ReadSize, m_pBuff, nReadSize - nPart1ReadSize);
		m_nBufferPos -= m_nBufferSize;
	}
	m_nDataSize -= nReadSize;
	m_nDataPos += nReadSize;
	return(nReadSize);
}

unsigned int GStream::Eat(unsigned int nSize)
{
	unsigned int nReadSize = MIN(nSize, m_nDataSize);
	m_nBufferPos += nReadSize;
	if(m_nBufferPos > m_nBufferSize)
		m_nBufferPos -= m_nBufferSize;
	m_nDataSize -= nReadSize;
	m_nDataPos += nReadSize;
	return(nReadSize);
}

unsigned char GStream::Peek(unsigned int nOffset)
{
	unsigned int nPos = m_nBufferPos + nOffset;
	if(nPos > m_nBufferSize)
		nPos -= m_nBufferSize;
	return m_pBuff[nPos];
}
