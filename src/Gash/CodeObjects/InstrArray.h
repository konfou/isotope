/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __INSTRARRAY_H__
#define __INSTRARRAY_H__

#include "../../GClasses/GMacros.h"
#include "Scope.h"

class GPointerArray;
class COInstruction;
class COMethodCallThatCanHaveChildren;
class COMethod;
class COBlock;
class COScope;
class COVariable;
class COProject;
class GQueue;

class COInstrArray : public COScope
{
protected:
	GPointerArray* m_pInstrArray;
	int m_nInstrWithChildrenCount;

public:
	COInstrArray(int nLine, int nCol, int nWid, COScope* pScope);

	virtual ~COInstrArray()
	{
		DestructInstructions(m_pInstrArray);
	}

	int GetInstrCount();
	int GetInstrWithChildrenCount();
	COInstruction* GetInstr(int n);
	void InsertInstr(int nPos, COInstruction* pNewInstruction);
	void SetInstructions(GPointerArray* pChildInstructions);
	void SetInstr(int n, COInstruction* pInstr);
	void AddInstr(COInstruction* pInstr);
	COVariable* FindVar(const char* szName);
	void LoadFromXML(GXMLTag* pTag, COProject* pCOProject, bool bPartial);
	void SaveToXML(GXMLTag* pParentTag);
	void SaveToClassicSyntax(GQueue* pQ, int nTabs);
	virtual ScopeType GetScopeType() { return ST_INSTRUCTION_ARRAY; }
	virtual COVariable* FindVariable(const char* pName, int nLength);
	int FindInstruction(COInstruction* pInstr);

protected:
	static void DestructInstructions(GPointerArray* pChildInstructions);
	COVariable* SearchInstructionsForVar(const char* pName, int nLength);
};

#endif // __INSTRARRAY_H__
