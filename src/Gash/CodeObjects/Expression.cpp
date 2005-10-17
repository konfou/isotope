/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "Expression.h"
#include "Method.h"
#include "Variable.h"
#include "Project.h"
#include "Class.h"
#include "Interface.h"
#include "Constant.h"
#include "../Engine/GCompiler.h"
#include "../Engine/EvalExprResult.h"
#include "../../GClasses/GXML.h"
#include "../../GClasses/GQueue.h"
#include "Operator.h"
#include "VarRef.h"
#include "Call.h"
#include "../Engine/InstrSet.h"
#ifdef DARWIN
#include <sys/malloc.h>
#else // DARWIN
#include <malloc.h>
#endif // !DARWIN

inline void EatLeadingWhitespace(const char** pszExpression, int* pnLen)
{
	while((*pnLen) > 0 && **pszExpression <= ' ')
	{
		(*pszExpression)++;
		(*pnLen)--;
	}
}

inline void EatTrailingWhitespace(const char** pszExpression, int* pnLen)
{
	while((*pnLen) > 0 && *((*pszExpression) + (*pnLen) - 1) <= ' ')
		(*pnLen)--;
}

/*static*/ COExpression* COExpression::ParseExpression(const char* szExpression, int nLen, COScope* pScope, COProject* pCOProject, GXMLTag* pTag, bool bPartial)
{
	// Eat whitespace
	EatTrailingWhitespace(&szExpression, &nLen);
	EatLeadingWhitespace(&szExpression, &nLen);
	if(nLen <= 0)
		pCOProject->ThrowError(&Error::EMPTY_EXPRESSION, pTag);

	// Parse modifiers
	bool bObjReadOnly = true;
	bool bVarReadOnly = true;
	if(*szExpression == '&')
	{
		bObjReadOnly = false;
		szExpression++;
		nLen--;
	}
	else if(*szExpression == '!')
	{
		bObjReadOnly = false;
		bVarReadOnly = false;
		szExpression++;
		nLen--;
	}

	// Parse the expression
	Holder<COExpression*> hExpr(Parse(szExpression, nLen, pScope, OP_NONE, NULL, pCOProject, pTag, bPartial));
	COExpression* pExpr = hExpr.Get();

	// Set modifiers
	const char* szTmpVarName;
	if(!pExpr->SetExpReadOnly(bObjReadOnly, bVarReadOnly, &szTmpVarName))
	{
		if(!bVarReadOnly)
			pCOProject->ThrowError(&Error::CANT_MODIFY_VARIABLE, pTag, szTmpVarName);
		else
		{
			GAssert(!bObjReadOnly, "read only access should always succeed");
			pCOProject->ThrowError(&Error::CANT_MODIFY_OBJECT, pTag, szTmpVarName);
		}
		GAssert(false, "how'd you get here?");
	}

	return hExpr.Drop();
}

// pClass should be NULL unless we are parsing the operand of a dot operator
/*static*/ COExpression* COExpression::Parse(const char* szExpression, int nLen, COScope* pScope, Operator eOp, COType* pType, COProject* pCOProject, GXMLTag* pTag, bool bPartial)
{
	// Find the content
	while(true)
	{
		// Eat whitespace
		EatTrailingWhitespace(&szExpression, &nLen);
		EatLeadingWhitespace(&szExpression, &nLen);
		if(nLen <= 0)
			pCOProject->ThrowError(&Error::EMPTY_EXPRESSION, pTag);

		// Check for unbalanced parens
		if(*szExpression == ')')
			pCOProject->ThrowError(&Error::UNBALANCED_PARENS, pTag);

		// Remove redundant Parenthesis
		if(*szExpression == '(' && *(szExpression + nLen - 1) == ')')
		{
			// Make sure it's not a case like "(a + b) * (c + d)"
			int nSum = 1;
			int n;
			for(n = 1; n < nLen - 1; n++)
			{
				if(szExpression[n] == '(')
					nSum++;
				else if(szExpression[n] == ')')
					nSum--;
				if(nSum < 1)
					break;
			}
			if(n >= nLen - 1)
			{
				szExpression++;
				nLen -= 2;
				continue;
			}
		}

		break;
	}

	// Find operator with lowest precidence.  (If there are multiple operators with
	// the lowest precidence, use the right-most one because that's what the dot
	// operator needs and the other operators are all commutative.)
	COOperator::Operator opBest = OP_NONE;
	int nBestOpPos;
	int nBestOpLen;
	COOperator::Operator opCur;
	int n;
	int nParenNests = 0;
	bool bInsideString = false;
	bool bInNumber = true;
	bool bNewToken = true;
	for(n = 0; n < nLen; n++)
	{
		if(szExpression[n] == '"')
			bInsideString = !bInsideString;
		else if(!bInsideString)
		{
			if(bNewToken)
				bInNumber = true;
			if(szExpression[n] < '0' ||
				(szExpression[n] > '9' && szExpression[n] < 'A') ||
				szExpression[n] > 'z')
				bNewToken = true;
			else
				bNewToken = false;
			if((szExpression[n] < '0' || szExpression[n] > '9') && szExpression[n] != '.' && szExpression[n] != '-')
				bInNumber = false;
			if(szExpression[n] == '(')
				nParenNests++;
			else if(szExpression[n] == ')')
				nParenNests--;
			else if(nParenNests < 0)
				pCOProject->ThrowError(&Error::UNBALANCED_PARENS, pTag);
			else if(nParenNests == 0)
			{
				int nOpLen = COOperator::GetOperator(szExpression + n, nLen - n, &opCur, bInNumber);
				if(nOpLen > 0)
				{
					if((n > 0 || opCur != COOperator::OP_MINUS) &&
						(opBest == OP_NONE || COOperator::ComparePrecidence(opBest, opCur) <= 0))
					{
						opBest = opCur;
						nBestOpPos = n;
						nBestOpLen = nOpLen;
					}
					n += (nOpLen - 1);
				}
			}
		}
	}

	// Parse the expression
	if(opBest == OP_NONE)
		return ParseToken(szExpression, nLen, pScope, eOp, pType, pCOProject, pTag, bPartial);
	else
	{
		// Parse the first expression part
		Holder<COExpression*> hChild1(Parse(szExpression, nBestOpPos, pScope, eOp, pType, pCOProject, pTag, bPartial));

		// Parse second expression part
		szExpression += (nBestOpPos + nBestOpLen);
		nLen -= (nBestOpPos + nBestOpLen);
		Holder<COExpression*> hChild2(Parse(szExpression, nLen, pScope, opBest, hChild1.Get()->GetType(pCOProject), pCOProject, pTag, bPartial));

		// Make the COOperator object
		int nLine = pTag->GetLineNumber();
		int nCol, nWid;
		pTag->GetOffsetAndWidth(&nCol, &nWid);
		if(opBest == OP_COLON || opBest == OP_POUND)
			return hChild2.Drop(); // Just delete hChild1 because it's not needed
		else
		{
			COOperator* pOperator = new COOperator(nLine, nCol, nWid, hChild1.Get(), opBest, hChild2.Get());
			hChild1.Drop();
			hChild2.Drop();
			return pOperator;
		}
	}
}

/*static*/ COExpression* COExpression::ParseToken(const char* szExpression, int nLen, COScope* pScope, Operator eOp, COType* pType, COProject* pCOProject, GXMLTag* pTag, bool bPartial)
{
	// Parse the token
	switch(eOp)
	{
		case OP_DOT:
			if(pType->GetTypeType() != COType::TT_CLASS)
				pCOProject->ThrowError(&Error::INTERFACES_CANT_HAVE_MEMBERS, pTag);
			return ParseMember(szExpression, nLen, (COClass*)pType, pTag, pCOProject, bPartial);
		case OP_COLON:
			return ParseDecl(szExpression, nLen, pType, pCOProject, pTag, bPartial, false);
		case OP_POUND:
			return ParseDecl(szExpression, nLen, pType, pCOProject, pTag, bPartial, true);
		default:
			return ParseIdentifier(szExpression, nLen, pScope, pCOProject, pTag, bPartial);
	}
}

/*static*/ COExpression* COExpression::ParseMember(const char* szExpression, int nLen, COClass* pClass, GXMLTag* pTag, COProject* pProject, bool bPartial)
{
	int nLine = pTag->GetLineNumber();
	int nCol, nWid;
	pTag->GetOffsetAndWidth(&nCol, &nWid);

	// See if it's a member
	COVariable* pVariable = pClass->FindMember(szExpression, nLen);
	if(pVariable)
		return(new COVarRef(nLine, nCol, nWid, pVariable));

	// See if it's a named constant
	COConstant* pConstant = pClass->FindConstant(szExpression, nLen);
	if(pConstant)
		return(new CONamedConst(nLine, nCol, nWid, pConstant));

	if(bPartial)
		return(new COVarRef(nLine, nCol, nWid, pProject->m_pNull));

	// Not found--Make an error
	pProject->ThrowError(&Error::VARIABLE_NOT_FOUND, pTag, szExpression); // todo: copy nLen chars of szExpression

	GAssert(false, "how'd you get here?");
	return NULL;
}

/*static*/ COExpression* COExpression::ParseDecl(const char* szExpression, int nLen, COType* pType, COProject* pCOProject, GXMLTag* pTag, bool bPartial, bool bAutoAlloc)
{
	int nLine = pTag->GetLineNumber();
	int nCol, nWid;
	pTag->GetOffsetAndWidth(&nCol, &nWid);

	// Eat whitespace
	EatLeadingWhitespace(&szExpression, &nLen);
	EatTrailingWhitespace(&szExpression, &nLen);

	// Extract the name
	char* pName = (char*)alloca(nLen + 1);
	memcpy(pName, szExpression, nLen);
	pName[nLen] = '\0';

	return new COVariable(nLine, nCol, nWid, pName, pType, true, true, bAutoAlloc);
}

/*static*/ COExpression* COExpression::ParseIdentifier(const char* szExpression, int nLen, COScope* pScope, COProject* pCOProject, GXMLTag* pTag, bool bPartial)
{
	int nLine = pTag->GetLineNumber();
	int nCol, nWid;
	pTag->GetOffsetAndWidth(&nCol, &nWid);

	// See if it's an integer value
	int n;
	bool bConstantNumber = true;
	bool bConstantFloat = false;
	for(n = 0; n < nLen; n++)
	{
		if(szExpression[n] >= '0' && szExpression[n] <= '9')
			continue;
		if(n == 0 && szExpression[n] == '-')
			continue;
		if(szExpression[n] == '.' && !bConstantFloat)
		{
			bConstantFloat = true;
			continue;
		}
		bConstantNumber = false;
		break;
	}
	if(bConstantNumber)
	{
		if(bConstantFloat)
		{
			// It's a float value
			int nClippedLength = nLen;
			if(nClippedLength > 60)
				nClippedLength = 60;
			char szTmp[64];
			for(n = 0; n < nClippedLength; n++)
				szTmp[n] = szExpression[n];
			szTmp[n] = '\0';
			return(new COConstFloat(nLine, nCol, nWid, atof(szTmp)));
		}
		else
		{
			// It's an integer value
			if(nLen > 10)
				pCOProject->ThrowError(&Error::INTEGER_TOO_BIG, pTag);
			char szTmp[16];
			for(n = 0; n < nLen; n++)
				szTmp[n] = szExpression[n];
			szTmp[n] = '\0';
			return(new COConstInt(nLine, nCol, nWid, atoi(szTmp)));
		}
	}

	// See if it's a string value
	if(szExpression[0] == '"')
	{
		const char* pEnd = szExpression + nLen - 1;
		int nStringLength = nLen - 2;
		while(*pEnd == ' ')
		{
			pEnd--;
			nStringLength--;
		}
		if(pEnd <= szExpression || *pEnd != '"')
			pCOProject->ThrowError(&Error::UNTERMINATED_STRING_CONSTANT, pTag);
		char* szTmp = (char*)alloca(nStringLength + 1);
		int n;
		for(n = 0; n < nStringLength; n++)
			szTmp[n] = szExpression[n + 1];
		szTmp[n] = '\0';
		return(new COConstString(nLine, nCol, nWid, szTmp));
	}

	// See if it's "null"
	if(nLen == 4 && strnicmp(szExpression, VAL_NULL, strlen(VAL_NULL)) == 0)
		return new COVarRef(nLine, nCol, nWid, pCOProject->m_pNull);

	// See if it's a local variable
	if(pScope)
	{
		COVariable* pVariable = pScope->FindVariable(szExpression, nLen);
		if(pVariable)
			return(new COVarRef(nLine, nCol, nWid, pVariable)); // todo: don't allow variables that are out of scope
	}

	// See if it's a type
	COType* pTypeTmp;
	if(bPartial)
		pTypeTmp = pCOProject->m_pInteger;
	else
		pTypeTmp = pCOProject->FindType(szExpression, nLen);
	if(pTypeTmp)
		return(new COTypeRef(nLine, nCol, nWid, pTypeTmp));

	char* szVarName = (char*)alloca(nLen + 1);
	memcpy(szVarName, szExpression, nLen);
	szVarName[nLen] = '\0';
	pCOProject->ThrowError(&Error::VARIABLE_NOT_FOUND, pTag, szVarName);

	GAssert(false, "how'd you get here?");
	return NULL;
}

COExpression* COExpression::LoadFromExpression(const char* szExpr, COScope* pScope, COProject* pCOProject, GXMLTag* pTag, bool bPartial)
{
	return ParseExpression(szExpr, strlen(szExpr), pScope, pCOProject, pTag, bPartial);
}

COExpression* COExpression::LoadFromXML(GXMLTag* pTag, COScope* pScope, COProject* pCOProject, bool bPartial)
{
	if(stricmp(pTag->GetName(), TAG_NAME_PARAM) != 0)
		pCOProject->ThrowError(&Error::EXPECTED_PARAM_TAG, pTag);
	GXMLAttribute* pExp = pTag->GetAttribute(ATTR_EXP);
	if(!pExp)
		pCOProject->ThrowError(&Error::EXPECTED_EXP_ATTRIBUTE, pTag);
	return LoadFromExpression(pExp->GetValue(), pScope, pCOProject, pTag, bPartial);
}

GXMLTag* COExpression::SaveToXML()
{
	int nSize = ToString(NULL);
	GTEMPBUF(pBuf, nSize);
	ToString(pBuf);
	GXMLAttribute* pAttr = new GXMLAttribute(ATTR_EXP, pBuf);
	GXMLTag* pTag = new GXMLTag(TAG_NAME_PARAM);
	pTag->AddAttribute(pAttr);
	return pTag;
}

void COExpression::SaveToClassicSyntax(GQueue* pQ)
{
	const char* szTmp;
	if(!CheckReadOnlyAccess(true, true, &szTmp))
		pQ->Push("!");
	else if(!CheckReadOnlyAccess(true, false, &szTmp))
		pQ->Push("&");
	int nSize = ToString(NULL);
	GTEMPBUF(pBuf, nSize);
	ToString(pBuf);
	pQ->Push(pBuf);
}

extern const char* g_stdDel;

/*static*/ void COExpression::FromClassicSyntax(ClassicSyntax* pParser, GXMLTag* pParentTag)
{
#ifdef NEWPARSER
	bool bFirst = true;
	while(true)
	{
		int nStartCol = pParser->m_nPos - pParser->m_nLineStartPos + 1;
		CSToken* pTok = pParser->GetToken(0);
		if(pTok->GetLength() <= 0)
		{
			pParser->SetError(&Error::EXPECTED_CLOSE_PAREN);
			return;
		}
		if(pTok->StartsWith(")"))
			return;
		if(bFirst)
			bFirst = false;
		else
		{
			if(!pParser->EatToken(","))
			{
				pParser->SetError(&Error::EXPECTED_COMMA_TOKEN);
				return;
			}
			pTok = pParser->GetToken(0);
			if(pTok->GetLength() <= 0)
			{
				pParser->SetError(&Error::EXPECTED_CLOSE_PAREN);
				return;
			}
		}
		GQueue q;
		int nParenNests = 0;
		while(true)
		{
			if(pTok->StartsWith("("))
				nParenNests++;
			else if(pTok->StartsWith(")"))
			{
				if(nParenNests > 0)
					nParenNests--;
				else
					break;
			}
			else if(pTok->StartsWith(","))
				break;
			q.Push((unsigned char*)pTok->GetValue(), pTok->GetLength());
			pParser->Advance();
			pTok = pParser->GetToken(0);
			if(pTok->GetLength() <= 0)
			{
				pParser->SetError(&Error::EXPECTED_CLOSE_PAREN);
				return;
			}
		}
		GXMLTag* pParamTag = new GXMLTag(TAG_NAME_PARAM);
		pParamTag->SetLineNumber(pParser->m_nLineNumber);
		int nEndCol = pParser->m_nPos - pParser->m_nLineStartPos + 1;
		pParamTag->SetColumnAndWidth(nStartCol, nEndCol - nStartCol);
		pParentTag->AddChildTag(pParamTag);
		Holder<char*> hExp(q.DumpToString());
		pParamTag->AddAttribute(new GXMLAttribute(ATTR_EXP, hExp.Get()));
	}
#else // NEWPARSER
	// todo: don't parse tokens inside "" string quotes
	bool bFirst = true;
	while(true)
	{
		int nStartCol = pParser->m_nPos - pParser->m_nLineStartPos + 1;
		if(!pParser->PeekToken(g_stdDel, true))
		{
			pParser->SetError(&Error::EXPECTED_CLOSE_PAREN);
			return;
		}
		if(pParser->m_pTokenBuffer[0] == ')')
			break;
		if(bFirst)
			bFirst = false;
		else
		{
			if(!pParser->EatToken(","))
			{
				pParser->SetError(&Error::EXPECTED_COMMA_TOKEN);
				return;
			}
			if(!pParser->PeekToken(g_stdDel, true))
			{
				pParser->SetError(&Error::EXPECTED_CLOSE_PAREN);
				return;
			}
		}
		GXMLTag* pParamTag = new GXMLTag(TAG_NAME_PARAM);
		pParamTag->SetLineNumber(pParser->m_nLineNumber);
		int nEndCol = pParser->m_nPos - pParser->m_nLineStartPos + 1;
		pParamTag->SetColumnAndWidth(nStartCol, nEndCol - nStartCol);
		pParentTag->AddChildTag(pParamTag);
		pParamTag->AddAttribute(new GXMLAttribute(ATTR_EXP, pParser->m_pTokenBuffer));
		bool b = pParser->Advance();
		GAssert(b, "unexpected state");
	}
#endif // NEWPARSER
}

/*virtual*/ COVariable* COExpression::FindVariable(const char* pName, int nLength)
{
	return NULL;
}

// This helper method is called by Compile().  It takes a param tree and
// reduces it down to one or two COVariable objects by creating temporary variables
// as necessary.  If pcOp is NULL, it reduces it to a single COVariable object.  If
// it is non-NULL, then you are requesting a L-Value or a modifiable variable,
// in which case it returns one COVariable object if it fits the need, or two COVariable
// object if it had to make temporary variables.  The value of *pcOp will be:
//	 '\0', to indicate ppOutResult holds a non-temp variable
//	 or '.', to incicate ppOutResult is an object, and ppOutResult2 holds a member number
bool COExpression::ParamTreeToDecl(GCompiler* pCompiler, COInstruction* pObjectWithError, COVariable** ppOutResult, COVariable** ppOutResult2/*=NULL*/, COOperator::Operator* peOp/*=NULL*/)
{
	// See if it's a leaf
	GAssert(ppOutResult, "Bad Params");
	GAssert((!peOp && !ppOutResult2) || (peOp && ppOutResult2), "Bad Params");
	*ppOutResult = NULL;
	if(ppOutResult2)
		*ppOutResult2 = NULL;
	switch(GetExpressionType())
	{
	case COExpression::ET_VARDECL:
		{
			COVariable* pVar = (COVariable*)this;
			if(pVar->IsAutoAlloc())
			{
				// Get the current code indexes
				int nLine = pObjectWithError->GetLineNumber();
				int nCol, nWid;
				pObjectWithError->GetColumnAndWidth(&nCol, &nWid);

				// Make the parameter list (contains only one parameter)
				COExpressionArray paramList;
				COVarRef* pVarRef = new COVarRef(nLine, nCol, nWid, pVar);
				const char* szTmpVarName;
				pVarRef->SetExpReadOnly(false, false, &szTmpVarName);
				paramList.AddExpression(pVarRef);

				// Compile the call
				EMethodSignature methodSig(SIG_NEW);
				bool bVarReadOnly = pVar->IsVarReadOnly();
				pVar->SetVarReadOnly(false);
				if(!COCall::CompileImplicitCall(pCompiler, pVar->GetType(), &methodSig, &paramList, pObjectWithError, (COInstrArray*)pObjectWithError->GetParent()))
				{
					pVar->SetVarReadOnly(bVarReadOnly);
					pCompiler->CheckError();
					return false;
				}
				pVar->SetVarReadOnly(bVarReadOnly);
			}			
			*ppOutResult = pVar;
		}
		return true;
	case COExpression::ET_VARREF:
		*ppOutResult = ((COVarRef*)this)->GetVar();
		return true;
	case COExpression::ET_CONST_INT:
		*ppOutResult = pCompiler->GetConstIntVar(((COConstInt*)this)->GetValue(), pObjectWithError);
		if(!*ppOutResult)
		{
			pCompiler->CheckError();
			return false;
		}
		return true;
	case COExpression::ET_CONST_FLOAT:
		*ppOutResult = pCompiler->GetConstFloatVar(((COConstFloat*)this)->GetValue(), pObjectWithError);
		if(!*ppOutResult)
		{
			pCompiler->CheckError();
			return false;
		}
		return true;
	case COExpression::ET_CONST_STRING:
		*ppOutResult = pCompiler->GetConstStringVar(((COConstString*)this)->GetStringValue(), pObjectWithError);
		if(!*ppOutResult)
		{
			pCompiler->CheckError();
			return false;
		}
		return true;
	case COExpression::ET_NAMED_CONSTANT:
		{
			COConstant* pConstant = ((CONamedConst*)this)->GetNamedConstant();
			if(pConstant->IsString())
				*ppOutResult = pCompiler->GetConstStringVar(pConstant->GetStringValue(), pObjectWithError);
			else
				*ppOutResult = pCompiler->GetConstIntVar(pConstant->GetIntegerValue(), pObjectWithError);
			if(!*ppOutResult)
			{
				pCompiler->CheckError();
				return false;
			}
			return true;
		}
	case COExpression::ET_TYPEREF:
		{
			COType* pType = GetType(pCompiler->m_pCOProject);
			if(pType->GetTypeType() != COType::TT_CLASS)
			{
				// todo: add support for procs in machine classes
				pCompiler->SetError(&Error::MACHINE_PROCS_NOT_SUPPORTED_YET, pObjectWithError);
				return false;
			}
			*ppOutResult = ((COClass*)pType)->GetVariable(pCompiler->m_pCOProject);
			if(!*ppOutResult)
			{
				pCompiler->SetError(&Error::INTERNAL_ERROR, pObjectWithError);
				return false;
			}
			return true;
		}
	case COExpression::ET_COMPOUND:
		{
			COOperator* pOperator = (COOperator*)this;

			// Parse Left side
			COVariable* pLeftDecl;
			if(!pOperator->GetLeftExpression()->ParamTreeToDecl(pCompiler, pObjectWithError, &pLeftDecl, NULL, NULL))
			{
				pCompiler->CheckError();
				return false;
			}

			// Parse Right side
			COVariable* pRightDecl;
			if(!pOperator->GetRightExpression()->ParamTreeToDecl(pCompiler, pObjectWithError, &pRightDecl, NULL, NULL))
			{
				pCompiler->CheckError();
				return false;
			}

			// Return the data
			if(peOp)
			{
				// We need an L-Value, so don't combine the two expressions
				*peOp = pOperator->GetOperator();
				if(*peOp != COOperator::OP_DOT)
				{
					pCompiler->SetError(&Error::EXPRESSION_CANT_EVALUATE_TO_MODIFIABLE_VALUE, pObjectWithError);
					return false;
				}
				*ppOutResult = pLeftDecl;
				*ppOutResult2 = pRightDecl;
				return true;
			}
			else
			{
				if(pOperator->GetLeftExpression()->GetExpressionType() == COExpression::ET_TYPEREF)
				{
					// We can drop the class reference because the second param refers to a global object.
					*ppOutResult = pRightDecl;
					return true;					
				}
				else
				{
					// Combine the two expressions into one
					*ppOutResult = CombineExpressions(pCompiler, pLeftDecl, pOperator->GetOperator(), pRightDecl, pObjectWithError);
					if(!(*ppOutResult))
					{
						pCompiler->CheckError();
						return false;
					}
					return true;
				}
			}
		}
	default:
		GAssert(false, "Unrecognized case");
		pCompiler->SetError(&Error::INTERNAL_ERROR, pObjectWithError);
	}
	return false;
}

// Pass in an expression (param tree) and this will return an object that contains a
// single variable name for you to use in place of the expression.	It
// allocates the EvalExprResult object, but it is your job to delete it
// immediately after you use the variable.
// (If you requested a modifiable of L-Value value, it may give you a temporary
// variable that it plans to set or copy to the right location when you delete the
// EvalExprResult object, so be sure to ALWAYS delete the EvalExprResult
// object as soon as you are done with it.
bool COExpression::Compile(bool bWillModifyVar, bool bWillModifyObj, GCompiler* pCompiler, Holder<EvalExprResult*>* phOutResults, COInstruction* pObjectWithError)
{
	int nLine = GetLineNumber();
	int nCol, nWid;
	GetColumnAndWidth(&nCol, &nWid);
	GAssert(phOutResults, "Bad Parameters");

	// Check read-only access
	const char* szTmpVarName;
	ErrorStruct* pError = CheckReadOnlyAccess(bWillModifyObj, bWillModifyVar, &szTmpVarName);
	if(pError)
	{
		if(pError->nParams == 0)
			pCompiler->SetError(pError, pObjectWithError);
		else if(pError->nParams == 1)
			pCompiler->SetError(pError, pObjectWithError, szTmpVarName);
		else
		{
			GAssert(false, "unexpected number of error message parameters");
		}
		return false;
	}

	// Evaluate
	if(bWillModifyVar || bWillModifyObj)
	{
		COVariable* pLeftResult;
		COOperator::Operator eOperator;
		COVariable* pRightResult;
		if(!ParamTreeToDecl(pCompiler, pObjectWithError, &pLeftResult, &pRightResult, &eOperator))
		{
			pCompiler->CheckError();
			return false;
		}
		if(!pRightResult)
		{
			// We got a lone variable--this will work for modifying pointer or value
			phOutResults->Set(new EvalExprResult(pLeftResult));
			return true;
		}
		else if(eOperator == COOperator::OP_DOT)
		{
			if(bWillModifyVar)
			{
				// Make a temporary value for now.	We'll copy it into the class member when we delete the object
				char szTmp[64];
				pCompiler->GetUniqueID(szTmp);
				COVariable* pVariable = new COVariable(nLine, nCol, nWid, szTmp, GetType(pCompiler->m_pCOProject), false, false, false);
				if(!pCompiler->AsmCmdDecl(pVariable, true, pObjectWithError))
				{
					pCompiler->CheckError();
					return false;
				}
				phOutResults->Set(new EvalExprResult(pVariable, pLeftResult, pRightResult, pCompiler));
				return true;
			}
			else
			{
				// Combine into one var (We can combine them anyway because ints are always used by reference, so it can be a modifiable value even if it's a temp variable)
				COVariable* pNewVar = CombineExpressions(pCompiler, pLeftResult, eOperator, pRightResult, pObjectWithError);
				if(!pNewVar)
				{
					pCompiler->CheckError();
					return false;
				}
				phOutResults->Set(new EvalExprResult(pNewVar));
				return true;
			}
		}
		pCompiler->SetError(&Error::INTERNAL_ERROR, pObjectWithError);
		return false;
	}
	else
	{
		COVariable* pResult;
		if(!ParamTreeToDecl(pCompiler, pObjectWithError, &pResult, NULL, NULL))
		{
			pCompiler->CheckError();
			return false;
		}
		phOutResults->Set(new EvalExprResult(pResult));
		return true;
	}
}

/*static*/ COVariable* COExpression::CompareVars(GCompiler* pCompiler, int nOffset1, int nOffset2, bool bEquals, COInstruction* pObjectWithError, int nLine, int nCol, int nWid)
{
	// Make an integer to hold the result
	char szTmp[64];
	pCompiler->GetUniqueID(szTmp);
	COVariable* pNewVar = new COVariable(nLine, nCol, nWid, szTmp, pCompiler->m_pCOProject->m_pInteger, true, false, false);
	if(!pCompiler->PushStackVariable(pNewVar, true))
	{
		pCompiler->CheckError();
		return NULL;
	}
	int nOffset3;
	if(!pCompiler->FindVarOnStack(&nOffset3, pNewVar, pObjectWithError))
	{
		pCompiler->CheckError();
		return NULL;
	}

	// Allocate the result object
	pCompiler->AddInstr(Instr_NewVariable, pObjectWithError);
	pCompiler->AddInstr(Instr_NewInteger, pObjectWithError);
	pCompiler->AddParam(nOffset3);

	// Write the instructions
	if(bEquals)
	{
		pCompiler->AddInstr(Instr_IsVarEqual, pObjectWithError);
		pCompiler->AddParam(nOffset3);
		pCompiler->AddParam(nOffset1);
		pCompiler->AddParam(nOffset2);
	}
	else
	{
		pCompiler->AddInstr(Instr_IsVarNotEqual, pObjectWithError);
		pCompiler->AddParam(nOffset3);
		pCompiler->AddParam(nOffset1);
		pCompiler->AddParam(nOffset2);
	}
	return pNewVar;
}

/*static*/ COVariable* COExpression::CombineInts(GCompiler* pCompiler, COVariable* pLeftDecl, COVariable* pRightDecl, int nOffset1, int nOffset2, COExpression::Operator eOperator, COInstruction* pObjectWithError, int nLine, int nCol, int nWid)
{
	// Check to be sure both variables are integers
	if(!pLeftDecl->GetType()->CanCastTo(pCompiler->m_pCOProject->m_pInteger))
	{
		pCompiler->SetError(&Error::BAD_EXPRESSION, pObjectWithError);
		return NULL;
	}
	if(!pRightDecl->GetType()->CanCastTo(pCompiler->m_pCOProject->m_pInteger))
	{
		pCompiler->SetError(&Error::BAD_EXPRESSION, pObjectWithError);
		return NULL;
	}

	// Make an integer to hold the result
	char szTmp[64];
	pCompiler->GetUniqueID(szTmp);
	COVariable* pNewVar = new COVariable(nLine, nCol, nWid, szTmp, pCompiler->m_pCOProject->m_pInteger, true, false, false);
	if(!pCompiler->PushStackVariable(pNewVar, true))
	{
		pCompiler->CheckError();
		return NULL;
	}
	int nOffset3;
	if(!pCompiler->FindVarOnStack(&nOffset3, pNewVar, pObjectWithError))
	{
		pCompiler->CheckError();
		return NULL;
	}

	// Allocate the result object
	pCompiler->AddInstr(Instr_NewVariable, pObjectWithError);
	pCompiler->AddInstr(Instr_NewInteger, pObjectWithError);
	pCompiler->AddParam(nOffset3);

	if(eOperator == COOperator::OP_OBJ_EQUALS)
	{
		pCompiler->AddInstr(Instr_IsIntEqual, pObjectWithError);
		pCompiler->AddParam(nOffset3);
		pCompiler->AddParam(nOffset1);
		pCompiler->AddParam(nOffset2);
	}
	else if(eOperator == COOperator::OP_OBJ_NOT_EQUALS)
	{
		pCompiler->AddInstr(Instr_IsIntNotEqual, pObjectWithError);
		pCompiler->AddParam(nOffset3);
		pCompiler->AddParam(nOffset1);
		pCompiler->AddParam(nOffset2);
	}
	else if(eOperator == COOperator::OP_OBJ_LESS_THAN)
	{
		pCompiler->AddInstr(Instr_IsIntLessThan, pObjectWithError);
		pCompiler->AddParam(nOffset3);
		pCompiler->AddParam(nOffset1);
		pCompiler->AddParam(nOffset2);
	}
	else if(eOperator == COOperator::OP_OBJ_LESS_THAN_OR_EQUAL)
	{
		pCompiler->AddInstr(Instr_IsIntGreaterThan, pObjectWithError);
		pCompiler->AddParam(nOffset3);
		pCompiler->AddParam(nOffset2);
		pCompiler->AddParam(nOffset1);
	}
	else if(eOperator == COOperator::OP_OBJ_GREATER_THAN)
	{
		pCompiler->AddInstr(Instr_IsIntGreaterThan, pObjectWithError);
		pCompiler->AddParam(nOffset3);
		pCompiler->AddParam(nOffset1);
		pCompiler->AddParam(nOffset2);
	}
	else if(eOperator == COOperator::OP_OBJ_GREATER_THAN_OR_EQUAL)
	{
		pCompiler->AddInstr(Instr_IsIntLessThan, pObjectWithError);
		pCompiler->AddParam(nOffset3);
		pCompiler->AddParam(nOffset2);
		pCompiler->AddParam(nOffset1);
	}
	else
	{
		// Copy the first variable into the result
		pCompiler->AddInstr(Instr_CopyInt, pObjectWithError);
		pCompiler->AddParam(nOffset3);
		pCompiler->AddParam(nOffset1);

		// Do the math operation
		if(eOperator == COOperator::OP_PLUS)
			pCompiler->AddInstr(Instr_AddInteger, pObjectWithError);
		else if(eOperator == COOperator::OP_MINUS)
			pCompiler->AddInstr(Instr_SubtractInteger, pObjectWithError);
		else if(eOperator == COOperator::OP_TIMES)
			pCompiler->AddInstr(Instr_MultiplyInteger, pObjectWithError);
		else if(eOperator == COOperator::OP_DIVIDE)
			pCompiler->AddInstr(Instr_DivideInteger, pObjectWithError);
		else if(eOperator == COOperator::OP_MODULUS)
			pCompiler->AddInstr(Instr_ModulusInteger, pObjectWithError);
		else if(eOperator == COOperator::OP_XOR)
			pCompiler->AddInstr(Instr_XorInteger, pObjectWithError);
		else if(eOperator == COOperator::OP_AND)
			pCompiler->AddInstr(Instr_AndInteger, pObjectWithError);
		else if(eOperator == COOperator::OP_OR)
			pCompiler->AddInstr(Instr_OrInteger, pObjectWithError);
		else if(eOperator == COOperator::OP_MAX)
			pCompiler->AddInstr(Instr_MaxInteger, pObjectWithError);
		else if(eOperator == COOperator::OP_MIN)
			pCompiler->AddInstr(Instr_MinInteger, pObjectWithError);
		else
		{
			pCompiler->SetError(&Error::BAD_EXPRESSION, pObjectWithError);
			return NULL;
		}
		pCompiler->AddParam(nOffset3);
		pCompiler->AddParam(nOffset2);
	}
	return pNewVar;
}

/*static*/ COVariable* COExpression::CombineCustomTypes(GCompiler* pCompiler, COVariable* pLeftDecl, COVariable* pRightDecl, int nOffset1, int nOffset2, COExpression::Operator eOperator, COInstruction* pObjectWithError, int nLine, int nCol, int nWid)
{
	// Find the common type
	COType* pCommonType;
	if(pLeftDecl->GetType()->CanCastTo(pRightDecl->GetType()))
		pCommonType = pRightDecl->GetType();
	else if(pRightDecl->GetType()->CanCastTo(pLeftDecl->GetType()))
		pCommonType = pLeftDecl->GetType();
	else
	{
		pCompiler->SetError(&Error::BAD_CAST, pObjectWithError);
		return NULL;
	}

	// Make an variable to hold the result
	char szTmp[64];
	pCompiler->GetUniqueID(szTmp);
	COVariable* pNewVar;
	bool bComparator = false;
	if(COOperator::IsComparator(eOperator))
	{
		pNewVar = new COVariable(nLine, nCol, nWid, szTmp, pCompiler->m_pCOProject->m_pInteger, true, false, false);
		bComparator = true;
	}
	else
		pNewVar = new COVariable(nLine, nCol, nWid, szTmp, pCommonType, false, false, false);
	if(!pCompiler->PushStackVariable(pNewVar, true))
	{
		pCompiler->CheckError();
		return NULL;
	}
	int nOffset3;
	if(!pCompiler->FindVarOnStack(&nOffset3, pNewVar, pObjectWithError))
	{
		pCompiler->CheckError();
		return NULL;
	}

	// Allocate the result object
	pCompiler->AddInstr(Instr_NewVariable, pObjectWithError);
	pCompiler->AddInstr(Instr_NewInteger, pObjectWithError);
	pCompiler->AddParam(nOffset3);

	// Select the appropriate signature
	char* szSig = (char*)alloca(50 + strlen(pCommonType->GetName())); // todo: unmagic this value
	strcpy(szSig, "method ");
	if(eOperator == COOperator::OP_OBJ_EQUALS)
		strcat(szSig, "isEqual(&Bool, ");
	else if(eOperator == COOperator::OP_OBJ_NOT_EQUALS)
		strcat(szSig, "isNotEqual(&Bool, ");
	else if(eOperator == COOperator::OP_OBJ_LESS_THAN)
		strcat(szSig, "isLessThan(&Bool, ");
	else if(eOperator == COOperator::OP_OBJ_LESS_THAN_OR_EQUAL)
		strcat(szSig, "isGreaterThan(&Bool, ");
	else if(eOperator == COOperator::OP_OBJ_GREATER_THAN)
		strcat(szSig, "isGreaterThan(&Bool, ");
	else if(eOperator == COOperator::OP_OBJ_GREATER_THAN_OR_EQUAL)
		strcat(szSig, "isLessThan(&Bool, ");
	else
	{
		if(eOperator == COOperator::OP_PLUS)
			strcat(szSig, "add(!");
		else if(eOperator == COOperator::OP_MINUS)
			strcat(szSig, "subtract(!");
		else if(eOperator == COOperator::OP_TIMES)
			strcat(szSig, "multiply(!");
		else if(eOperator == COOperator::OP_DIVIDE)
			strcat(szSig, "divide(!");
		else if(eOperator == COOperator::OP_MODULUS)
			strcat(szSig, "modulus(!");
		else if(eOperator == COOperator::OP_XOR)
			strcat(szSig, "xor(!");
		else if(eOperator == COOperator::OP_AND)
			strcat(szSig, "and(!");
		else if(eOperator == COOperator::OP_OR)
			strcat(szSig, "or(!");
		else if(eOperator == COOperator::OP_MAX)
			strcat(szSig, "max(!");
		else if(eOperator == COOperator::OP_MIN)
			strcat(szSig, "min(!");
		else
		{
			pCompiler->SetError(&Error::UNKNOWN_OPERATOR, pObjectWithError);
			return NULL;
		}
		strcat(szSig, pCommonType->GetName());
		strcat(szSig, ", ");
	}
	strcat(szSig, pCommonType->GetName());
	strcat(szSig, ")");
	EMethodSignature methodSig(szSig);

	// Make the parameter list
	COExpressionArray paramList;
	if(eOperator == COOperator::OP_OBJ_LESS_THAN_OR_EQUAL || eOperator == COOperator::OP_OBJ_GREATER_THAN_OR_EQUAL)
	{
		paramList.AddExpression(new COVarRef(nLine, nCol, nWid, pRightDecl));
		COVarRef* pResultVarRef = new COVarRef(nLine, nCol, nWid, pNewVar);
		const char* szTmp;
		pResultVarRef->SetExpReadOnly(false, true, &szTmp);
		paramList.AddExpression(pResultVarRef);
		paramList.AddExpression(new COVarRef(nLine, nCol, nWid, pLeftDecl));
	}
	else
	{
		paramList.AddExpression(new COVarRef(nLine, nCol, nWid, pLeftDecl));
		COVarRef* pResultVarRef = new COVarRef(nLine, nCol, nWid, pNewVar);
		const char* szTmp;
		pResultVarRef->SetExpReadOnly(false, bComparator, &szTmp);
		paramList.AddExpression(pResultVarRef);
		paramList.AddExpression(new COVarRef(nLine, nCol, nWid, pRightDecl));
	}

	// Compile the call
	if(!COCall::CompileImplicitCall(pCompiler, pCommonType, &methodSig, &paramList, pObjectWithError, (COInstrArray*)pObjectWithError->GetParent()))
	{
		pCompiler->CheckError();
		return NULL;
	}
	return pNewVar;
}

/*static*/ COVariable* COExpression::DereferenceMember(GCompiler* pCompiler, int nOffset1, COVariable* pLeftDecl, COVariable* pRightDecl, COInstruction* pObjectWithError, int nLine, int nCol, int nWid)
{
	// Find the class for the type of variable 1 and member number specified by variable 2
	COType* pType = pLeftDecl->GetType();
	int nMemberNumber = -1;
	if(pType->GetTypeType() == COType::TT_CLASS)
		nMemberNumber = ((COClass*)pType)->FindMember(pRightDecl);
	if(nMemberNumber < 0)
	{
		pCompiler->SetError(&Error::MEMBER_NOT_FOUND, pObjectWithError);
		return NULL;
	}

	// Make a temporary variable to hold the result
	char szTmp[64];
	pCompiler->GetUniqueID(szTmp);
	COVariable* pNewVar = new COVariable(nLine, nCol, nWid, szTmp, pRightDecl->GetType(), false, false, false);
	if(!pCompiler->PushStackVariable(pNewVar, true))
	{
		pCompiler->CheckError();
		return NULL;
	}
	int nOffset3;
	if(!pCompiler->FindVarOnStack(&nOffset3, pNewVar, pObjectWithError))
	{
		pCompiler->CheckError();
		return NULL;
	}

	// Set [Object].[Member] to [Variable]
	pCompiler->AddInstr(Instr_NewVariable, pObjectWithError);
	pCompiler->AddInstr(Instr_GetMember, pObjectWithError);
	pCompiler->AddParam(nOffset3);
	pCompiler->AddParam(nOffset1);
	pCompiler->AddParam(nMemberNumber);

	return pNewVar;
}

/*static*/ COVariable* COExpression::CombineExpressions(GCompiler* pCompiler, COVariable* pLeftDecl, COExpression::Operator eOperator, COVariable* pRightDecl, COInstruction* pObjectWithError)
{
	int nLine = pLeftDecl->GetLineNumber();
	int nCol, nWid;
	pLeftDecl->GetColumnAndWidth(&nCol, &nWid);

	// Find the two variables
	int nOffset1;
	if(!pCompiler->FindVarOnStack(&nOffset1, pLeftDecl, pObjectWithError))
	{
		pCompiler->CheckError();
		return NULL;
	}
	int nOffset2;
	if(eOperator != COOperator::OP_DOT) // Right-decl won't be on stack for '.' because it is a field, not a variable
	{
		if(!pCompiler->FindVarOnStack(&nOffset2, pRightDecl, pObjectWithError))
		{
			pCompiler->CheckError();
			return NULL;
		}
	}

	// Apply the operator
	COVariable* pNewVar = NULL;
	if(eOperator == COOperator::OP_DOT)
		pNewVar = DereferenceMember(pCompiler, nOffset1, pLeftDecl, pRightDecl, pObjectWithError, nLine, nCol, nWid);
	else if(eOperator == COOperator::OP_VAR_EQUALS)
		pNewVar = CompareVars(pCompiler, nOffset1, nOffset2, true, pObjectWithError, nLine, nCol, nWid);
	else if(eOperator == COOperator::OP_VAR_NOT_EQUALS)
		pNewVar = CompareVars(pCompiler, nOffset1, nOffset2, false, pObjectWithError, nLine, nCol, nWid);
	else if(pLeftDecl->GetType()->CanCastTo(pCompiler->m_pCOProject->m_pInteger) || pRightDecl->GetType()->CanCastTo(pCompiler->m_pCOProject->m_pInteger))
		pNewVar = CombineInts(pCompiler, pLeftDecl, pRightDecl, nOffset1, nOffset2, eOperator, pObjectWithError, nLine, nCol, nWid);
	else
		pNewVar = CombineCustomTypes(pCompiler, pLeftDecl, pRightDecl, nOffset1, nOffset2, eOperator, pObjectWithError, nLine, nCol, nWid);

	return pNewVar;
}











// --------------------------------------------------------------------------------------------

int COTypeRef::ToString(char* pBuf)
{
	const char* szName = m_pType->GetName();
	int nSize = strlen(szName);
	if(pBuf)
		strcpy(pBuf, szName);
	return strlen(szName) + 1;
}

/*virtual*/ bool COTypeRef::SetExpReadOnly(bool bObj, bool bVar, const char** ppszVarName)
{
	if(!bObj || !bVar)
	{
		*ppszVarName = m_pType->GetName();
		return false;
	}
	return true;
}

/*virtual*/ ErrorStruct* COTypeRef::CheckReadOnlyAccess(bool bWillModifyObj, bool bWillModifyVar, const char** ppszVarName)
{
	GAssert(!bWillModifyVar || bWillModifyObj, "If you modify the var, the obj is modified too");
	if(bWillModifyVar || bWillModifyObj)
	{
		*ppszVarName = m_pType->GetName();
		return &Error::EXPRESSION_CANT_EVALUATE_TO_MODIFIABLE_VALUE;
	}
	return NULL;
}


// -----------------------------------------------------------------------

int COConstInt::ToString(char* pBuf)
{
	char szTmp[32];
	itoa(m_nValue, szTmp, 10);
	if(pBuf)
		strcpy(pBuf, szTmp);
	return strlen(szTmp) + 1;
}

/*virtual*/ COType* COConstInt::GetType(COProject* pCOProject)
{
	return pCOProject->m_pInteger;
}

// -----------------------------------------------------------------------

int COConstFloat::ToString(char* pBuf)
{
	char szTmp[64];
	sprintf(szTmp, "%lf", m_dValue);
	if(pBuf)
		strcpy(pBuf, szTmp);
	return strlen(szTmp) + 1;
}

/*virtual*/ COType* COConstFloat::GetType(COProject* pCOProject)
{
	return pCOProject->GetFloatClass();
}

// -----------------------------------------------------------------------

COConstString::COConstString(int nLine, int nCol, int nWid, const char* szValue) : COExpression(nLine, nCol, nWid, ET_CONST_STRING)
{
	GAssert(szValue != NULL, "Bad string");
	m_szValue = new char[strlen(szValue) + 1];
	Unescape(szValue);
}

/*virtual*/ COConstString::~COConstString()
{
	delete(m_szValue);
}

void COConstString::Unescape(const char* szSrc)
{
	char* szDest = m_szValue;
	while(*szSrc != '\0')
	{
		if(*szSrc == '\\')
		{
			switch(szSrc[1])
			{
				case 'n':
					*szDest = '\n';
					break;
				case 't':
					*szDest = '\t';
					break;
				case '0':
					*szDest = '\0';
					break;
				default:
					if(szSrc[1] == '\0')
						szSrc--;
					*szDest = '?'; // todo make a compiler/parser error here
					break;
			}
			szSrc++;
		}
		else
			*szDest = *szSrc;
		szDest++;
		szSrc++;
	}
	*szDest = '\0';
}

int COConstString::ToString(char* pBuf)
{
	const char* szVal = m_szValue;
	int nLen = strlen(szVal);
	if(pBuf)
	{
		pBuf[0] = '"';
		memcpy(&pBuf[1], szVal, nLen);
		pBuf[nLen + 1] = '"';
		pBuf[nLen + 2] = '\0';
	}
	return nLen + 3;
}

/*virtual*/ COType* COConstString::GetType(COProject* pCOProject)
{
	return pCOProject->GetStringClass();
}

// -----------------------------------------------------------------------

int CONamedConst::ToString(char* pBuf)
{
	const char* szName = m_pConstant->GetName();
	int nSize = strlen(szName);
	if(pBuf)
		strcpy(pBuf, szName);
	return strlen(szName) + 1;
}

/*virtual*/ bool CONamedConst::SetExpReadOnly(bool bObj, bool bVar, const char** ppszVarName)
{
	if(!bObj || !bVar)
	{
		*ppszVarName = m_pConstant->GetName();
		return false;
	}
	return true;
}

/*virtual*/ ErrorStruct* CONamedConst::CheckReadOnlyAccess(bool bWillModifyObj, bool bWillModifyVar, const char** ppszVarName)
{
	GAssert(!bWillModifyVar || bWillModifyObj, "If you modify the var, the obj is modified too");
	if(bWillModifyVar || bWillModifyObj)
	{
		*ppszVarName = m_pConstant->GetName();
		return &Error::EXPRESSION_CANT_EVALUATE_TO_MODIFIABLE_VALUE;
	}
	return NULL;
}

/*virtual*/ COType* CONamedConst::GetType(COProject* pCOProject)
{
	return pCOProject->m_pInteger;
}

// -----------------------------------------------------------------------

/*virtual*/ COType* COOperator::GetType(COProject* pCOProject)
{
	if(m_eOperator == OP_DOT)
		return m_pExpression2->GetType(pCOProject);
	return pCOProject->m_pInteger;
}

