/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __COVariable_H__
#define __COVariable_H__

#include "CodeObject.h"
#include "Instruction.h"
#include "Expression.h"
#include "../../GClasses/GMacros.h"

class COInterface;
class GParser;
class COType;

class COVariable : public COExpression
{
protected:
	COType* m_pType;
	char* m_szName;
	bool m_bDeclVarIsReadOnly;
	bool m_bDeclObjIsReadOnly;
	bool m_bAutoAlloc;

public:
	COVariable(int nLine, int nCol, int nWid, const char* szName, COType* pType, bool bVarIsReadOnly, bool bObjIsReadOnly, bool bAutoAlloc);
	virtual ~COVariable();

	virtual COType* GetType(COProject* pCOProject)
	{
		return m_pType;
	}

	COType* GetType() { return m_pType; }

	bool IsAutoAlloc() { return m_bAutoAlloc; }
	const char* GetName() { return m_szName; }
	void SetName(const char* szName) { GAssert(szName, "Must have a valid name"); delete(m_szName); m_szName = new char[strlen(szName) + 1]; strcpy(m_szName, szName); }
	void SetType(COType* pType) { m_pType = pType; }

	static COVariable* FromXML(GXMLTag* pTag, COProject* pCOProject, bool bPartial);
	virtual GXMLTag* SaveToXML();
	virtual void SaveToClassicSyntax(GQueue* pQ);
	static void FromClassicSyntax(ClassicSyntax* pParser, GXMLTag* pParentTag);

	bool IsVarReadOnly() { return m_bDeclVarIsReadOnly; }
	bool IsObjReadOnly() { return m_bDeclObjIsReadOnly; }
	void SetVarReadOnly(bool bRO) { m_bDeclVarIsReadOnly = bRO; }
	void SetObjReadOnly(bool bRO) { m_bDeclObjIsReadOnly = bRO; }

	virtual bool SetExpReadOnly(bool bObj, bool bVar, const char** ppszVarName)
	{
		GAssert(bVar || !bObj, "If you can modify the var, you can modify the obj");
		m_bDeclObjIsReadOnly = bObj;
		m_bDeclVarIsReadOnly = bVar;
		return true;
	}

	virtual ErrorStruct* CheckReadOnlyAccess(bool bWillModifyObj, bool bWillModifyVar, const char** ppszVarName)
	{
		GAssert(!bWillModifyVar || bWillModifyObj, "If you modify the var, the obj is modified too");
		if(bWillModifyVar && m_bDeclVarIsReadOnly)
		{
			*ppszVarName = GetName();
			return &Error::EXPECTED_BANG_MODIFIER;
		}
		if(bWillModifyObj && m_bDeclObjIsReadOnly)
		{
			*ppszVarName = GetName();
			return &Error::EXPECTED_AMPERSAND_MODIFIER;
		}
		return NULL;
	}

	virtual int ToString(char* pBuf);
	virtual COVariable* FindVariable(const char* pName, int nLength);
};

#endif // __COVariable_H__
