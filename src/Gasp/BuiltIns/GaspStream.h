/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GASPSTREAM_H__
#define __GASPSTREAM_H__

#include <wchar.h>
#include "../Include/GaspEngine.h"
#include "../../GClasses/GQueue.h"
#include "../../GClasses/GMacros.h"

class GaspStream : public WrapperObject
{
friend class Engine;
public:
	GQueue m_value;

public:
	GaspStream(Engine* pEngine)
		: WrapperObject(pEngine, "Stream")
	{
	}

	virtual ~GaspStream()
	{
	}

	void toStream(Engine* pEngine, EVar* pStream, EVar* pRefs);
	void fromStream(Engine* pEngine, EVar* pStream);
	void setRefs(Engine* pEngine, EVar* pRefs);

	virtual void GetDisplayValue(wchar_t* pBuf, int nSize)
	{
		GAssert(nSize > 32, "Buffer too small");
		swprintf(pBuf, 32, L"Queue of size %d", m_value.GetSize());
	}

	void allocate(Engine* pEngine)
	{
		pEngine->SetThis(new GaspStream(pEngine));
	}

	void getSize(Engine* pEngine, EVar* pOutSize)
	{
		pOutSize->pIntObject->m_value = m_value.GetSize();
	}

	void writeObject(Engine* pEngine, EVar* pObject)
	{
		EVar v;
		v.eObType = VT_OB_REF;
		v.pOb = this;
		pEngine->SerializeObject(pObject->pOb, &v);
	}

	void readObject(Engine* pEngine, EVar* pObject)
	{
		EVar v;
		v.eObType = VT_OB_REF;
		v.pOb = this;
		pEngine->SetVar(pObject, pEngine->DeserializeObject(&v));
	}
};


#endif // __GASPSTREAM_H__
