/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include <stdio.h>
#include "EMethod.h"
#include "EClass.h"
#include "EInstrArray.h"
#include "../CodeObjects/Method.h"
#include "../CodeObjects/Project.h"
#include "../CodeObjects/Class.h"
#include "../CodeObjects/Variable.h"
#include "../../GClasses/GXML.h"
#include "../../GClasses/GHashTable.h"
#include "../../GClasses/GMacros.h"

EMethod::EMethod()
:	m_pMethodTag(NULL),
	m_szName(NULL),
	m_pEInstrArray(NULL),
	m_pSignature(NULL),
	m_pClass(NULL)
{
}

EMethod::~EMethod()
{
	delete(m_pEInstrArray);
	delete(m_pSignature);
}

int EMethod::CountParams()
{
	int nCount = 0;
	GXMLTag* pParamTag;
	for(pParamTag = m_pMethodTag->GetFirstChildTag(); pParamTag; pParamTag = m_pMethodTag->GetNextChildTag(pParamTag))
	{
		if(stricmp(pParamTag->GetName(), TAG_NAME_VAR) != 0)
			continue;
		nCount++;
	}
	if(stricmp(m_pMethodTag->GetName(), TAG_NAME_METHOD) == 0)
		nCount++;
	return nCount;
}

const char* EMethod::FindParamName(int n)
{
	int nParam = 0;
	if(stricmp(m_pMethodTag->GetName(), TAG_NAME_METHOD) == 0)
	{
		if(n == 0)
			return "this";
		nParam++;
	}
	GXMLTag* pParamTag;
	for(pParamTag = m_pMethodTag->GetFirstChildTag(); pParamTag; pParamTag = m_pMethodTag->GetNextChildTag(pParamTag))
	{
		if(stricmp(pParamTag->GetName(), TAG_NAME_VAR) != 0)
			continue;
		if(nParam == n)
		{
			GXMLAttribute* pExpAttr = pParamTag->GetAttribute(ATTR_EXP);
			if(pExpAttr)
			{
				const char* szVarName = pExpAttr->GetValue();
				while(*szVarName != ':' && *szVarName != '\0')
					szVarName++;
				if(*szVarName == ':')
					szVarName++;
				return szVarName;
			}
			else
				return "<error>";
		}
		nParam++;
	}
	return "<error>";
}

EInstrArray* EMethod::GetEInstrArray(COMethod* pMethod)
{
	if(!m_pEInstrArray)
		m_pEInstrArray = new EInstrArray(m_pMethodTag, pMethod);
	return m_pEInstrArray;
}

EMethodSignature* EMethod::GetSignature()
{
	if(!m_pSignature)
		m_pSignature = new EMethodSignature(m_pMethodTag);
	return m_pSignature;
}

int EMethod::GetID()
{
	GXMLAttribute* pIDAttr = m_pMethodTag->GetAttribute(ATTR_ID);
	return atoi(pIDAttr->GetValue());
}

void EMethod::GetDependentMethods(GHashTable* pMethodTable, GHashTable* pTypeTable, Library* pLibrary, COProject* pProject)
{
	int nID = GetID();
	int nTmp;
	if(pMethodTable->Get(nID, (void**)&nTmp))
		return;
	pMethodTable->Add(nID, NULL);
	EInstrArray* pEInstrArray = GetEInstrArray(NULL);
	pEInstrArray->GetDependentMethods(pMethodTable, pTypeTable, pLibrary, pProject);
}

COMethod* EMethod::GetCOMethod(COProject* pProject)
{
	COClass* pClass = pProject->FindClass(m_pClass->GetName());
	if(!pClass)
		return NULL;
	EMethodSignature sig(m_pMethodTag);
	COMethod* pMethod = pClass->FindMethod(&sig);
	return pMethod;
}


// ---------------------------------------------------------


EMethodSignature::EMethodSignature(COMethodDecl* pMethod)
{
	int nParam = 0;
	int nParamCount = pMethod->GetParameterCount();
	if(pMethod->IsStatic())
		m_sSignature.Add("proc "); // todo: unmagic
	else
	{
		m_sSignature.Add("method "); // todo: unmagic
		COVariable* pThis = pMethod->GetParameter(0);
		if(!pThis->IsVarReadOnly())
			m_sSignature.Add("!");
		else if(!pThis->IsObjReadOnly())
			m_sSignature.Add("&");
		nParam++;
	}
	m_sSignature.Add(pMethod->GetName());
	m_sSignature.Add("(");
	bool bFirst = true;
	for( ; nParam < nParamCount; nParam++)
	{
		if(bFirst)
			bFirst = false;
		else
			m_sSignature.Add(", ");
		COVariable* pParam = pMethod->GetParameter(nParam);
		if(!pParam->IsVarReadOnly())
			m_sSignature.Add("!");
		else if(!pParam->IsObjReadOnly())
			m_sSignature.Add("&");
		m_sSignature.Add(pParam->GetType()->GetName());
	}
	m_sSignature.Add(")");
}

void EMethodSignature::AddParams(GXMLTag* pTag)
{
	GXMLTag* pChildTag;
	bool bFirst = true;
	for(pChildTag = pTag->GetFirstChildTag(); pChildTag; pChildTag = pTag->GetNextChildTag(pChildTag))
	{
		if(stricmp(pChildTag->GetName(), TAG_NAME_VAR) != 0)
			continue;
		if(bFirst)
			bFirst = false;
		else
			m_sSignature.Add(", ");
		GXMLAttribute* pExpAttr = pChildTag->GetAttribute(ATTR_EXP);
		GAssert(pExpAttr, "no expression");
		if(pExpAttr)
		{
			const char* pExp = pExpAttr->GetValue();
			int i;
			for(i = 0; pExp[i] != '\0' && pExp[i] != ':'; i++);
			GTEMPBUF(pBuf, i + 1);
			strncpy(pBuf, pExp, i);
			pBuf[i] = '\0';
			m_sSignature.Add(pBuf);
		}
	}
}

EMethodSignature::EMethodSignature(GXMLTag* pMethodTag)
{
	if(stricmp(pMethodTag->GetName(), TAG_NAME_PROCEDURE) == 0)
		m_sSignature.Add("proc "); // todo: unmagic
	else if(stricmp(pMethodTag->GetName(), TAG_NAME_METHOD) == 0)
		m_sSignature.Add("method "); // todo: unmagic
	GXMLAttribute* pNameAttr = pMethodTag->GetAttribute(ATTR_NAME);
	GAssert(pNameAttr, "No name!");
	if(pNameAttr)
		m_sSignature.Add(pNameAttr->GetValue());
	m_sSignature.Add("(");
	AddParams(pMethodTag);
	m_sSignature.Add(")");
}

EMethodSignature::EMethodSignature(const char* szSignature)
{
	m_sSignature.Add(szSignature);
}

int EMethodSignature::Compare(EMethodSignature* pThat)
{
	return m_sSignature.CompareTo(&pThat->m_sSignature);
}

