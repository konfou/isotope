/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "Instruction.h"
#include "InstrArray.h"
#include "Variable.h"
#include "Call.h"
#include "Block.h"
#include "Project.h"
#include "../Engine/TagNames.h"
#include "../../GClasses/GXML.h"
#include "../../GClasses/GQueue.h"
#include "../../GClasses/GArray.h"
#include "../../GClasses/GPointerQueue.h"

COInstruction::COInstruction(int nLine, int nCol, int nWid, COInstrArray* pParent)
: COScope(nLine, nCol, nWid, pParent)
{
	m_nIndex = -1;
}

COInstruction::~COInstruction()
{
}

/*static*/ COInstruction* COInstruction::FromXML(GXMLTag* pTag, COInstrArray* pParent, COProject* pCOProject, bool bPartial, int* pnInstructionIndex)
{
	COInstruction* pNewInstruction = NULL;
	const char* szName = pTag->GetName();
	int nIndex = *pnInstructionIndex;
	(*pnInstructionIndex)++;
	COInstruction* pInstruction;
	if(stricmp(szName, TAG_NAME_CALL) == 0)
		pInstruction = COCall::FromXML(pTag, pParent, pCOProject, bPartial, pnInstructionIndex);
	else if(stricmp(szName, TAG_NAME_BLOCK) == 0)
		pInstruction = COBlock::FromXML(pTag, pParent, pCOProject, bPartial);
	else
		pCOProject->ThrowError(&Error::EXPECTED_ASM_CALL_OR_CALLBACK_TAG, pTag);
	pInstruction->m_nIndex = nIndex;
	return pInstruction;
}

GXMLTag* COInstruction::SaveToXML(COInstrArray* pParent)
{
	switch(GetInstructionType())
	{
		case IT_CALL:
			return ((COCall*)this)->SaveToXML(pParent);

		case IT_BLOCK:
			return ((COBlock*)this)->SaveToXML(pParent);
	}

	GAssert(false, "Unrecognized instruction type");
	return NULL;
}

void COInstruction::SaveToClassicSyntax(GQueue* pQ, int nTabs, bool bDisplay)
{
	switch(GetInstructionType())
	{
		case IT_CALL:
			((COCall*)this)->SaveToClassicSyntax(pQ, nTabs, bDisplay);
			return;

		case IT_BLOCK:
			((COBlock*)this)->SaveToClassicSyntax(pQ, nTabs);
			return;
	}

	GAssert(false, "Unrecognized instruction type");
}

void COInstruction::FromClassicSyntax(ClassicSyntax* pParser, GXMLTag* pParentTag, ClassicSyntax::InstrType eType)
{
	// Parse '{'
	if(eType != ClassicSyntax::IT_BLOCK)
	{
		CSToken* pTok = pParser->GetToken(0);
		if(pTok->GetLength() <= 0)
		{
			pParser->SetError(&Error::EXPECTED_OPEN_SQUIGGLY_BRACE);
			return;
		}
		if(pTok->StartsWith("{"))
		{
			bool b = pParser->EatToken("{");
			GAssert(b, "unexpected state");
		}
		else
		{
			pParser->ParseSingleInstr(pParentTag);
			pTok = pParser->GetToken(0);
			if(pTok->GetLength() <= 0)
			{
				pParser->SetError(&Error::EXPECTED_CLOSE_SQUIGGLY_BRACE);
				return;
			}
			if(eType == ClassicSyntax::IT_IF && pTok->StartsWith("Else"))
				pParser->ParseElse(pParentTag);
			return;
		}
	}

	// Parse each instruction	
	while(true)
	{
		CSToken* pTok = pParser->GetToken(0);
		if(pTok->GetLength() <= 0)
		{
			pParser->SetError(&Error::EXPECTED_CLOSE_SQUIGGLY_BRACE);
			return;
		}
		if(pTok->StartsWith("}"))
			break;
		else if(eType == ClassicSyntax::IT_BLOCK && (pParser->m_bFoundBlankLine || pTok->StartsWith("/")))
			break;
		pParser->ParseSingleInstr(pParentTag);
		if(pParser->m_pErrorHolder->HaveError())
			return;
	}

	// Parse closer
	if(eType != ClassicSyntax::IT_BLOCK)
	{
		if(!pParser->EatToken("}"))
		{
			pParser->SetError(&Error::EXPECTED_CLOSE_SQUIGGLY_BRACE);
			return;
		}
	}
	CSToken* pTok = pParser->GetToken(0);
	if(pTok->GetLength() <= 0)
	{
		pParser->SetError(&Error::EXPECTED_CLOSE_SQUIGGLY_BRACE); // for the end of the class
		return;
	}
	if(eType == ClassicSyntax::IT_IF && pTok->StartsWith("Else"))
		pParser->ParseElse(pParentTag);
}

void COInstruction::GetStepOverInstructions(GPointerQueue* pInstrQueue)
{
	GAssert(m_pParent->GetScopeType() == ST_INSTRUCTION_ARRAY, "unexpected parent type");
	COInstrArray* pInstrArray = (COInstrArray*)m_pParent;
	int nIndex = pInstrArray->FindInstruction(this);
	GAssert(nIndex >= 0, "Couldn't find self");
	int nCount = pInstrArray->GetInstrCount();
	int nNext = nIndex + 1;
	if(nNext >= nCount)
	{
		if(pInstrArray->GetParent()->GetScopeType() == ST_INSTRUCTION)
		{
			COInstruction* pOuterInstruction = (COInstruction*)pInstrArray->GetParent();
			pOuterInstruction->GetStepOverInstructions(pInstrQueue);
		}
		nNext = 0;
	}
	pInstrQueue->Push(pInstrArray->GetInstr(nNext));
}
