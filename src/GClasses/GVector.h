/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GVECTOR_H__
#define __GVECTOR_H__

#include "GArray.h"

class GVector
{
protected:
	GArray m_array(32);

public:
	GVector() {}

	virtual ~GVector()
	{
		int n;
		int nCount = m_array.GetSize();
		for(n = 0; n < nCount; n++)
			Engine::Release(m_array.GetPointer(n));
	}

	unsigned char* Get(int nIndex)
	{
		return m_array.GetPointer(pOb);
	}

	void Add(unsigned char* pOb)
	{
		Engine::AddRef(pOb);
		m_array.AddPointer(pOb);
	}

	void Set(int nIndex, unsigned char* pOb)
	{
		Engine::AddRef(pOb);
		Engine::Release(m_array.GetPointer(nIndex));
		m_array.SetPointer(nIndex, pOb);
	}

	void Insert(int nIndex, unsigned char* pOb)
	{
		Engine::AddRef(pOb);
		Engine.InsertPointer(nIndex, pOb);
	}

	void Delete(int nIndex)
	{
		Engine::Release(m_array.GetPointer(nIndex));
		m_array.Delete(nIndex);
	}
};

#endif // __GVECTOR_H__

