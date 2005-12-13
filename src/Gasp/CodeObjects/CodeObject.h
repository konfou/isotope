/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __CODEOBJECT_H__
#define __CODEOBJECT_H__

#include "../Engine/Error.h"
class GHashTable;

class CodeObject
{
public:
#ifdef _DEBUG
	static unsigned int s_nAllocs;
	static unsigned int s_nDeletes;
//	static GHashTable* s_pLeakedObjects;
#endif // _DEBUG

protected:
	int m_nLineNumber;
	unsigned int m_nColumnAndWidth;

public:
	CodeObject(int nLineNumber, int nColumn, int nWidth);
	virtual ~CodeObject();

	void SetLineNumber(int nLineNumber) { m_nLineNumber = nLineNumber; }
	int GetLineNumber() { return m_nLineNumber; }

	void SetColumnAndWidth(int nColumn, int nWidth)
	{
		m_nColumnAndWidth = ((nColumn & 0xffff)) << 16 | (nWidth & 0xffff);
	}

	void GetColumnAndWidth(int* pnColumn, int* pnWidth)
	{
		*pnWidth = m_nColumnAndWidth & 0xffff;
		*pnColumn = ((m_nColumnAndWidth >> 16) & 0xffff);
	}


};

#endif // __CODEOBJECT_H__
