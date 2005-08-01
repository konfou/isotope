/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "InstrArray.h"
#include "Instruction.h"
#include "Block.h"
#include "Call.h"
#include "../../GClasses/GArray.h"
#include "../../GClasses/GXML.h"
#include "../../GClasses/GQueue.h"

#define ARRAY_GROW_SIZE 16

COInstrArray::COInstrArray(int nLine, int nCol, int nWid, COScope* pScope)
: COScope(nLine, nCol, nWid, pScope)
{
	GAssert(pScope, "COInstrArray should always have a valid parent");
	m_pInstrArray = NULL;
	m_nInstrWithChildrenCount = -1;
}

int COInstrArray::GetInstrWithChildrenCount()
{
	if(m_nInstrWithChildrenCount < 0)
	{
		m_nInstrWithChildrenCount = 0;
		int nCount = m_pInstrArray->GetSize();
		COInstruction* pInstruction;
		int n;
		for(n = 0; n < nCount; n++)
		{
			pInstruction = (COInstruction*)m_pInstrArray->GetPointer(n);
			switch(pInstruction->GetInstructionType())
			{
				case COInstruction::IT_BLOCK:
					m_nInstrWithChildrenCount++;
					break;

				case COInstruction::IT_CALL:
					if(((COCall*)pInstruction)->CanHaveChildren())
						m_nInstrWithChildrenCount++;
					break;

				default:
					GAssert(false, "unexpected type");
			}
		}
	}
	return m_nInstrWithChildrenCount;
}

void COInstrArray::DestructInstructions(GPointerArray* pChildInstructions)
{
	if(!pChildInstructions)
		return;
	COInstruction* pInstruction;
	int n;
	int nCount = pChildInstructions->GetSize();
	for(n = 0; n < nCount; n++)
	{
		pInstruction = (COInstruction*)pChildInstructions->GetPointer(n);
		delete(pInstruction);
	}
	delete(pChildInstructions);
}

void COInstrArray::SetInstructions(GPointerArray* pChildInstructions)
{
	DestructInstructions(m_pInstrArray);
	m_pInstrArray = pChildInstructions;
}

void COInstrArray::AddInstr(COInstruction* pInstr)
{
	if(!m_pInstrArray)
		m_pInstrArray = new GPointerArray(ARRAY_GROW_SIZE);
	m_pInstrArray->AddPointer(pInstr);
}

void COInstrArray::InsertInstr(int nPos, COInstruction* pNewInstruction)
{
	if(!m_pInstrArray)
		m_pInstrArray = new GPointerArray(ARRAY_GROW_SIZE);
	GAssert(nPos >= 0 && nPos <= m_pInstrArray->GetSize(), "Out of range");
	m_pInstrArray->InsertPointer(nPos, pNewInstruction);
}

int COInstrArray::GetInstrCount()
{
	if(m_pInstrArray)
		return m_pInstrArray->GetSize();
	else
		return 0;
}

COInstruction* COInstrArray::GetInstr(int n)
{
	return (COInstruction*)m_pInstrArray->GetPointer(n);
}

void COInstrArray::SetInstr(int n, COInstruction* pInstr)
{
	m_pInstrArray->SetPointer(n, pInstr);
}

void COInstrArray::LoadFromXML(GXMLTag* pTag, COProject* pCOProject, bool bPartial)
{
	GAssert(!m_pInstrArray, "Array for instructions already created!");
	m_pInstrArray = new GPointerArray(ARRAY_GROW_SIZE);

	// Load all the instructions
	GXMLTag* pChild;
	for(pChild = pTag->GetFirstChildTag(); pChild; pChild = pTag->GetNextChildTag(pChild))
	{
		COInstruction* pNewInstruction = COInstruction::FromXML(pChild, this, pCOProject, bPartial);
		AddInstr(pNewInstruction);
		if(pNewInstruction->GetInstructionType() == COInstruction::IT_BLOCK)
			((COBlock*)pNewInstruction)->LoadChildInstructions(pChild, pCOProject, bPartial);
	}
}

void COInstrArray::SaveToXML(GXMLTag* pParentTag)
{
	// Add child instructions
	if(m_pInstrArray)
	{
		COInstruction* pInstruction;
		int nCount = GetInstrCount();
		int n;
		for(n = 0; n < nCount; n++)
		{
			pInstruction = GetInstr(n);
			pParentTag->AddChildTag(pInstruction->SaveToXML(this));
		}
	}
}

void COInstrArray::SaveToClassicSyntax(GQueue* pQ, int nTabs)
{
	COInstruction* pInstruction;
	int nCount = GetInstrCount();
	if(nCount != 1)
	{
		int n;
		for(n = 0; n < nTabs; n++)
			pQ->Push("\t");
		pQ->Push("{\n");
	}
	int n;
	for(n = 0; n < nCount; n++)
	{
		pInstruction = GetInstr(n);
		pInstruction->SaveToClassicSyntax(pQ, nTabs + 1);
	}
	if(nCount != 1)
	{
		for(n = 0; n < nTabs; n++)
			pQ->Push("\t");
		pQ->Push("}\n");
	}
}

COVariable* COInstrArray::SearchInstructionsForVar(const char* pName, int nLength)
{
	COVariable* pVar = NULL;
	COInstruction* pInstruction;
	int nCount = GetInstrCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		pInstruction = GetInstr(n);
		if(pInstruction->GetInstructionType() == COInstruction::IT_BLOCK)
			pVar = ((COBlock*)pInstruction)->GetChildInstructions()->SearchInstructionsForVar(pName, nLength);
		else
			pVar = pInstruction->FindVariable(pName, nLength);
		if(pVar)
			return pVar;
	}
	return NULL;
}

/*virtual*/ COVariable* COInstrArray::FindVariable(const char* pName, int nLength)
{
	if(m_pParent->GetScopeType() != ST_INSTRUCTION ||
		((COInstruction*)m_pParent)->GetInstructionType() != COInstruction::IT_BLOCK)
	{
		COVariable* pVar = SearchInstructionsForVar(pName, nLength);
		if(pVar)
			return pVar;
	}
	return m_pParent->FindVariable(pName, nLength);
}

int COInstrArray::FindInstruction(COInstruction* pInstr)
{
	int nCount = GetInstrCount();
	int n;
	for(n = 0; n < nCount; n++)
	{
		if(GetInstr(n) == pInstr)
			return n;
	}
	return -1;
}

