/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include <string.h>
#include "../Include/GaspEngine.h"
#include "../CodeObjects/Method.h"
#include "../../GClasses/GHashTable.h"
#include "../../GClasses/GQueue.h"
#include "../../GClasses/GPointerQueue.h"
#include "GObject.h"
#include "../Engine/EClass.h"
#include "../BuiltIns/GaspStream.h"
#include "../../GClasses/GMacros.h"

void ObjectObject::toStream(Engine* pEngine, EVar* pOutBlob, EVar* pOutRefs)
{
	GAssert(false, "this should never be called");
}

void ObjectObject::fromStream(Engine* pEngine, EVar* pStream)
{
	GAssert(false, "this should never be called");
}

void ObjectObject::setRefs(Engine* pEngine, EVar* pRefs)
{
	GAssert(false, "this should never be called");
}

// -------------------------------------------------------------------------

void IntObject::toStream(Engine* pEngine, EVar* pOutBlob, EVar* pOutRefs)
{
	pOutBlob->pStreamObject->m_value.Push(m_value);
}

void IntObject::fromStream(Engine* pEngine, EVar* pStream)
{
	GAssert(false, "todo: throw--this should never be called");
}

void IntObject::setRefs(Engine* pEngine, EVar* pRefs)
{
	GAssert(false, "todo: throw--this should never be called");
}

// -------------------------------------------------------------------------

WrapperObject::WrapperObject(Engine* pEngine, const char* szClassName)
{
	memset(this, '\0', sizeof(GObject));
	m_pType = pEngine->m_pLibrary->FindTypeConst(szClassName);
	GAssert(m_pType, "Couldn't find machine object with that name--todo: handle this case");

#ifdef _DEBUG
	// Set Alloc ID
	nAllocID = pEngine->m_nObjectID++;
#endif // _DEBUG

	// Track this object for garbage collection
	LinkObject(this, &pEngine->m_objectList);
	pEngine->m_nObjectCount++;
}


WrapperObject::~WrapperObject()
{
}

/*static*/ void WrapperObject::RegisterMachineClass(GConstStringHashTable** ppClassesTable, const char* szClassName, RegisterMethods pRegisterFunc)
{
	// Lazily create the class table
	if(*ppClassesTable == NULL)
		*ppClassesTable = new GConstStringHashTable(13, true);
	else
	{
		GAssert((*ppClassesTable)->GetCount() >= 0 && (*ppClassesTable)->GetCount() < 50000, "Looks like a bogus table pointer.  This is probably due to the order in which your compiler initializes globals.");
	}

	// Make a table of all the methods in the class
	GConstStringHashTable* pMethodList = new GConstStringHashTable(7, true);
	pRegisterFunc(pMethodList);

	// Add the method table to the classes table
	(*ppClassesTable)->Add(szClassName, pMethodList);
}

/*static*/ EMethodPointerHolder* WrapperObject::FindMachineMethod(GConstStringHashTable* pClassesTable, const char* szClassName, EMethodSignature* pMethodSignature)
{
	// Find the method table
	GAssert(pClassesTable, "No machine classes were ever added to the table");
	GAssert(pClassesTable->GetCount() >= 0 && pClassesTable->GetCount() < 50000, "looks like a corrupted hash table");
	GConstStringHashTable* pMethodTable;
	if(!pClassesTable->Get(szClassName, (void**)&pMethodTable))
		return NULL;

	// Find the method
	const wchar_t* wszSig = pMethodSignature->GetString();
	ConvertUnicodeToAnsi(wszSig, szSig);
	EMethodPointerHolder* pFunc;
	if(!pMethodTable->Get(szSig, (void**)&pFunc))
		return NULL;

	return pFunc;
}

