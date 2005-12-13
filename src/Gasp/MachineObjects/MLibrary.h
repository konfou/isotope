/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __MLIBRARY_H__
#define __MLIBRARY_H__

#include "../BuiltIns/GaspString.h"
#include "../Include/GaspEngine.h"
#include "../../GClasses/GMacros.h"

class MLibrary : public WrapperObject
{
friend class GaspEngine;
protected:
	Library* m_pLibrary;

	MLibrary(Engine* pEngine, Library* pLibrary)
		: WrapperObject(pEngine, "Library")
	{
		m_pLibrary = pLibrary;
	}

public:
	virtual ~MLibrary()
	{
		delete(m_pLibrary);
	}

	void toStream(Engine* pEngine, EVar* pOutBlob, EVar* pOutRefs)
	{
		GAssert(false, "todo: write me");
	}

	void fromStream(Engine* pEngine, EVar* pStream)
	{
		GAssert(false, "todo: write me");
	}

	void setRefs(Engine* pEngine, EVar* pRefs)
	{
		GAssert(false, "todo: write me");
	}

	virtual void GetDisplayValue(wchar_t* pBuf, int nSize)
	{
		wcscpy(pBuf, L"<Library>"); // todo: do something better
	}

	void load(Engine* pEngine, EVar* pFilename);
	void save(Engine* pEngine, EVar* pFilename);
};

#endif // __MLIBRARY_H__
