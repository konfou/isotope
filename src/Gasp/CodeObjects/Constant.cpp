/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include <stdlib.h>
#include "Constant.h"
#include "../Engine/TagNames.h"
#include "../../GClasses/GXML.h"
#include "../../GClasses/GQueue.h"
#include "Project.h"
#ifdef DARWIN
#include <sys/malloc.h>
#else // DARWIN
#include <malloc.h>
#endif // !DARWIN

COConstant::COConstant(int nLine, int nCol, int nWid, const char* szName, const char* szValue)
: CodeObject(nLine, nCol, nWid)
{
	m_szName = NULL;
	m_szValue = NULL;
	SetName(szName);
	SetValue(szValue);
}

COConstant::COConstant(int nLine, int nCol, int nWid, const char* szName, int nValue)
: CodeObject(nLine, nCol, nWid)
{
	m_szName = NULL;
	m_szValue = NULL;
	SetName(szName);
	SetValue(nValue);
}

COConstant::~COConstant()
{
	delete(m_szName);
	delete(m_szValue);
}

void COConstant::SetName(const char* szName)
{
	delete(m_szName);
	m_szName = new char[strlen(szName) + 1];
	strcpy(m_szName, szName);
}

void COConstant::SetValue(const char* szValue)
{
	delete(m_szValue);
	m_szValue = new char[strlen(szValue) + 1];
	strcpy(m_szValue, szValue);
	m_bIsString = true;
	m_nValue = 0;
}

void COConstant::SetValue(int nValue)
{
	delete(m_szValue);
	m_szValue = NULL;
	m_nValue = nValue;
	m_bIsString = false;
}

COConstant* COConstant::LoadFromXML(GXMLTag* pTag, COProject* pCOProject)
{
	int nLine = pTag->GetLineNumber();
	int nCol, nWid;
	pTag->GetOffsetAndWidth(&nCol, &nWid);
	GXMLAttribute* pNameTag = pTag->GetAttribute(ATTR_NAME);
	if(!pNameTag)
		pCOProject->ThrowError(&Error::EXPECTED_NAME_ATTRIBUTE, pTag);
	GXMLAttribute* pValueTag = pTag->GetAttribute(ATTR_VAL);
	if(!pValueTag)
		pCOProject->ThrowError(&Error::EXPECTED_VALUE_ATTRIBUTE, pTag);
	const char* szValue = pValueTag->GetValue();
	COConstant* pNewConstant;
	if(*szValue == '\'')
	{
		char* szTmp = (char*)alloca(strlen(szValue));
		int n;
		szValue++;
		for(n = 0; szValue[n] != '\0'; n++)
			szTmp[n] = szValue[n];
		if(szTmp[n - 1] != '\'')
			pCOProject->ThrowError(&Error::UNTERMINATED_STRING_CONSTANT, pTag);
		szTmp[n - 1] = '\0';
		pNewConstant = new COConstant(nLine, nCol, nWid, pNameTag->GetValue(), szTmp);
	}
	else
	{
		int n;
		for(n = 0; szValue[n] != '\0'; n++)
		{
			if(szValue[n] != '-' && (szValue[n] < '0' || szValue[n] > '9'))
				pCOProject->ThrowError(&Error::INVALID_CONSTANT_CHARACTER, pTag);
		}
		pNewConstant = new COConstant(nLine, nCol, nWid, pNameTag->GetValue(), atoi(szValue));
	}
	return pNewConstant;
}

GXMLTag* COConstant::SaveToXML()
{
	GXMLTag* pConstantTag = new GXMLTag(TAG_NAME_CONSTANT);
	pConstantTag->AddAttribute(new GXMLAttribute(ATTR_NAME, GetName()));
	if(IsString())
	{
		const char* szString = GetStringValue();
		int nLen = strlen(szString);
		char* szTmp = (char*)alloca(nLen + 3);
		szTmp[0] = '\'';
		strcpy(szTmp + 1, szString);
		szTmp[nLen + 1] = '\'';
		szTmp[nLen + 2] = '\0';
		pConstantTag->AddAttribute(new GXMLAttribute(ATTR_VAL, szTmp));
	}
	else
	{
		char szTmp[32];
		itoa(GetIntegerValue(), szTmp, 10);
		pConstantTag->AddAttribute(new GXMLAttribute(ATTR_VAL, szTmp));
	}
	return pConstantTag;
}

void COConstant::SaveToClassicSyntax(GQueue* pQ)
{
	pQ->Push("\tconst ");
	pQ->Push(m_szName);
	pQ->Push("(");
	if(m_bIsString)
	{
		pQ->Push("'");
		pQ->Push(m_szValue);
		pQ->Push("'");
	}
	else
	{
		char szTmp[32];
		itoa(GetIntegerValue(), szTmp, 10);
		pQ->Push(szTmp);
	}
	pQ->Push(")\n");
}
