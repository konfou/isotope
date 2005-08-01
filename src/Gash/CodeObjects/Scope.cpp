/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "Scope.h"
#include "../../GClasses/GMacros.h"

COScope::COScope(int nLine, int nCol, int nWid, COScope* pParent)
: CodeObject(nLine, nCol, nWid)
{
	m_pParent = pParent;
}

COMethod* COScope::GetMethod()
{
	COScope* pScope = this;
	while(pScope->GetScopeType() != ST_METHOD)
		pScope = pScope->m_pParent;
	return (COMethod*)pScope;
}

