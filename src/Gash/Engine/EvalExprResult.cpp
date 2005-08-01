/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include <stdio.h>
#include "GCompiler.h"
#include "EvalExprResult.h"
#include "../../GClasses/GMacros.h"

EvalExprResult::EvalExprResult(COVariable* pResult, COVariable* pVar1, COVariable* pVar2, GCompiler* pParser)
{
	GAssert((pVar1 == NULL && pVar2 == NULL) || (pVar1 != NULL && pVar2 != NULL), "Bad Params");
	m_pResult = pResult;
	m_pParser = pParser;
	m_pVar1 = pVar1;
	m_pVar2 = pVar2;
}

EvalExprResult::~EvalExprResult()
{
	if(!m_pVar1)
		return;
	GAssert(m_pParser && m_pVar1 && m_pVar2, "This should be set");
	if(!m_pParser->AsmCmdSetMember(NULL, m_pVar1, m_pResult, m_pVar2))
	{
		GAssert(false, "This should never happen");
	}
}
