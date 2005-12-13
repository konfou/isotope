/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __SCOPE_H__
#define __SCOPE_H__

#include "CodeObject.h"

class COMethod;
class COVariable;

class COScope : public CodeObject
{
public:
	enum ScopeType
	{
		ST_INSTRUCTION,
		ST_INSTRUCTION_ARRAY,
		ST_METHOD,
	};

protected:
	COScope* m_pParent;

public:
	COScope(int nLine, int nCol, int nWid, COScope* pParent);
	virtual ~COScope() {}

	COMethod* GetMethod();
	virtual COVariable* FindVariable(const char* pName, int nLength) = 0;
	virtual ScopeType GetScopeType() = 0;
	COScope* GetParent() { return m_pParent; }
};

#endif // __SCOPE_H__
