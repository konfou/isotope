/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "Variable.h"
#include "../Engine/TagNames.h"
#include "Project.h"
#include "Class.h"
#include "Interface.h"
#include "../../GClasses/GXML.h"
#include "../../GClasses/GQueue.h"
#ifdef DARWIN
#include <sys/malloc.h>
#else // DARWIN
#include <malloc.h>
#endif // !DARWIN
#include "Operator.h"

COVariable::COVariable(int nLine, int nCol, int nWid, const char* szName, COType* pType, bool bVarIsReadOnly, bool bObjIsReadOnly, bool bAutoAlloc)
: COExpression(nLine, nCol, nWid, ET_VARDECL)
{
	GAssert(bVarIsReadOnly || !bObjIsReadOnly, "If the var can be modified, so can the obj");
	GAssert(szName, "You must have a valid name");
	m_szName = new char[strlen(szName) + 1];
	m_pType = pType;
	strcpy(m_szName, szName);
	m_bDeclVarIsReadOnly = bVarIsReadOnly;
	m_bDeclObjIsReadOnly = bObjIsReadOnly;
	m_bAutoAlloc = bAutoAlloc;
}

COVariable::~COVariable()
{
	delete(m_szName);
}

/*static*/ COVariable* COVariable::FromXML(GXMLTag* pTag, COProject* pCOProject, bool bPartial)
{
	// Get the expression
	GXMLAttribute* pAttrExp = pTag->GetAttribute(ATTR_EXP);
	if(!pAttrExp)
		pCOProject->ThrowError(&Error::EXPECTED_EXP_ATTRIBUTE, pTag);

	// Convert to an expression
	Holder<COExpression*> hExp(COExpression::LoadFromExpression(pAttrExp->GetValue(), NULL, pCOProject, pTag, bPartial));

	// Check expression type
	if(hExp.Get()->GetExpressionType() != COExpression::ET_VARDECL)
		pCOProject->ThrowError(&Error::EXPECTED_COLON, pTag);
	return (COVariable*)hExp.Drop();
}


GXMLTag* COVariable::SaveToXML()
{
	GXMLTag* pVariable = new GXMLTag(TAG_NAME_VAR);
	const char* szTypeName = GetType()->GetName();
	char* szExp = (char*)alloca(1 + strlen(szTypeName) + 1 + strlen(m_szName) + 1);
	char* szModifier = "";
	if(!m_bDeclVarIsReadOnly)
		szModifier = "!";
	else if(!m_bDeclObjIsReadOnly)
		szModifier = "&";
	strcpy(szExp, szModifier);
	strcat(szExp, szTypeName);
	strcat(szExp, ":");
	strcat(szExp, m_szName);
	pVariable->AddAttribute(new GXMLAttribute(ATTR_EXP, szExp));
	return pVariable;			 
}

void COVariable::SaveToClassicSyntax(GQueue* pQ)
{
	if(!IsVarReadOnly())
		pQ->Push("!");
	else if(!IsObjReadOnly())
		pQ->Push("&");
	pQ->Push(GetType()->GetName());
	pQ->Push(":");
	pQ->Push(GetName());
}

/*static*/ void COVariable::FromClassicSyntax(ClassicSyntax* pParser, GXMLTag* pParentTag)
{
#ifdef NEWPARSER
	// Get next token
	int nStartCol = pParser->m_nPos - pParser->m_nLineStartPos + 1;
	CSToken* pTok = pParser->GetToken(0);
	if(pTok->GetLength() <= 0)
	{
		pParser->SetError(&Error::UNEXPECTED_EOF);
		return;
	}

	// Make the tag
	GXMLTag* pVarTag = new GXMLTag(TAG_NAME_VAR);
	pVarTag->SetLineNumber(pParser->m_nLineNumber);
	int nEndCol = pParser->m_nPos - pParser->m_nLineStartPos + 1;
	pVarTag->SetColumnAndWidth(nStartCol, nEndCol - nStartCol);
	pParentTag->AddChildTag(pVarTag);
	pVarTag->AddAttribute(new GXMLAttribute(ATTR_EXP, pTok->GetValue(), pTok->GetLength()));
	bool b = pParser->Advance();
	GAssert(b, "unexpected state");
#else // NEWPARSER
	// Get next token
	int nStartCol = pParser->m_nPos - pParser->m_nLineStartPos + 1;
	if(!pParser->PeekToken())
	{
		pParser->SetError(&Error::UNEXPECTED_EOF);
		return;
	}

	// Find the ':'
	bool bFoundColon = false;
	int n;
	for(n = 0; pParser->m_pTokenBuffer[n] != '\0'; n++)
	{
		if(pParser->m_pTokenBuffer[n] == ':')
			bFoundColon = true;
	}
	if(!bFoundColon)
	{
		pParser->SetError(&Error::EXPECTED_COLON);
		return;
	}

	// Make the tag
	GXMLTag* pVarTag = new GXMLTag(TAG_NAME_VAR);
	pVarTag->SetLineNumber(pParser->m_nLineNumber);
	int nEndCol = pParser->m_nPos - pParser->m_nLineStartPos + 1;
	pVarTag->SetColumnAndWidth(nStartCol, nEndCol - nStartCol);
	pParentTag->AddChildTag(pVarTag);
	pVarTag->AddAttribute(new GXMLAttribute(ATTR_EXP, pParser->m_pTokenBuffer));
	bool b = pParser->Advance();
	GAssert(b, "unexpected state");
#endif // NEWPARSER
}

/*virtual*/ int COVariable::ToString(char* pBuf)
{
	int nPos = 0;
	const char* pTypeName = m_pType->GetName();
	int nTypeNameLen = strlen(pTypeName);
	int nNameLen = strlen(m_szName);
	if(pBuf)
	{
		strcpy(pBuf, pTypeName);
		pBuf[nTypeNameLen] = ':';
		strcpy(pBuf + nTypeNameLen + 1, m_szName);
	}
	return nTypeNameLen + 1 + nNameLen + 1;
}

/*virtual*/ COVariable* COVariable::FindVariable(const char* pName, int nLength)
{
	if(strnicmp(pName, m_szName, nLength) == 0 && m_szName[nLength] == '\0')
		return this;
	return NULL;
}

void COVariable::SetType(COType* pType)
{
	m_pType = pType;
}

void COVariable::ReplaceType(COType* pOld, COType* pNew)
{
	if(m_pType == pOld)
		m_pType = pNew;
}
