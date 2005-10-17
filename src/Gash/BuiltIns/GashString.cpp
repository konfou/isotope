/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GashString.h"
#include "../../GClasses/GHashTable.h"
#include "../../GClasses/GQueue.h"
#include "GashStream.h"

void RegisterGashString(GConstStringHashTable* pTable)
{
    pTable->Add("method &add(String)", new EMethodPointerHolder((MachineMethod1)&GashString::add));
	pTable->Add("method !new()", new EMethodPointerHolder((MachineMethod0)&GashString::allocate));
	pTable->Add("method &clear()", new EMethodPointerHolder((MachineMethod0)&GashString::clear));
	pTable->Add("method compare(&Integer, String)", new EMethodPointerHolder((MachineMethod2)&GashString::compare));
	pTable->Add("method compareIgnoringCase(&Integer, String)", new EMethodPointerHolder((MachineMethod2)&GashString::compareIgnoringCase));
    pTable->Add("method &copy(String)", new EMethodPointerHolder((MachineMethod1)&GashString::copy));
    pTable->Add("method &copySub(String, Integer, Integer)", new EMethodPointerHolder((MachineMethod3)&GashString::copySub));
	pTable->Add("method find(&Integer, String)", new EMethodPointerHolder((MachineMethod2)&GashString::find));
	pTable->Add("method findIgnoringCase(&Integer, String)", new EMethodPointerHolder((MachineMethod2)&GashString::findIgnoringCase));
	pTable->Add("method &fromInteger(Integer)", new EMethodPointerHolder((MachineMethod1)&GashString::fromInteger));
	pTable->Add("method !getConstantString(Integer)", new EMethodPointerHolder((MachineMethod1)&GashString::getConstantString));
    pTable->Add("method getLength(&Integer)", new EMethodPointerHolder((MachineMethod1)&GashString::getLength));
	pTable->Add("method toInteger(&Integer)", new EMethodPointerHolder((MachineMethod1)&GashString::toInteger));
	pTable->Add("method &toLower()", new EMethodPointerHolder((MachineMethod0)&GashString::toLower));
	pTable->Add("method &toUpper()", new EMethodPointerHolder((MachineMethod0)&GashString::toUpper));
	pTable->Add("method toStream(&Stream, &Stream)", new EMethodPointerHolder((MachineMethod2)&GashString::toStream));
	pTable->Add("method !fromStream(&Stream)", new EMethodPointerHolder((MachineMethod1)&GashString::fromStream));
	pTable->Add("method &setRefs(&Stream)", new EMethodPointerHolder((MachineMethod1)&GashString::setRefs));
	pTable->Add("method &setChar(Integer, Integer)", new EMethodPointerHolder((MachineMethod2)&GashString::setChar));
	pTable->Add("method getChar(&Integer, Integer)", new EMethodPointerHolder((MachineMethod2)&GashString::getChar));
}

void GashString::toStream(Engine* pEngine, EVar* pStream, EVar* pRefs)
{
	int nLen = m_value.GetLength();
	GQueue* pQ = &pStream->pStreamObject->m_value;
	pQ->Push(nLen);
	int n;
	for(n = 0; n < nLen; n++)
		pQ->Push(m_value.GetWChar(n));
}

void GashString::fromStream(Engine* pEngine, EVar* pStream)
{
	GashString* pNewString = new GashString(pEngine);
	GQueue* pQ = &pStream->pStreamObject->m_value;
	int nLen;
	pQ->Pop(&nLen); // todo: handle error conditions by throwing
	pNewString->m_value.SetChar(nLen - 1, L' ');
	wchar_t wc;
	int n;
	for(n = 0; n < nLen; n++)
	{
		pQ->Pop(&wc);
		pNewString->m_value.SetChar(n, wc);
	}
	pEngine->SetThis(pNewString);
}

void GashString::setRefs(Engine* pEngine, EVar* pRefs)
{
	GAssert(false, "todo: throw--this should never be called");
}
