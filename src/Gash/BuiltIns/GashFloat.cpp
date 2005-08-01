/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GashFloat.h"
#include "../../GClasses/GHashTable.h"
#include "../../GClasses/GQueue.h"
#include "GashStream.h"

void RegisterGashFloat(GConstStringHashTable* pTable)
{
	pTable->Add("method &add(Float)", new EMethodPointerHolder((MachineMethod1)&GashFloat::add));
	pTable->Add("method add(!Float, Float)", new EMethodPointerHolder((MachineMethod2)&GashFloat::addTwo));
	pTable->Add("method &add(Integer)", new EMethodPointerHolder((MachineMethod1)&GashFloat::addInt));
	pTable->Add("method compareTo(&Bool, Float)", new EMethodPointerHolder((MachineMethod2)&GashFloat::compareTo));
	pTable->Add("method &cosine(Float)", new EMethodPointerHolder((MachineMethod1)&GashFloat::cosine));
	pTable->Add("method isEqual(&Bool, Float)", new EMethodPointerHolder((MachineMethod2)&GashFloat::isEqual));
	pTable->Add("method isNotEqual(&Bool, Float)", new EMethodPointerHolder((MachineMethod2)&GashFloat::isNotEqual));
	pTable->Add("method isLessThan(&Bool, Float)", new EMethodPointerHolder((MachineMethod2)&GashFloat::isLessThan));
	pTable->Add("method isGreaterThan(&Bool, Float)", new EMethodPointerHolder((MachineMethod2)&GashFloat::isGreaterThan));
	pTable->Add("method !newcopy(Float)", new EMethodPointerHolder((MachineMethod1)&GashFloat::newcopy));
	pTable->Add("method &copy(Float)", new EMethodPointerHolder((MachineMethod1)&GashFloat::copy));
	pTable->Add("method &copy(Integer)", new EMethodPointerHolder((MachineMethod1)&GashFloat::copyInt));
	pTable->Add("method &divide(Float)", new EMethodPointerHolder((MachineMethod1)&GashFloat::divide));
	pTable->Add("method divide(!Float, Float)", new EMethodPointerHolder((MachineMethod2)&GashFloat::divideTwo));
	pTable->Add("method &divide(Integer)", new EMethodPointerHolder((MachineMethod1)&GashFloat::divideInt));
	pTable->Add("method &fromInteger(Integer)", new EMethodPointerHolder((MachineMethod1)&GashFloat::fromInteger));
	pTable->Add("method &fromString(String)", new EMethodPointerHolder((MachineMethod1)&GashFloat::fromString));
	pTable->Add("method isZero(&Bool)", new EMethodPointerHolder((MachineMethod1)&GashFloat::isZero));
	pTable->Add("method isPositive(&Bool)", new EMethodPointerHolder((MachineMethod1)&GashFloat::isPositive));
	pTable->Add("method isNonZero(&Bool)", new EMethodPointerHolder((MachineMethod1)&GashFloat::isNonZero));
	pTable->Add("method max(!Float, Float)", new EMethodPointerHolder((MachineMethod2)&GashFloat::maxTwo));
	pTable->Add("method min(!Float, Float)", new EMethodPointerHolder((MachineMethod2)&GashFloat::minTwo));
	pTable->Add("method &multiply(Float)", new EMethodPointerHolder((MachineMethod1)&GashFloat::multiply));
	pTable->Add("method multiply(!Float, Float)", new EMethodPointerHolder((MachineMethod2)&GashFloat::multiplyTwo));
	pTable->Add("method &multiply(Integer)", new EMethodPointerHolder((MachineMethod1)&GashFloat::multiplyInt));
	pTable->Add("method !new()", new EMethodPointerHolder((MachineMethod0)&GashFloat::allocate));
	pTable->Add("method &sine(Float)", new EMethodPointerHolder((MachineMethod1)&GashFloat::sine));
	pTable->Add("method &subtract(Float)", new EMethodPointerHolder((MachineMethod1)&GashFloat::subtract));
	pTable->Add("method subtract(!Float, Float)", new EMethodPointerHolder((MachineMethod2)&GashFloat::subtractTwo));
	pTable->Add("method &subtract(Integer)", new EMethodPointerHolder((MachineMethod1)&GashFloat::subtractInt));
	pTable->Add("method toInteger(&Integer)", new EMethodPointerHolder((MachineMethod1)&GashFloat::toInteger));
	pTable->Add("method toString(&String)", new EMethodPointerHolder((MachineMethod1)&GashFloat::toString));
	pTable->Add("method toStream(&Stream, &Stream)", new EMethodPointerHolder((MachineMethod2)&GashFloat::toStream));
	pTable->Add("method !fromStream(&Stream)", new EMethodPointerHolder((MachineMethod1)&GashFloat::fromStream));
	pTable->Add("method &setRefs(&Stream)", new EMethodPointerHolder((MachineMethod1)&GashFloat::setRefs));
}

void GashFloat::toStream(Engine* pEngine, EVar* pStream, EVar* pRefs)
{
	GQueue* pQ = &pStream->pStreamObject->m_value;
	unsigned char* pBytes = (unsigned char*)&m_value;
	int n;
	for(n = 0; n < sizeof(double); n++)
		pQ->Push(pBytes[n]);
}

void GashFloat::fromStream(Engine* pEngine, EVar* pStream)
{
	GashFloat* pNewFloat = new GashFloat(pEngine);
	GQueue* pQ = &pStream->pStreamObject->m_value;
	unsigned char* pBytes = (unsigned char*)&pNewFloat->m_value;
	int n;
	for(n = 0; n < sizeof(double); n++)
		pQ->Pop(&pBytes[n]);
	pEngine->SetThis(pNewFloat);
}

void GashFloat::setRefs(Engine* pEngine, EVar* pRefs)
{
	GAssert(false, "todo: throw--this should never be called");
}
