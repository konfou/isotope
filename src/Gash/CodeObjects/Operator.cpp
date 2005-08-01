/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "Operator.h"

int GetOperatorPrecidence(COOperator::Operator eOp)
{
	// Bigger = more precidence
	switch(eOp)
	{
		case COOperator::OP_POUND: return 9;
		case COOperator::OP_COLON: return 9;
		case COOperator::OP_DOT: return 8;
		case COOperator::OP_TIMES: return 7;
		case COOperator::OP_DIVIDE: return 6;
		case COOperator::OP_PLUS: return 5;
		case COOperator::OP_MINUS: return 5;
		case COOperator::OP_MODULUS: return 4;
		case COOperator::OP_XOR: return 3;
		case COOperator::OP_AND: return 2;
		case COOperator::OP_OR: return 2;
		case COOperator::OP_MAX: return 1;
		case COOperator::OP_MIN: return 1;
		case COOperator::OP_VAR_EQUALS: return 0;
		case COOperator::OP_VAR_NOT_EQUALS: return 0;
		case COOperator::OP_OBJ_EQUALS: return 0;
		case COOperator::OP_OBJ_NOT_EQUALS: return 0;
		case COOperator::OP_OBJ_LESS_THAN: return 0;
		case COOperator::OP_OBJ_LESS_THAN_OR_EQUAL: return 0;
		case COOperator::OP_OBJ_GREATER_THAN: return 0;
		case COOperator::OP_OBJ_GREATER_THAN_OR_EQUAL: return 0;
	}
	GAssert(false, "unrecognized operator");
	return 0;
}

/*static*/ int COOperator::ComparePrecidence(COOperator::Operator eOp1, COOperator::Operator eOp2)
{
	int nP1 = GetOperatorPrecidence(eOp1);
	int nP2 = GetOperatorPrecidence(eOp2);
	if(nP1 > nP2)
		return -1;
	if(nP2 > nP1)
		return 1;
	return 0;
}

int COOperator::ToString(char* pBuf)
{
	int nPos = 0;

	// First Expression
	if(m_pExpression1->GetExpressionType() == ET_COMPOUND)
	{
		if(pBuf)
			pBuf[nPos] = '(';
		nPos++;	
	}
	nPos += m_pExpression1->ToString(pBuf ? pBuf + nPos : NULL) - 1;
	if(m_pExpression1->GetExpressionType() == ET_COMPOUND)
	{
		if(pBuf)
			pBuf[nPos] = ')';
		nPos++;	
	}

	// Operator
	if(m_eOperator != OP_DOT)
	{
		if(pBuf)
			pBuf[nPos] = ' ';
		nPos++;
	}
	nPos += ToString(m_eOperator, (pBuf ? pBuf + nPos : NULL));
	if(m_eOperator != OP_DOT)
	{
		if(pBuf)
			pBuf[nPos] = ' ';
		nPos++;
	}

	// Second Expression
	if(m_pExpression2->GetExpressionType() == ET_COMPOUND)
	{
		if(pBuf)
			pBuf[nPos] = '(';
		nPos++;	
	}
	nPos += m_pExpression2->ToString(pBuf ? pBuf + nPos : NULL);
	if(m_pExpression2->GetExpressionType() == ET_COMPOUND)
	{
		if(pBuf)
			pBuf[nPos] = ')';
		nPos++;	
	}

	return nPos;
}


// -------------------------------------------------------------

/*static*/ int COOperator::ToString(COOperator::Operator eOp, char* szBuf)
{
	const char* szOp;
	switch(eOp)
	{
		case OP_POUND: szOp = "#"; break;
		case OP_COLON: szOp = ":"; break;
		case OP_DOT: szOp = "."; break;
		case OP_PLUS: szOp = "+"; break;
		case OP_MINUS: szOp = "-"; break;
		case OP_TIMES: szOp = "*"; break;
		case OP_DIVIDE: szOp = "/"; break;
		case OP_MODULUS: szOp = "%"; break;
		case OP_XOR: szOp = "^"; break;
		case OP_AND: szOp = "&"; break;
		case OP_OR: szOp = "|"; break;
		case OP_MAX: szOp = "?"; break;
		case OP_MIN: szOp = "~"; break;
		case OP_VAR_EQUALS: szOp = "=="; break;
		case OP_VAR_NOT_EQUALS: szOp = "!="; break;
		case OP_OBJ_EQUALS: szOp = "="; break;
		case OP_OBJ_NOT_EQUALS: szOp = "<>"; break;
		case OP_OBJ_LESS_THAN: szOp = "<"; break;
		case OP_OBJ_LESS_THAN_OR_EQUAL: szOp = "<="; break;
		case OP_OBJ_GREATER_THAN: szOp = ">"; break;
		case OP_OBJ_GREATER_THAN_OR_EQUAL: szOp = ">="; break;
		default:
			GAssert(false, "unexpected opcode");
			return 0;
	}
	int nLen = strlen(szOp);
	if(szBuf)
		memcpy(szBuf, szOp, nLen);
	return nLen;
}

/*static*/ int COOperator::GetOperator(const char* szExpr, int nExprSize, COOperator::Operator* pOp, bool bInNumber)
{
	// Eat whitespace
	int nPos = 0;
	while(nPos < nExprSize && szExpr[nPos] <= ' ')
		nPos++;
	if(nPos >= nExprSize)
		return 0;

	bool bGotIt = true;
	int nLen = 1;
	switch(szExpr[nPos])
	{
		case '#': *pOp = OP_POUND; break;
		case ':': *pOp = OP_COLON; break;
		case '.':
			if(!bInNumber)
				*pOp = OP_DOT;
			else
				bGotIt = false;
			break;
		case '+': *pOp = OP_PLUS; break;
		case '-': *pOp = OP_MINUS; break;
		case '*': *pOp = OP_TIMES; break;
		case '/': *pOp = OP_DIVIDE; break;
		case '%': *pOp = OP_MODULUS; break;
		case '^': *pOp = OP_XOR; break;
		case '&': *pOp = OP_AND; break;
		case '|': *pOp = OP_OR; break;
		case '?': *pOp = OP_MAX; break;
		case '~': *pOp = OP_MIN; break;
		case '=':
			if(szExpr[nPos + 1] == '=')
			{
				*pOp = OP_VAR_EQUALS;
				nLen++;
			}
			else
				*pOp = OP_OBJ_EQUALS;
			break;
		case '!':
			if(szExpr[nPos + 1] == '=')
			{
				*pOp = OP_VAR_NOT_EQUALS;
				nLen++;
			}
			else
				bGotIt = false;
			break;
		case '<':
			if(szExpr[nPos + 1] == '>')
			{
				*pOp = OP_OBJ_NOT_EQUALS;
				nLen++;
			}
			else if(szExpr[nPos + 1] == '=')
			{
				*pOp = OP_OBJ_LESS_THAN_OR_EQUAL;
				nLen++;
			}
			else
				*pOp = OP_OBJ_LESS_THAN;
			break;
		case '>':
			if(szExpr[nPos + 1] == '=')
			{
				*pOp = OP_OBJ_GREATER_THAN_OR_EQUAL;
				nLen++;
			}
			else
				*pOp = OP_OBJ_GREATER_THAN;
			break;
		default:
			bGotIt = false;
	}
	if(bGotIt)
		return nPos + nLen;
	return 0;
}

/*virtual*/ bool COOperator::SetExpReadOnly(bool bObj, bool bVar, const char** ppszVarName)
{
	GAssert(bVar || !bObj, "If you can modify the var, you can modify the obj too");
	if(m_eOperator == OP_DOT)
	{
		if(bVar)
		{
			if(!m_pExpression1->SetExpReadOnly(bObj, true, ppszVarName))
				return false;
			if(!m_pExpression2->SetExpReadOnly(bObj, true, ppszVarName))
				return false;
		}
		else
		{
			if(!m_pExpression1->SetExpReadOnly(false, true, ppszVarName))
				return false;
			if(!m_pExpression2->SetExpReadOnly(false, false, ppszVarName))
				return false;
		}
	}
	else if(m_eOperator == OP_COLON || m_eOperator == OP_POUND)
	{
		if(!m_pExpression2->SetExpReadOnly(bObj, bVar, ppszVarName))
			return false;
	}
	else
	{
		if(!bVar)
		{
			*ppszVarName = "<non-variable expression>";
			return false;
		}
		if(!m_pExpression1->SetExpReadOnly(bObj, true, ppszVarName))
			return false;
		if(!m_pExpression2->SetExpReadOnly(bObj, true, ppszVarName))
			return false;
	}
	return true;
}

