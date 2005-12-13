/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GaspFloat.h"
#include "../../GClasses/GHashTable.h"
#include "../../GClasses/GQueue.h"
#include "GaspStream.h"

void RegisterGaspFloat(GConstStringHashTable* pTable)
{
	pTable->Add("method &add(Float)", new EMethodPointerHolder((MachineMethod1)&GaspFloat::add));
	pTable->Add("method add(!Float, Float)", new EMethodPointerHolder((MachineMethod2)&GaspFloat::addTwo));
	pTable->Add("method &add(Integer)", new EMethodPointerHolder((MachineMethod1)&GaspFloat::addInt));
	pTable->Add("method compareTo(&Bool, Float)", new EMethodPointerHolder((MachineMethod2)&GaspFloat::compareTo));
	pTable->Add("method &cosine(Float)", new EMethodPointerHolder((MachineMethod1)&GaspFloat::cosine));
	pTable->Add("method isEqual(&Bool, Float)", new EMethodPointerHolder((MachineMethod2)&GaspFloat::isEqual));
	pTable->Add("method isNotEqual(&Bool, Float)", new EMethodPointerHolder((MachineMethod2)&GaspFloat::isNotEqual));
	pTable->Add("method isLessThan(&Bool, Float)", new EMethodPointerHolder((MachineMethod2)&GaspFloat::isLessThan));
	pTable->Add("method isGreaterThan(&Bool, Float)", new EMethodPointerHolder((MachineMethod2)&GaspFloat::isGreaterThan));
	pTable->Add("method !newcopy(Float)", new EMethodPointerHolder((MachineMethod1)&GaspFloat::newcopy));
	pTable->Add("method &copy(Float)", new EMethodPointerHolder((MachineMethod1)&GaspFloat::copy));
	pTable->Add("method &copy(Integer)", new EMethodPointerHolder((MachineMethod1)&GaspFloat::copyInt));
	pTable->Add("method &divide(Float)", new EMethodPointerHolder((MachineMethod1)&GaspFloat::divide));
	pTable->Add("method divide(!Float, Float)", new EMethodPointerHolder((MachineMethod2)&GaspFloat::divideTwo));
	pTable->Add("method &divide(Integer)", new EMethodPointerHolder((MachineMethod1)&GaspFloat::divideInt));
	pTable->Add("method &fromInteger(Integer)", new EMethodPointerHolder((MachineMethod1)&GaspFloat::fromInteger));
	pTable->Add("method &fromString(String)", new EMethodPointerHolder((MachineMethod1)&GaspFloat::fromString));
	pTable->Add("method isZero(&Bool)", new EMethodPointerHolder((MachineMethod1)&GaspFloat::isZero));
	pTable->Add("method isPositive(&Bool)", new EMethodPointerHolder((MachineMethod1)&GaspFloat::isPositive));
	pTable->Add("method isNonZero(&Bool)", new EMethodPointerHolder((MachineMethod1)&GaspFloat::isNonZero));
	pTable->Add("method max(!Float, Float)", new EMethodPointerHolder((MachineMethod2)&GaspFloat::maxTwo));
	pTable->Add("method min(!Float, Float)", new EMethodPointerHolder((MachineMethod2)&GaspFloat::minTwo));
	pTable->Add("method &multiply(Float)", new EMethodPointerHolder((MachineMethod1)&GaspFloat::multiply));
	pTable->Add("method multiply(!Float, Float)", new EMethodPointerHolder((MachineMethod2)&GaspFloat::multiplyTwo));
	pTable->Add("method &multiply(Integer)", new EMethodPointerHolder((MachineMethod1)&GaspFloat::multiplyInt));
	pTable->Add("method !new()", new EMethodPointerHolder((MachineMethod0)&GaspFloat::allocate));
	pTable->Add("method &sine(Float)", new EMethodPointerHolder((MachineMethod1)&GaspFloat::sine));
	pTable->Add("method &subtract(Float)", new EMethodPointerHolder((MachineMethod1)&GaspFloat::subtract));
	pTable->Add("method subtract(!Float, Float)", new EMethodPointerHolder((MachineMethod2)&GaspFloat::subtractTwo));
	pTable->Add("method &subtract(Integer)", new EMethodPointerHolder((MachineMethod1)&GaspFloat::subtractInt));
	pTable->Add("method toInteger(&Integer)", new EMethodPointerHolder((MachineMethod1)&GaspFloat::toInteger));
	pTable->Add("method toString(&String)", new EMethodPointerHolder((MachineMethod1)&GaspFloat::toString));
	pTable->Add("method toStream(&Stream, &Stream)", new EMethodPointerHolder((MachineMethod2)&GaspFloat::toStream));
	pTable->Add("method !fromStream(&Stream)", new EMethodPointerHolder((MachineMethod1)&GaspFloat::fromStream));
	pTable->Add("method &setRefs(&Stream)", new EMethodPointerHolder((MachineMethod1)&GaspFloat::setRefs));
}

void GaspFloat::toStream(Engine* pEngine, EVar* pStream, EVar* pRefs)
{
	GQueue* pQ = &pStream->pStreamObject->m_value;
	unsigned char* pBytes = (unsigned char*)&m_value;
	int n;
	for(n = 0; n < sizeof(double); n++)
		pQ->Push(pBytes[n]);
}

void GaspFloat::fromStream(Engine* pEngine, EVar* pStream)
{
	GaspFloat* pNewFloat = new GaspFloat(pEngine);
	GQueue* pQ = &pStream->pStreamObject->m_value;
	unsigned char* pBytes = (unsigned char*)&pNewFloat->m_value;
	int n;
	for(n = 0; n < sizeof(double); n++)
		pQ->Pop(&pBytes[n]);
	pEngine->SetThis(pNewFloat);
}

void GaspFloat::setRefs(Engine* pEngine, EVar* pRefs)
{
	GAssert(false, "todo: throw--this should never be called");
}
