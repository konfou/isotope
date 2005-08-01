/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __COExpression_H__
#define __COExpression_H__

#include "CodeObject.h"
#include "../../GClasses/GMacros.h"
#include "../../GClasses/GArray.h"

class COVariable;
class COMethod;
class COProject;
class COConstant;
class COType;
class GQueue;
class COClass;
class ClassicSyntax;
class COScope;
class GCompilerBase;
class EvalExprResult;
class COOperator;
class COCall;
class COInstruction;

// This class represents a single Expression in a call.  (It could be
// a tree of COExpressions that are combined to express the expression.)
class COExpression : public CodeObject
{
public:
	enum ExpressionType
	{
		ET_VARDECL,
		ET_VARREF,
		ET_TYPEREF,
		ET_CONST_INT,
		ET_CONST_FLOAT,
		ET_CONST_STRING,
		ET_NAMED_CONSTANT,
		ET_COMPOUND,
	};

	enum Operator
	{
		OP_NONE,
		OP_DOT,
		OP_PLUS,
		OP_MINUS,
		OP_TIMES,
		OP_DIVIDE,
		OP_COLON,
		OP_POUND,
		OP_MODULUS,
		OP_XOR,
		OP_AND,
		OP_OR,
		OP_MAX,
		OP_MIN,
		OP_VAR_EQUALS,
		OP_VAR_NOT_EQUALS,
		OP_OBJ_EQUALS,
		OP_OBJ_NOT_EQUALS,
		OP_OBJ_LESS_THAN,
		OP_OBJ_LESS_THAN_OR_EQUAL,
		OP_OBJ_GREATER_THAN,
		OP_OBJ_GREATER_THAN_OR_EQUAL,
	};

protected:
	ExpressionType m_eExpressionType;

public:
	COExpression(int nLine, int nCol, int nWid, ExpressionType eType)
	: CodeObject(nLine, nCol, nWid)
	{
		m_eExpressionType = eType;
	}

	virtual ~COExpression()
	{
	}

	// Getters
	virtual COType* GetType(COProject* pCOProject) = 0;
	ExpressionType GetExpressionType() { return m_eExpressionType; }

	// XML methods
	static COExpression* LoadFromExpression(const char* szExpr, COScope* pScope, COProject* pCOProject, GXMLTag* pTag, bool bPartial);
	static COExpression* LoadFromXML(GXMLTag* pTag, COScope* pScope, COProject* pCOProject, bool bPartial);
	GXMLTag* SaveToXML();
	void SaveToClassicSyntax(GQueue* pQ);
	static void FromClassicSyntax(ClassicSyntax* pParser, GXMLTag* pParentTag);

	// Misc
	virtual int ToString(char* pBuf) = 0;
	virtual bool SetExpReadOnly(bool bObj, bool bVar, const char** ppszVarName) = 0;
	virtual ErrorStruct* CheckReadOnlyAccess(bool bWillModifyObj, bool bWillModifyVar, const char** ppszVarName) = 0;
	virtual COVariable* FindVariable(const char* pName, int nLength);

	bool Compile(bool bWillModifyVar, bool bWillModifyObj, GCompiler* pCompiler, Holder<EvalExprResult*>* phOutResults, COInstruction* pObjectWithError);

protected:
	// This is the method to call to parse an expression
	static COExpression* ParseExpression(const char* szExpression, int nLen, COScope* pScope, COProject* pCOProject, GXMLTag* pTag, bool bPartial);

	// These are all helpers for ParseExpression
	static COExpression* Parse(const char* szExpression, int nLen, COScope* pScope, Operator eOp, COType* pType, COProject* pCOProject, GXMLTag* pTag, bool bPartial);
	static COExpression* ParseToken(const char* szExpression, int nLen, COScope* pScope, Operator eOp, COType* pType, COProject* pCOProject, GXMLTag* pTag, bool bPartial);
	static COExpression* ParseMember(const char* szExpression, int nLen, COClass* pClass, GXMLTag* pTag, COProject* pProject, bool bPartial);
	static COExpression* ParseDecl(const char* szExpression, int nLen, COType* pType, COProject* pCOProject, GXMLTag* pTag, bool bPartial, bool bAutoAlloc);
	static COExpression* ParseIdentifier(const char* szExpression, int nLen, COScope* pScope, COProject* pCOProject, GXMLTag* pTag, bool bPartial);

	// Misc
	int GetExpressionInternal(char* szBuff, int nBuffSize);
	void Clear();
	bool ParamTreeToDecl(GCompiler* pCompiler, COInstruction* pObjectWithError, COVariable** ppOutResult, COVariable** ppOutResult2/*=NULL*/, Operator* peOp/*=NULL*/);

	// Helper method used by Compile.  This creates a new variable on the stack (the stack
	// keeps ownership of it) and returns a pointer to the COVariable it created
	static COVariable* CombineExpressions(GCompiler* pCompiler, COVariable* pLeftDecl, Operator eOperator, COVariable* pRightDecl, COInstruction* pObjectWithError);

	static COVariable* CompareVars(GCompiler* pCompiler, int nOffset1, int nOffset2, bool bEquals, COInstruction* pObjectWithError, int nLine, int nCol, int nWid);
	static COVariable* CombineInts(GCompiler* pCompiler, COVariable* pLeftDecl, COVariable* pRightDecl, int nOffset1, int nOffset2, COExpression::Operator eOperator, COInstruction* pObjectWithError, int nLine, int nCol, int nWid);
	static COVariable* CombineCustomTypes(GCompiler* pCompiler, COVariable* pLeftDecl, COVariable* pRightDecl, int nOffset1, int nOffset2, COExpression::Operator eOperator, COInstruction* pObjectWithError, int nLine, int nCol, int nWid);
	static COVariable* DereferenceMember(GCompiler* pCompiler, int nOffset1, COVariable* pLeftDecl, COVariable* pRightDecl, COInstruction* pObjectWithError, int nLine, int nCol, int nWid);
};







class COTypeRef : public COExpression
{
protected:
	COType* m_pType;

public:
	COTypeRef(int nLine, int nCol, int nWid, COType* pType) : COExpression(nLine, nCol, nWid, ET_TYPEREF)
	{
		GAssert(pType != NULL, "Bad type");
		m_pType = pType;
	}

	virtual ~COTypeRef()
	{
	}

	virtual COType* GetType(COProject* pCOProject)
	{
		return m_pType;
	}

	virtual int ToString(char* pBuf);
	virtual ErrorStruct* CheckReadOnlyAccess(bool bWillModifyObj, bool bWillModifyVar, const char** ppszVarName);
	virtual bool SetExpReadOnly(bool bObj, bool bVar, const char** ppszVarName);
};




class COConstInt : public COExpression
{
protected:
	unsigned int m_nValue;

public:
	COConstInt(int nLine, int nCol, int nWid, int nValue) : COExpression(nLine, nCol, nWid, ET_CONST_INT)
	{
		m_nValue = nValue;
	}

	virtual ~COConstInt()
	{
	}

	virtual COType* GetType(COProject* pCOProject);
	virtual int ToString(char* pBuf);

	int GetValue()
	{
		GAssert(m_eExpressionType == ET_CONST_INT, "not a ET_CONST_INT type");
		return m_nValue;
	}

	virtual ErrorStruct* CheckReadOnlyAccess(bool bWillModifyObj, bool bWillModifyVar, const char** ppszVarName)
	{
		GAssert(!bWillModifyVar || bWillModifyObj, "If you modify the var, the obj is modified too");
		if(bWillModifyVar || bWillModifyObj)
		{
			*ppszVarName = "<constant integer>";
			return &Error::EXPRESSION_CANT_EVALUATE_TO_MODIFIABLE_VALUE;
		}
		return NULL;
	}

	virtual bool SetExpReadOnly(bool bObj, bool bVar, const char** ppszVarName)
	{
		if(!bObj || !bVar)
		{
			*ppszVarName = "<constant integer>";
			return false;
		}
		return true;
	}
};




class COConstFloat : public COExpression
{
protected:
	double m_dValue;

public:
	COConstFloat(int nLine, int nCol, int nWid, double dValue) : COExpression(nLine, nCol, nWid, ET_CONST_FLOAT)
	{
		m_dValue = dValue;
	}

	virtual ~COConstFloat()
	{
	}

	virtual COType* GetType(COProject* pCOProject);
	virtual int ToString(char* pBuf);

	double GetValue()
	{
		GAssert(m_eExpressionType == ET_CONST_FLOAT, "not a ET_CONST_FLOAT type");
		return m_dValue;
	}

	virtual ErrorStruct* CheckReadOnlyAccess(bool bWillModifyObj, bool bWillModifyVar, const char** ppszVarName)
	{
		GAssert(!bWillModifyVar || bWillModifyObj, "If you modify the var, the obj is modified too");
		if(bWillModifyVar || bWillModifyObj)
		{
			*ppszVarName = "<constant float>";
			return &Error::EXPRESSION_CANT_EVALUATE_TO_MODIFIABLE_VALUE;
		}
		return NULL;
	}

	virtual bool SetExpReadOnly(bool bObj, bool bVar, const char** ppszVarName)
	{
		if(!bObj || !bVar)
		{
			*ppszVarName = "<constant float>";
			return false;
		}
		return true;
	}
};




class COConstString : public COExpression
{
protected:
	char* m_szValue;

public:
	COConstString(int nLine, int nCol, int nWid, const char* szValue);
	virtual ~COConstString();

	virtual COType* GetType(COProject* pCOProject);
	virtual int ToString(char* pBuf);

	void Unescape(const char* szVal);

	const char* GetStringValue()
	{
		GAssert(m_eExpressionType == ET_CONST_STRING, "not a ET_CONST_STRING type");
		return m_szValue;
	}

	virtual ErrorStruct* CheckReadOnlyAccess(bool bWillModifyObj, bool bWillModifyVar, const char** ppszVarName)
	{
		GAssert(!bWillModifyVar || bWillModifyObj, "If you modify the var, the obj is modified too");
		if(bWillModifyVar || bWillModifyObj)
		{
			*ppszVarName = "<constant string>";
			return &Error::EXPRESSION_CANT_EVALUATE_TO_MODIFIABLE_VALUE;
		}
		return NULL;
	}

	virtual bool SetExpReadOnly(bool bObj, bool bVar, const char** ppszVarName)
	{
		if(!bObj || !bVar)
		{
			*ppszVarName = "<constant string>";
			return false;
		}
		return true;
	}
};




class CONamedConst : public COExpression
{
protected:
	COConstant* m_pConstant;

public:
	CONamedConst(int nLine, int nCol, int nWid, COConstant* pConstant) : COExpression(nLine, nCol, nWid, ET_NAMED_CONSTANT)
	{
		m_pConstant = pConstant;
	}

	virtual ~CONamedConst()
	{
	}

	virtual COType* GetType(COProject* pCOProject);
	virtual int ToString(char* pBuf);

	COConstant* GetNamedConstant()
	{
		GAssert(m_eExpressionType == ET_NAMED_CONSTANT, "Not a ET_NAMED_CONSTANT type");
		return m_pConstant;
	}

	virtual ErrorStruct* CheckReadOnlyAccess(bool bWillModifyObj, bool bWillModifyVar, const char** ppszVarName);
	virtual bool SetExpReadOnly(bool bObj, bool bVar, const char** ppszVarName);
};



class COExpressionArray : public GPointerArray
{
public:
	COExpressionArray() : GPointerArray(4)
	{
	}

	virtual ~COExpressionArray()
	{
		int nCount = GetSize();
		int n;
		for(n = 0; n < nCount; n++)
			delete(GetExpression(n));
	}

	COExpression* GetExpression(int nIndex) { return (COExpression*)GetPointer(nIndex); }
	void AddExpression(COExpression* pExp) { AddPointer(pExp); }
};


#endif // __COExpression_H__
