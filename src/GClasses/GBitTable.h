/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GBITTABLE_H__
#define __GBITTABLE_H__

class GBitTable
{
protected:
	unsigned int m_nBitCount;
	unsigned char* m_pData;

public:
	GBitTable(unsigned int nBitCount);
	virtual ~GBitTable();

	// Sets all bits to false
	void ClearAll();

	// Sets all bits to true
	void SetAll();

	// Returns the bit at nIndex
	bool GetBit(unsigned int nIndex);

	// Sets the bit at nIndex
	void SetBit(unsigned int nIndex, bool bValue);

	// Toggles the bit at nIndex
	void ToggleBit(unsigned int nIndex);
};

#endif // __GBITTABLE_H__
