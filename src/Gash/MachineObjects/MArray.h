/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __MARRAY_H__
#define __MARRAY_H__

#include "../Include/GashEngine.h"
#include "../../GClasses/GArray.h"
#include "../../GClasses/GMacros.h"
#include <wchar.h>

class MArray : public WrapperObject
{
public:
	GPointerArray* m_pArray; // todo: pull the whole array class into this one
	Engine* m_pEngine;

	MArray(Engine* pEngine, int nGrowBy)
		: WrapperObject(pEngine, "Array")
	{
		m_pArray = new GPointerArray(nGrowBy);
		m_pEngine = pEngine;
	}

public:
	virtual ~MArray()
	{
		int nCount = m_pArray->GetSize();
		int n;
		GObject* pOb;
		for(n = 0; n < nCount; n++)
		{
			pOb = (GObject*)m_pArray->GetPointer(n);
			m_pEngine->UnpinObject(pOb);
		}
		delete(m_pArray);
	}

	void toStream(Engine* pEngine, EVar* pOutBlob, EVar* pOutRefs);
	void fromStream(Engine* pEngine, EVar* pStream);
	void setRefs(Engine* pEngine, EVar* pRefs);

	virtual void GetDisplayValue(wchar_t* pBuf, int nSize)
	{
		GAssert(nSize > 32, "Buffer too small");
		swprintf(pBuf, 32, L"Array of size %d", m_pArray->GetSize());
	}

	void allocate(Engine* pEngine, EVar* pSize)
	{
		pEngine->SetThis(new MArray(pEngine, pSize->pIntObject->m_value));
	}

	void getSize(Engine* pEngine, EVar* pOutSize)
	{
		pOutSize->pIntObject->m_value = m_pArray->GetSize();
	}

	void add(Engine* pEngine, EVar* pObj)
	{
		pEngine->PinObject(pObj->pOb);
		m_pArray->AddPointer(pObj->pOb);
	}

	void get(Engine* pEngine, EVar* pOutObj, EVar* pIndex)
	{
		GObject* pOb = (GObject*)m_pArray->GetPointer(pIndex->pIntObject->m_value);
		pEngine->SetVar(pOutObj, pOb);
	}

	void set(Engine* pEngine, EVar* pIndex, EVar* pObj)
	{
		while(m_pArray->GetSize() <= pIndex->pIntObject->m_value)
			m_pArray->AddPointer(NULL);
		pEngine->PinObject(pObj->pOb);
		GObject* pOldOb = (GObject*)m_pArray->GetPointer(pIndex->pIntObject->m_value);
		pEngine->UnpinObject(pOldOb);
		m_pArray->SetPointer(pIndex->pIntObject->m_value, pObj->pOb);
	}

	GPointerArray* GetArray() { return m_pArray; }
};

#endif // __MARRAY_H__
