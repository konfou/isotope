/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __VARREF_H__
#define __VARREF_H__

#include "Expression.h"

class COVarRef : public COExpression
{
protected:
	COVariable* m_pVariable;
	bool m_bExpVarIsReadOnly;
	bool m_bExpObjIsReadOnly;

public:
	COVarRef(int nLine, int nCol, int nWid, COVariable* pVariable)
	 : COExpression(nLine, nCol, nWid, ET_VARREF),
	   m_bExpVarIsReadOnly(true),
	   m_bExpObjIsReadOnly(true)
	{
		GAssert(pVariable != NULL, "Bad Var");
		m_pVariable = pVariable;
	}

	virtual ~COVarRef()
	{
	}

	virtual COType* GetType(COProject* pCOProject);
	virtual int ToString(char* pBuf);
	virtual ErrorStruct* CheckReadOnlyAccess(bool bWillModifyObj, bool bWillModifyVar, const char** ppszVarName);
	virtual bool SetExpReadOnly(bool bObj, bool bVar, const char** ppszVarName);

	COVariable* GetVar()
	{
		GAssert(m_eExpressionType == ET_VARREF, "not a ET_VARREF type");
		return m_pVariable;
	}
};

#endif // __VARREF_H__
