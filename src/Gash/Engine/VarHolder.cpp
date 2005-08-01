/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "TagNames.h"
#include "../Include/GashEngine.h"
#include "../../GClasses/GString.h"
#include "../CodeObjects/Method.h"
#include "../BuiltIns/GashString.h"
#include "../BuiltIns/GashFloat.h"

VarHolder::VarHolder(Engine* pEngine)
{
	GAssert(pEngine, "must supply a valid Engine pointer");
	Init(pEngine);
}

VarHolder::VarHolder(Engine* pEngine, Primitive* pPrimitive)
{
	GAssert(pEngine, "must supply a valid Engine pointer");
	Init(pEngine);

	switch(pPrimitive->GetType())
	{
	case Primitive::PT_INTEGER:
		{
			pEngine->SetVar(&m_var, m_pEngine->Allocate(pEngine->GetLibrary()->GetInteger()));
			m_var.pIntObject->m_value = ((PrimitiveInteger*)pPrimitive)->m_nValue;
		}
		break;

	case Primitive::PT_BOOL:
		{
			EClass* pClass = (EClass*)pEngine->GetLibrary()->FindTypeConst(CLASS_NAME_BOOL); // todo: make sure it's really a class
			GAssert(pClass, "Couldn't find Bool class");
			pEngine->SetVar(&m_var, pEngine->Allocate(pClass));
			m_var.pIntObject->m_value = ((PrimitiveBool*)pPrimitive)->m_bValue ? 1 : 0; // This assumes that 1 = True, and 0 = False
		}
		break;

	case Primitive::PT_STRING:
		{
			GAssert(false, "todo: primitive strings not supported yet");
			// Find the "New" procedure
			EClass* pClass = (EClass*)pEngine->GetLibrary()->FindTypeConst(CLASS_NAME_STRING);  // todo: make sure it's really a class
			GAssert(pClass, "Couldn't find String class");
			EMethodSignature methodSig(SIG_NEW);
			struct MethodRef mr;
			bool bOK = pEngine->GetLibrary()->FindMethod(&mr, pClass, &methodSig);
			GAssert(bOK, "Couldn't find String.new--todo: handle this case");

			// Call it
			VarHolder* pThis = this;
			pEngine->Call(&mr, &pThis, 1);

			// Check it
			GAssert(m_var.pOb, "Failed to construct the string");

			// Set the value
			GString* pString = &m_var.pStringObject->m_value;
			pString->Copy(((PrimitiveString*)pPrimitive)->GetValue());
		}
		break;

	case Primitive::PT_FLOAT:
		{
			EClass* pClass = (EClass*)pEngine->GetLibrary()->FindTypeConst(CLASS_NAME_FLOAT); // todo: make sure it's really a class
			GAssert(pClass, "Couldn't find Float class");
			pEngine->SetVar(&m_var, pEngine->Allocate(pClass));
			m_var.pFloatObject->m_value = ((PrimitiveFloat*)pPrimitive)->m_dValue;
		}
		break;

	default:
		GAssert(false, "unexpected type");
		break;
	}

	// Pin the object and release the implicit ref from the allocation
	m_pEngine->PinObject(m_var.pOb);
	m_pEngine->Release(m_var.pOb);
}

VarHolder::~VarHolder()
{
	if(m_pEngine)
		SetGObject(NULL);
#ifdef _DEBUG
	// Remove from linked list
	if(m_pNext)
		m_pNext->m_pPrev = m_pPrev;
	if(m_pPrev)
		m_pPrev->m_pNext = m_pNext;
	else
		m_pEngine->m_pVarHolders = m_pNext;
#endif // _DEBUG
}

void VarHolder::Init(Engine* pEngine)
{
	m_pEngine = pEngine;
#ifdef _DEBUG
	// Add to linked list of all VarHolders for this Engine
	m_szID = NULL;
	m_pNext = m_pEngine->m_pVarHolders;
	m_pPrev = NULL;
	if(m_pEngine->m_pVarHolders)
		m_pEngine->m_pVarHolders->m_pPrev = this;
	m_pEngine->m_pVarHolders = this;
#endif // _DEBUG
	m_var.eObType = VT_OB_REF;
	m_var.pOb = NULL;
}

void VarHolder::SetGObject(GObject* pObject)
{
	if(pObject)
		m_pEngine->PinObject(pObject);
	if(m_var.pOb)
		m_pEngine->UnpinObject(m_var.pOb);
	m_var.pOb = pObject;
}

// --------------------------------------------------

// I didn't know where to put this method so I put it here
void PrimitiveString::SetValue(const wchar_t* sValue)
{
	delete(m_sValue);
	m_sValue = new wchar_t[wcslen(sValue) + 1];
	wcscpy(m_sValue, sValue);
}
