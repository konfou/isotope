/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __OPERATOR_H__
#define __OPERATOR_H__

#include "Expression.h"

class COOperator : public COExpression
{
protected:
	COExpression* m_pExpression1;
	COExpression* m_pExpression2;
	Operator m_eOperator;

public:
	COOperator(int nLine, int nCol, int nWid, COExpression* pExpression1, Operator eOperator, COExpression* pExpression2) : COExpression(nLine, nCol, nWid, ET_COMPOUND)
	{
		GAssert(pExpression1 != NULL && pExpression2 != NULL, "Bad params");
		m_pExpression1 = pExpression1;
		m_pExpression2 = pExpression2;
		m_eOperator = eOperator;
	}

	virtual ~COOperator()
	{
		delete(m_pExpression1);
		delete(m_pExpression2);
	}

	Operator GetOperator()
	{
		return m_eOperator;
	}

	COExpression* GetLeftExpression()
	{
		return m_pExpression1;
	}

	COExpression* GetRightExpression()
	{
		return m_pExpression2;
	}

	COExpression* DropLeftExpression()
	{
		COExpression* pExp = m_pExpression1;
		m_pExpression1 = NULL;
		return pExp;
	}

	COExpression* DropRightExpression()
	{
		COExpression* pExp = m_pExpression2;
		m_pExpression2 = NULL;
		return pExp;
	}

	virtual COVariable* FindVariable(const char* pName, int nLength)
	{
		if(m_eOperator == OP_COLON || m_eOperator == OP_POUND)
			return m_pExpression2->FindVariable(pName, nLength);
		return NULL;
	}

	virtual bool SetExpReadOnly(bool bObj, bool bVar, const char** ppszVarName);

	virtual ErrorStruct* CheckReadOnlyAccess(bool bWillModifyObj, bool bWillModifyVar, const char** ppszVarName)
	{
		GAssert(!bWillModifyVar || bWillModifyObj, "If you modify the var, the obj is modified too");
		ErrorStruct* pError = m_pExpression1->CheckReadOnlyAccess(bWillModifyObj, false, ppszVarName);
		if(pError)
			return pError;
		return m_pExpression2->CheckReadOnlyAccess(bWillModifyObj, bWillModifyVar, ppszVarName);
	}

	virtual COType* GetType(COProject* pCOProject);
	virtual int ToString(char* pBuf);

	static int GetOperator(const char* szExpr, int nExprSize, COOperator::Operator* pOp, bool bInNumber);
	static int ToString(COOperator::Operator eOp, char* szBuf);
	static int ComparePrecidence(COOperator::Operator eOp1, COOperator::Operator eOp2);

	static bool IsComparator(Operator op)
	{
		switch(op)
		{
			case OP_OBJ_EQUALS:
			case OP_OBJ_NOT_EQUALS:
			case OP_OBJ_LESS_THAN:
			case OP_OBJ_LESS_THAN_OR_EQUAL:
			case OP_OBJ_GREATER_THAN:
			case OP_OBJ_GREATER_THAN_OR_EQUAL:
				return true;
			default:
				return false;
		}
	}
};

#endif // __OPERATOR_H__
