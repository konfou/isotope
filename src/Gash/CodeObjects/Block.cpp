/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "Block.h"
#include "InstrArray.h"
#include "../Engine/TagNames.h"
#include "../Engine/GCompiler.h"
#include "../../GClasses/GXML.h"
#include "../../GClasses/GArray.h"
#include "../../GClasses/GQueue.h"

COBlock::COBlock(int nLine, int nCol, int nWid, COInstrArray* pParent, const char* szComment)
 : COInstruction(nLine, nCol, nWid, pParent)
{
	GAssert(pParent, "Must have valid parent");
	m_pInstrArray = new COInstrArray(nLine, nCol, nWid, this);
	m_szComment = NULL;
	if(szComment)
		SetComment(szComment);
}

COBlock::~COBlock()
{
	delete(m_pInstrArray);
	delete(m_szComment);
}

void COBlock::SetComment(const char* szComment)
{
	delete(m_szComment);
	m_szComment = new char[strlen(szComment) + 1];
	strcpy(m_szComment, szComment);
}

/*static*/ COBlock* COBlock::FromXML(GXMLTag* pTag, COInstrArray* pParent, COProject* pCOProject, bool bPartial)
{
	// Make a COBlock object
	const char* szComment = NULL;
	GXMLAttribute* pCommentTag = pTag->GetAttribute(ATTR_COMMENT);
	if(pCommentTag)
		szComment = pCommentTag->GetValue();
	int nLine = pTag->GetLineNumber();
	int nCol, nWid;
	pTag->GetOffsetAndWidth(&nCol, &nWid);
	COBlock* pBlock = new COBlock(nLine, nCol, nWid, pParent, szComment);

	return pBlock;
}

void COBlock::LoadChildInstructions(GXMLTag* pTag, COProject* pCOProject, bool bPartial)
{
	m_pInstrArray->LoadFromXML(pTag, pCOProject, bPartial);
}

/*virtual*/ GXMLTag* COBlock::SaveToXML(COInstrArray* pParent)
{
	// Make the Block tag
	GXMLTag* pBlockTag = new GXMLTag(TAG_NAME_BLOCK);
	if(strlen(m_szComment) > 0)
		pBlockTag->AddAttribute(new GXMLAttribute(ATTR_COMMENT, m_szComment));

	// Add child instructions
	m_pInstrArray->SaveToXML(pBlockTag);

	return pBlockTag;
}

void COBlock::SaveToClassicSyntax(GQueue* pQ, int nTabs)
{
	int n;
	for(n = 0; n < nTabs; n++)
		pQ->Push("\t");
	pQ->Push("/ ");
	pQ->Push(GetComment());
	pQ->Push("\r\n");

	// Add child instructions
	m_pInstrArray->SaveToClassicSyntax(pQ, nTabs);

	pQ->Push("\r\n");
}

/*static*/ void COBlock::FromClassicSyntax(ClassicSyntax* pParser, GXMLTag* pParentTag)
{
#ifdef NEWPARSER
	GAssert(false, "how'd this happen?");
#else // NEWPARSER
	pParser->m_bFoundBlankLine = false;
	GQueue q;
	while(true)
	{
		bool b = pParser->EatToken("/");
		GAssert(b, "unexpected state");
		if(pParser->PeekToken("\n")) // todo: fix--if comment is bigger than buffer, it gets lost
			q.Push(pParser->m_pTokenBuffer);
		b = pParser->Advance();
		GAssert(b, "unexpected state");
		if(!pParser->PeekToken() || pParser->m_pTokenBuffer[0] != '/')
			break;
	}
	GXMLTag* pBlockTag = new GXMLTag(TAG_NAME_BLOCK);
	pBlockTag->SetLineNumber(pParser->m_nLineNumber);
	pBlockTag->SetColumnAndWidth(pParser->m_nPos - pParser->m_nLineStartPos + 1, 5);
	pParentTag->AddChildTag(pBlockTag);
	Holder<char*> hComments(q.DumpToString());
	pBlockTag->AddAttribute(new GXMLAttribute(ATTR_COMMENT, hComments.Get()));
	COInstruction::FromClassicSyntax(pParser, pBlockTag, ClassicSyntax::IT_BLOCK);
#endif // NEWPARSER
}

/*virtual*/ COVariable* COBlock::FindVariable(const char* pName, int nLength)
{
	return m_pParent->FindVariable(pName, nLength);
}

/*virtual*/ bool COBlock::Compile(GCompiler* pCompiler, COMethod* pMethod, COInstruction* pSymbolInstr)
{
	// Compile each child instruction
	COInstruction* pInstruction;
	COInstrArray* pChildren = GetChildInstructions();
	int nInstructionCount = pChildren->GetInstrCount();
	int n;
	for(n = 0; n < nInstructionCount; n++)
	{
		pInstruction = pChildren->GetInstr(n);
		if(!pInstruction->Compile(pCompiler, pMethod, pSymbolInstr))
		{
			pCompiler->CheckError();
			return false;
		}
	}
	return true;
}

