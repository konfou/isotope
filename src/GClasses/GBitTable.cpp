/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GBitTable.h"
#include <string.h> // (for memset)

GBitTable::GBitTable(unsigned int nBitCount)
{
	m_nBitCount = nBitCount;
	unsigned int nByteSize = (nBitCount + 7) >> 3;
	m_pData = new unsigned char[nByteSize];
	memset(m_pData, '\0', nByteSize);
}

GBitTable::~GBitTable()
{
	delete(m_pData);
}

void GBitTable::ClearAll()
{
	unsigned int nByteSize = (m_nBitCount + 7) >> 3;
	memset(m_pData, '\0', nByteSize);
}

void GBitTable::SetAll()
{
	unsigned int nByteSize = (m_nBitCount + 7) >> 3;
	memset(m_pData, 255, nByteSize);
}

bool GBitTable::GetBit(unsigned int nIndex)
{
	return((m_pData[nIndex >> 3] & (1 << (nIndex & 7))) ? true : false);
}

void GBitTable::SetBit(unsigned int nIndex, bool bValue)
{
	if(bValue)
		m_pData[nIndex >> 3] |= (1 << (nIndex & 7));
	else
		m_pData[nIndex >> 3] &= ((1 << (nIndex & 7)) ^ 0xff);
}

void GBitTable::ToggleBit(unsigned int nIndex)
{
	m_pData[nIndex >> 3] ^= (1 << (nIndex & 7));
}
