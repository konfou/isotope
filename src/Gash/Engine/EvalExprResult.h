/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __EVALEXPRRESULT_H__
#define __EVALEXPRRESULT_H__

class GCompilerBase;

// The only way to get an object of this class is to call GParser::Eval()
class EvalExprResult
{
friend bool COExpression::Compile(bool bCanModifyPointer, bool bCanModifyValue, GCompiler* pCompiler, Holder<EvalExprResult*>* phOutResults, COInstruction* pObjectWithError);

protected:
	COVariable* m_pResult;
	COVariable* m_pVar1;
	COVariable* m_pVar2;
	GCompiler* m_pParser;

private:
	// The only constructor is private so that only GParser::Eval can
	// make objects of this class.
	EvalExprResult(COVariable* pResult, COVariable* pVar1 = NULL, COVariable* pVar2 = NULL, GCompiler* pParser = NULL);

public:
	virtual ~EvalExprResult();

	COVariable* GetResult() { return m_pResult; }
};




class ParamVarArrayHolder
{
protected:
	int m_nCount;
	int* m_pStackPositions;
	Holder<EvalExprResult*>** m_pVars;

public:
	ParamVarArrayHolder(int nCount)
	{
		// Allocate an array of ints
		m_nCount = nCount;
		m_pStackPositions = new int[nCount];

		// Allocate an array of pointers
		m_pVars = new Holder<EvalExprResult*>*[nCount];

		// Init each element to an empty Holder<EvalExprResult*>
		int n;
		for(n = 0; n < nCount; n++)
			m_pVars[n] = new Holder<EvalExprResult*>(NULL);
	}

	~ParamVarArrayHolder()
	{
		// Delete each element
		int n;
		for(n = m_nCount - 1; n >= 0; n--)
			delete(m_pVars[n]);

		// Delete the arrays
		delete(m_pVars);
		delete(m_pStackPositions);
	}

	int GetCount() { return m_nCount; }

	Holder<EvalExprResult*>* GetParamVarHolder(int n)
	{
		GAssert(n >= 0 && n < m_nCount, "out of range");
		return m_pVars[n];
	}

	COVariable* GetParamVar(int n)
	{
		return GetParamVarHolder(n)->Get()->GetResult();
	}

	void SetStackPos(int nParam, int nValue)
	{
		GAssert(nParam >= 0 && nParam < m_nCount, "out of range");
		m_pStackPositions[nParam] = nValue;
	}

	int GetStackPos(int nParam)
	{
		GAssert(nParam >= 0 && nParam < m_nCount, "out of range");
		return m_pStackPositions[nParam];
	}
};




#endif // __EVALEXPRRESULT_H__
