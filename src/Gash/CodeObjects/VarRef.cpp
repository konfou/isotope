/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "VarRef.h"
#include "Variable.h"
#include "Type.h"

COVarRef::COVarRef(int nLine, int nCol, int nWid, COVariable* pVariable)
	: COExpression(nLine, nCol, nWid, ET_VARREF),
	m_bExpVarIsReadOnly(true),
	m_bExpObjIsReadOnly(true)
{
	GAssert(pVariable != NULL, "Bad Var");
	m_pVariable = pVariable;
}

/*virtual*/ COVarRef::~COVarRef()
{
}

int COVarRef::ToString(char* pBuf)
{
	const char* szName = m_pVariable->GetName();
	int nSize = strlen(szName);
	if(pBuf)
		strcpy(pBuf, szName);
	return strlen(szName) + 1;
}

/*virtual*/ COType* COVarRef::GetType(COProject* pCOProject)
{
	return m_pVariable->GetType();
}

/*virtual*/ ErrorStruct* COVarRef::CheckReadOnlyAccess(bool bWillModifyObj, bool bWillModifyVar, const char** ppszVarName)
{
	GAssert(!bWillModifyVar || bWillModifyObj, "If you modify the var, the obj is modified too");
	if(bWillModifyVar && m_bExpVarIsReadOnly)
	{
		*ppszVarName = m_pVariable->GetName();
		return &Error::EXPECTED_BANG_MODIFIER;
	}
	if(bWillModifyObj && m_bExpObjIsReadOnly)
	{
		*ppszVarName = m_pVariable->GetName();
		return &Error::EXPECTED_AMPERSAND_MODIFIER;
	}
	if(m_pVariable->IsVarReadOnly() && !m_bExpVarIsReadOnly)
	{
		*ppszVarName = m_pVariable->GetName();
		return &Error::CANT_MODIFY_VARIABLE;
	}
	if(m_pVariable->IsObjReadOnly() && !m_bExpObjIsReadOnly)
	{
		*ppszVarName = m_pVariable->GetName();
		return &Error::CANT_MODIFY_OBJECT;
	}
	return NULL;
}

/*virtual*/ bool COVarRef::SetExpReadOnly(bool bObj, bool bVar, const char** ppszVarName)
{
	GAssert(bVar || !bObj, "If you can modify the var, you can modify the obj too");
	m_bExpObjIsReadOnly = bObj;
	m_bExpVarIsReadOnly = bVar;
	if(!bObj && m_pVariable->IsObjReadOnly())
	{
		*ppszVarName = m_pVariable->GetName();
		return false;
	}
	if(!bVar && m_pVariable->IsVarReadOnly())
	{
		*ppszVarName = m_pVariable->GetName();
		return false;
	}
	return true;
}
