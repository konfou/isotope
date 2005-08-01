/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GParseTree.h"
#include "GGrammar.h"

GParseTreeNode::GParseTreeNode(GGrammarRule* pRule)
{
	m_pRule = pRule;
	m_nStartPos = 0;
	m_nEndPos = 0;
}

GParseTreeNode::~GParseTreeNode()
{

}

// **********************************************************

GStructureParseTreeNode::GStructureParseTreeNode(GStructureGrammarRule* pRule) : GParseTreeNode(pRule)
{
	m_eType = PTNT_STRUCTURE;
	m_nFieldCount = pRule->m_pFields->GetSize();
	m_ppChildren = new GParseTreeNode*[m_nFieldCount];
	m_nChildCount = 0;
}

GStructureParseTreeNode::~GStructureParseTreeNode()
{
	int n;
	for(n = 0; n < m_nChildCount; n++)
		delete(m_ppChildren[n]);
	delete(m_ppChildren);
}

void GStructureParseTreeNode::AddChild(GParseTreeNode* pChild)
{
	if(m_nChildCount >= m_nFieldCount)
		GAssert(false, "Error, too many children");
	m_ppChildren[m_nChildCount] = pChild;
	m_nChildCount++;
}

GParseTreeNode* GStructureParseTreeNode::GetChild(int n)
{
	if(n >= m_nChildCount)
	{
		GAssert(false, "Not that many children");
		return NULL;
	}
	return m_ppChildren[n];
}

// **********************************************************

GListParseTreeNode::GListParseTreeNode(GListGrammarRule* pRule) : GParseTreeNode(pRule)
{
	m_eType = PTNT_LIST;
	m_pChildren = new GSmallArray(sizeof(GParseTreeNode*), 32);
}

GListParseTreeNode::~GListParseTreeNode()
{
	int nCount = GetChildCount();
	int n;
	for(n = 0; n < nCount; n++)
		delete(GetChild(n));
	delete(m_pChildren);
}


// **********************************************************

GNonDetParseTreeNode::GNonDetParseTreeNode(GNonDetGrammarRule* pRule) : GParseTreeNode(pRule)
{
	m_eType = PTNT_NONDET;
	m_nWhichOne = -1;
	m_pChild = NULL;
}

GNonDetParseTreeNode::~GNonDetParseTreeNode()
{
	delete(m_pChild);
}

// **********************************************************

GTextParseTreeNode::GTextParseTreeNode(ParseTreeNodeTypes ePTNT, GGrammarRule* pRule) : GParseTreeNode(pRule)
{
	m_eType = ePTNT;

}

GTextParseTreeNode::~GTextParseTreeNode()
{

}

// **********************************************************

GSyntaxParseTreeNode::GSyntaxParseTreeNode(GGrammarRule* pRule) : GParseTreeNode(pRule)
{
	m_eType = PTNT_SYNTAX;

}

GSyntaxParseTreeNode::~GSyntaxParseTreeNode()
{

}

