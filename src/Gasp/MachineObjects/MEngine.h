/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __GASP_ENGINE_H__
#define __GASP_ENGINE_H__

#include "../Include/GaspEngine.h"
#include <stdlib.h>
#include "../Engine/EType.h"

class GaspEngine : public WrapperObject
{
	GaspEngine(Engine* pEngine)
		: WrapperObject(pEngine, "Engine")
	{
	}

public:
	virtual ~GaspEngine()
	{
	}

	void collectTheGarbage(Engine* pEngine)
	{
		pEngine->CollectTheGarbage();
	}

	void getObjectCount(Engine* pEngine, EVar* pOutCount)
	{
		pOutCount->pIntObject->m_value = pEngine->GetObjectCount();
	}

	void getClass(Engine* pEngine, EVar* pOutClassID, EVar* pObject)
	{
		pOutClassID->pIntObject->m_value = pObject->pOb->GetType()->GetID();
	}

	void getRandomNumber(Engine* pEngine, EVar* pOutInt)
	{
		unsigned char* pChars = (unsigned char*)&pOutInt->pIntObject->m_value;
		pChars[0] = rand() % 256;
		pChars[1] = rand() % 256;
		pChars[2] = rand() % 256;
		pChars[3] = rand() % 256;
	}

	void getRefs(Engine* pEngine, EVar* pOutRefs, EVar* pObject)
	{
		pOutRefs->pIntObject->m_value = pObject->pOb->nRefCount + pObject->pOb->nPinnedRefs;
	}

	void buildProject(Engine* pEngine, EVar* pLibrary, EVar* pProject);

	void setGlobalObject(Engine* pEngine, EVar* pObj)
	{
		pEngine->SetGlobalObject(pObj->pOb);
	}

	void getGlobalObject(Engine* pEngine, EVar* pDest)
	{
		pEngine->SetVar(pDest, pEngine->GetGlobalObject());
	}
};


#endif // __GASP_ENGINE_H__
