/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "MArray.h"
#include "MachineObjects.h"
#include "../../GClasses/GHashTable.h"
#include "../Include/GaspLib.h"
#include "../BuiltIns/GaspStream.h"

void RegisterMArray(GConstStringHashTable* pTable)
{
	pTable->Add("method &add(Object)", new EMethodPointerHolder((MachineMethod1)&MArray::add));
	pTable->Add("method !new(Integer)", new EMethodPointerHolder((MachineMethod1)&MArray::allocate));
	pTable->Add("method get(!Object, Integer)", new EMethodPointerHolder((MachineMethod2)&MArray::get));
	pTable->Add("method getSize(&Integer)", new EMethodPointerHolder((MachineMethod1)&MArray::getSize));
	pTable->Add("method &set(Integer, Object)", new EMethodPointerHolder((MachineMethod2)&MArray::set));	
	pTable->Add("method toStream(&Stream, &Stream)", new EMethodPointerHolder((MachineMethod2)&MArray::toStream));
	pTable->Add("method !fromStream(&Stream)", new EMethodPointerHolder((MachineMethod1)&MArray::fromStream));
	pTable->Add("method &setRefs(&Stream)", new EMethodPointerHolder((MachineMethod1)&MArray::setRefs));
}

void MArray::toStream(Engine* pEngine, EVar* pOutBlob, EVar* pOutRefs)
{
	int nSize = m_pArray->GetSize();
	pOutBlob->pStreamObject->m_value.Push(nSize);
	int n;
	for(n = 0; n < nSize; n++)
		pOutRefs->pStreamObject->m_value.Push(m_pArray->GetPointer(n));
}

void MArray::fromStream(Engine* pEngine, EVar* pStream)
{
	int nSize;
	pStream->pStreamObject->m_value.Pop(&nSize);
	MArray* pNewArray = new MArray(pEngine, nSize);
	while(pNewArray->m_pArray->GetSize() < nSize)
		pNewArray->m_pArray->AddPointer(NULL);
	pEngine->SetThis(pNewArray);
}

void MArray::setRefs(Engine* pEngine, EVar* pRefs)
{
	int nSize = m_pArray->GetSize();
	GObject* pOb;
	int n;
	for(n = 0; n < nSize; n++)
	{
		pRefs->pStreamObject->m_value.Pop((void**)&pOb);
		m_pArray->SetPointer(n, pOb);
	}
}
