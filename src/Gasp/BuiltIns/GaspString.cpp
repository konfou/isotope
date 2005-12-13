/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GaspString.h"
#include "../../GClasses/GHashTable.h"
#include "../../GClasses/GQueue.h"
#include "GaspStream.h"

void RegisterGaspString(GConstStringHashTable* pTable)
{
    pTable->Add("method &add(String)", new EMethodPointerHolder((MachineMethod1)&GaspString::add));
	pTable->Add("method !new()", new EMethodPointerHolder((MachineMethod0)&GaspString::allocate));
	pTable->Add("method &clear()", new EMethodPointerHolder((MachineMethod0)&GaspString::clear));
	pTable->Add("method compare(&Integer, String)", new EMethodPointerHolder((MachineMethod2)&GaspString::compare));
	pTable->Add("method compareIgnoringCase(&Integer, String)", new EMethodPointerHolder((MachineMethod2)&GaspString::compareIgnoringCase));
    pTable->Add("method &copy(String)", new EMethodPointerHolder((MachineMethod1)&GaspString::copy));
    pTable->Add("method &copySub(String, Integer, Integer)", new EMethodPointerHolder((MachineMethod3)&GaspString::copySub));
	pTable->Add("method find(&Integer, String)", new EMethodPointerHolder((MachineMethod2)&GaspString::find));
	pTable->Add("method findIgnoringCase(&Integer, String)", new EMethodPointerHolder((MachineMethod2)&GaspString::findIgnoringCase));
	pTable->Add("method &fromInteger(Integer)", new EMethodPointerHolder((MachineMethod1)&GaspString::fromInteger));
	pTable->Add("method !getConstantString(Integer)", new EMethodPointerHolder((MachineMethod1)&GaspString::getConstantString));
    pTable->Add("method getLength(&Integer)", new EMethodPointerHolder((MachineMethod1)&GaspString::getLength));
	pTable->Add("method toInteger(&Integer)", new EMethodPointerHolder((MachineMethod1)&GaspString::toInteger));
	pTable->Add("method &toLower()", new EMethodPointerHolder((MachineMethod0)&GaspString::toLower));
	pTable->Add("method &toUpper()", new EMethodPointerHolder((MachineMethod0)&GaspString::toUpper));
	pTable->Add("method toStream(&Stream, &Stream)", new EMethodPointerHolder((MachineMethod2)&GaspString::toStream));
	pTable->Add("method !fromStream(&Stream)", new EMethodPointerHolder((MachineMethod1)&GaspString::fromStream));
	pTable->Add("method &setRefs(&Stream)", new EMethodPointerHolder((MachineMethod1)&GaspString::setRefs));
	pTable->Add("method &setChar(Integer, Integer)", new EMethodPointerHolder((MachineMethod2)&GaspString::setChar));
	pTable->Add("method getChar(&Integer, Integer)", new EMethodPointerHolder((MachineMethod2)&GaspString::getChar));
}

void GaspString::toStream(Engine* pEngine, EVar* pStream, EVar* pRefs)
{
	int nLen = m_value.GetLength();
	GQueue* pQ = &pStream->pStreamObject->m_value;
	pQ->Push(nLen);
	int n;
	for(n = 0; n < nLen; n++)
		pQ->Push(m_value.GetWChar(n));
}

void GaspString::fromStream(Engine* pEngine, EVar* pStream)
{
	GaspString* pNewString = new GaspString(pEngine);
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

void GaspString::setRefs(Engine* pEngine, EVar* pRefs)
{
	GAssert(false, "todo: throw--this should never be called");
}
