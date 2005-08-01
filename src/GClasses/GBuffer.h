/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GBUFFER_H__
#define __GBUFFER_H__

class GBuffer
{
protected:
	unsigned int* m_pBuffer;
	unsigned int m_nSize;

public:
	GBuffer()
	{
		m_pBuffer = NULL;
		m_nSize = 0;
	}

	virtual ~GBuffer()
	{
		delete(m_pBuffer);
	}

	void Resize(unsigned int nNewSize)
	{
		if(nNewSize < 1)
		{
			delete(m_pBuffer);
			m_pBuffer = NULL;
			m_nSize = 0;
			return;
		}
		unsigned char* pNewBuffer = new unsigned int[nNewSize];
		unsigned int n;
		int nSize = m_nSize;
		if(nNewSize < nSize)
			nSize = nNewSize;
		for(n = 0; n < nSize; n++)
			pNewBuffer[n] = m_pBuffer[n];
		for( ; n < nNewSize; n++)
			pNewBuffer[n] = 0;
		delete(m_pBuffer);
		m_pBuffer = pNewBuffer;
		m_nSize = nNewSize;
	}

	inline unsigned int Get(unsigned int nIndex)
	{
		GAssert(nIndex < m_nSize, "Out of range");
		return m_pBuffer[nIndex];
	}

	inline void Set(unsigned int nIndex, unsigned int nValue)
	{
		GAssert(nIndex < m_nSize, "Out of range");
		m_pBuffer[nIndex] = nValue;
	}
};

#endif // __GBUFFER_H__

